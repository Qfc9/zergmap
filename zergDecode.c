#define _XOPEN_SOURCE
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "zergHeaders.h"
#include "netHeaders.h"
#include "util.h"
#include "zergDecode.h"

#define MINPCAPLENGTH 54

bool invalidEthernetHeader(FILE *fp, unsigned int ppLength, unsigned int *skipBytes)
{
    union ethernetH eHeader;
    struct ipv4H ipHeader;
    struct ipv6H ip6Header;

    // Checking if packet is of a valid length
    if(ppLength < MINPCAPLENGTH)
    {
        skipAhead(fp, 1, "Invalid Packet Header", (*skipBytes));
        return true;
    }

    // Reading Ethernet Header
    setEthHead(fp, &eHeader, "Ethernet Header");   
    (*skipBytes) -= (sizeof(eHeader) + ETHCORRECTION);

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
        (*skipBytes) -= sizeof(ipHeader);

        // Checking if valid IP Header
        if(ipHeader.version != IPV4 || ipHeader.proto != UDP || ipHeader.ihl < IHLDEFAULT)
        {
            skipAhead(fp, 1, "Invalid IPv4 Header", (*skipBytes));
            return true;
        }
        // Moving cursors forward if there are options
        else if(ipHeader.ihl > IHLDEFAULT)
        {
            unsigned int ihl = ((ipHeader.ihl - IHLDEFAULT) * 4);
            if(ppLength < (MINPCAPLENGTH + ihl))
            {
                (*skipBytes) -= ihl;
                fseek(fp, ihl, SEEK_CUR);
            }
            else
            {
                skipAhead(fp, 1, "Invalid Packet Header Data length", (*skipBytes));
                return true;
            }
        }
    }
    // Checking if it is IPv6
    else if(eHeader.ethInfo.type == ETHIPV6)
    {
        //ip6Header
        setIPv6Head(fp, &ip6Header, "IPv6 Header");
        (*skipBytes) -= sizeof(ip6Header);

        // Checking if valid IP Header
        if(ip6Header.nextHead != UDP)
        {
            skipAhead(fp, 1, "Invalid Transport Layer protocol", (*skipBytes));
            return true;
        }
    }
    else
    {
        skipAhead(fp, 1, "Invalid Ethernet Header Type", (*skipBytes));
        return true;

    }

    return false;
}