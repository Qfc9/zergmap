#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zergHeaders.h"
#include "netHeaders.h"
#include "util.h"
#include "graph.h"

#define MINPCAPLENGTH 54

int main(int argc, char *argv[])
{
    // Initializing Variables
    FILE *fp;
    struct pcapFileH pHeader;
    struct pcapPacketH ppHeader;
    union ethernetH eHeader;
    struct ipv4H ipHeader;
    struct ipv6H ip6Header;
    struct udpH udpHeader;
    union zergH zHeader;
    struct gpsH zGPS;
    struct statusH zStatus;
    int err = 0;
    int swap = 0;
    long int dataLength = 0;
    unsigned int skipBytes = 0;

    // Setting getopt to not display errors
    opterr = 0;
    int optCode;
    int minHp = 10;

    // Looping through each flag
    while ((optCode = getopt(argc, argv, "h:")) != -1)
    {
        switch (optCode)
        {
        case 'h':
            minHp = strtol(optarg, NULL, 10);
            break;
        default:
            fprintf(stderr, "Unknown flag -%c\n", optopt);
            return 1;
        }
    }

    // Checking for valid amount for args
    if((argc - optind) == 0)
    {
        fprintf(stderr, "Invalid amount of args\n");
        return 1;
    }

    // Creating the graph
    graph zergGraph = graphCreate();
    if (!zergGraph)
    {
        fprintf(stderr, "Calloc Error!\n");
        return 1;
    }

    for (int i = optind; i < argc; i++)
    {
        // Attempting to open the file given
        fp = fopen(argv[i], "r");
        if(fp == NULL)
        {
            fprintf(stderr, "Unable to open the file: %s\n", argv[1]);
            graphDestroy(zergGraph);
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
            graphDestroy(zergGraph);
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
                    if((zHeader.details.length - 24) < 0)
                    {
                        err = 1;
                        break;
                    }

                    // Adding a status to the graph
                    setZStatus(fp, &zStatus, sizeof(zStatus));
                    err = graphAddStatus(zergGraph, zHeader, zStatus);
                    break;
                case 3:
                    // Adding a Zerg to the graph
                    setZGPS(fp, &zGPS, sizeof(zGPS));
                    err = graphAddNode(zergGraph, zHeader, &zGPS);
                    break;

                default:
                    fprintf(stderr, "Invalid Zerg payload, Skipping packet\n");
            }

            // Checking if there were any errors in printing
            if(err == 2)
            {
                fprintf(stderr, "Duplicate Zerg Ids! Exiting...\n"); 
                break;
            }
            else if(err > 0)
            {
                fprintf(stderr, "A payload error occurred, Skipping packet\n");
            }

            // Reading any extra data 
            dataLength = (ftell(fp) - dataLength);
            if(dataLength != ppHeader.length)
            {
                skipAhead(fp, 0, "", (ppHeader.length - dataLength));
            }

        }
        
        fclose(fp);
        if(err == 2)
        {
            graphDestroy(zergGraph);
            return 2;
        }
    }

    // Removing incomplete zerg items
    graphRemoveBadNodes(zergGraph);

    // Analyzing the graph
    graphAnalyzeGraph(zergGraph);

    // Printing Graph information
    graphPrint(zergGraph);
    graphPrintLowHP(zergGraph, minHp);

    // Disassembling the graph
    graphDestroy(zergGraph);

    return 0;
}
