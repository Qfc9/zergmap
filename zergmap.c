#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "zergHeaders.h"
#include "zergPrint.h"
#include "netHeaders.h"
#include "util.h"
#include "graph.h"

#define MINPCAPLENGTH 54

int main(int argc, char *argv[])
{
    /*
        This is my main function for decode.
    */

    // Initializing Variables
    FILE *fp;
    struct pcapFileH pHeader;
    struct pcapPacketH ppHeader;
    union ethernetH eHeader;
    struct ipv4H ipHeader;
    struct ipv6H ip6Header;
    struct udpH udpHeader;
    union zergH zHeader;
    int err = 0;
    int swap = 0;
    long int dataLength = 0;
    unsigned int skipBytes = 0;

    // Checking for valid amount for args
    if(argc != 2)
    {
        fprintf(stderr, "Invalid amount of args\n");
        return 1;
    }

    // Attempting to open the file given
    fp = fopen(argv[1], "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Unable to open the file: %s\n", argv[1]);
        return 1;
    }

    // Reading the first header of the file
    if(setPcapHead(fp, &pHeader, "Packet is corrupted or empty"))
    {
        swap = 1;
    }

    // Checking for valid PCAP Header
    if((pHeader.majVer != PCAPHEADMAJ) || (pHeader.minVer != PCAPHEADMIN) || (pHeader.linkType != PCAPHEADLINK))
    {
        fprintf(stderr, "Invalid PCAP Version\n");
        fclose(fp);
        return 1;
    }

    // Main reading loop
    while(setPacketHead(fp, &ppHeader, swap))
    {
        skipBytes = ppHeader.length;
        dataLength = ftell(fp);

        // Checking if packet is of a valid length
        if(ppHeader.length < MINPCAPLENGTH)
        {
            skipAhead(fp, 1, "Invalid Packet Header", skipBytes);
            continue;
        }

        // Reading Ethernet Header
        setEthHead(fp, &eHeader, "Ethernet Header");   
        skipBytes -= (sizeof(eHeader) + ETHCORRECTION);

        // Checking if it's 802.1Q
        if(eHeader.ethInfo.type == ETH8021Q)
        {
            skipAhead(fp, 0, "", ETH8021CORRECTION);
            setEthHead(fp, &eHeader, "Ethernet 802.1Q Header");
        }
        else if(eHeader.ethInfo.type == ETH8021Q4)
        {
            skipAhead(fp, 0, "", ETH8021CORRECTION);
            setEthHead(fp, &eHeader, "Ethernet 802.1Q Header");
            if(eHeader.ethInfo.type == ETH8021Q)
            {
                skipAhead(fp, 0, "", ETH8021CORRECTION);
                setEthHead(fp, &eHeader, "Ethernet 802.1Q Header");
            }
        }

        // Checking if valid Ethernet Header
        if(eHeader.ethInfo.type == ETHIPV4)
        {
            // Reading IP Header
            setIPv4Head(fp, &ipHeader, "IP Header");
            skipBytes -= sizeof(ipHeader);

            // Checking if valid IP Header
            if(ipHeader.version != IPV4 || ipHeader.proto != UDP || ipHeader.ihl < IHLDEFAULT)
            {
                skipAhead(fp, 1, "Invalid IPv4 Header", skipBytes);
                continue;
            }
            // Moving cursors forward if there are options
            else if(ipHeader.ihl > IHLDEFAULT)
            {
                unsigned int ihl = ((ipHeader.ihl - IHLDEFAULT) * 4);
                if(ppHeader.length < (MINPCAPLENGTH + ihl))
                {
                    skipBytes -= ihl;
                    fseek(fp, ihl, SEEK_CUR);
                }
                else
                {
                    skipAhead(fp, 1, "Invalid Packet Header Data length", skipBytes);
                    continue;
                }
            }
        }
        // Checking if it is IPv6
        else if(eHeader.ethInfo.type == ETHIPV6)
        {
            //ip6Header
            setIPv6Head(fp, &ip6Header, "IPv6 Header");
            skipBytes -= sizeof(ip6Header);

            // Checking if valid IP Header
            if(ip6Header.nextHead != UDP)
            {
                skipAhead(fp, 1, "Invalid Transport Layer protocol", skipBytes);
                continue;
            }
        }
        else
        {
            skipAhead(fp, 1, "Invalid Ethernet Header Type", skipBytes);
            continue;

        }

        // Reading UDP and Zerg
        setUDPHead(fp, &udpHeader, "UDP Header");
        skipBytes -= sizeof(udpHeader);
        if(udpHeader.dport != ZERGPORT)
        {
            skipAhead(fp, 1, "Invalid Destination port", skipBytes);
            continue;
        }

        setZergH(fp, &zHeader, "Zerg Header");
        skipBytes -= sizeof(zHeader);
        if(zHeader.details.version != 1)
        {
            skipAhead(fp, 1, "Invalid Zerg Version", skipBytes);
            continue;
        }

        // Printing the correct payload
        switch(getZType(&zHeader))
        {
            case 1:
                err = printZStatus(&zHeader, fp);
                break;
            case 3:
                // Doesn't needs to take fp
                err = printZGPS(&zHeader, fp);
                break;

            default:
                fprintf(stderr, "Invalid Zerg payload, Skipping packet\n");
        }

        // Checking if there were any errors in printing
        if(err)
        {
            fprintf(stderr, "An output error occurred, Skipping packet\n"); 
        }

        // Reading any extra data 
        dataLength = (ftell(fp) - dataLength);
        if(dataLength != ppHeader.length)
        {
            skipAhead(fp, 0, "", (ppHeader.length - dataLength));
        }

    }
    
    fclose(fp);
    return 0;
}
