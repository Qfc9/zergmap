/*  netHeaders.c  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "zergHeaders.h"
#include "util.h"
#include "netHeaders.h"

// Setting and writing all the headers to the pcap
void
setAllHeaders(
    int payloadLenth,
    FILE * fp,
    struct pcapPacketH *packetHead,
    union ethernetH *ethHead,
    struct ipv4H *ipHead,
    struct udpH *udpHead)
{
    const char     *msg = "Writing All Headers";

    setPacketHeadDefault(packetHead, 42 + payloadLenth);
    setEthHeadDefault(ethHead);
    setIPHeadDefault(ipHead, 28 + payloadLenth);
    setUDPHeadDefault(udpHead, 8 + payloadLenth);
    safeWrite(fp, packetHead, sizeof(*packetHead), msg);
    safeWrite(fp, ethHead, sizeof(*ethHead), msg);
    skipAhead(fp, 0, "", ETHCORRECTION);
    safeWrite(fp, ipHead, sizeof(*ipHead), msg);
    safeWrite(fp, udpHead, sizeof(*udpHead), msg);
}

// Setting the UDP headers
void
setUDPHead(
    FILE * fp,
    struct udpH *udpHead,
    const char *msg)
{
    safeRead(fp, udpHead, sizeof(*udpHead), msg);
    udpHead->dport = u16BitSwap(udpHead->dport);
}

// Settings IPv6 Header
void
setIPv6Head(
    FILE * fp,
    struct ipv6H *ipHead,
    const char *msg)
{
    safeRead(fp, ipHead, sizeof(*ipHead), msg);
    ipHead->nextHead = u8BitSwap(ipHead->nextHead);
}

// Setting IPv4 header
void
setIPv4Head(
    FILE * fp,
    struct ipv4H *ipHead,
    const char *msg)
{
    safeRead(fp, ipHead, sizeof(*ipHead), msg);
}

// Setting Ethernet header
void
setEthHead(
    FILE * fp,
    union ethernetH *ethHead,
    const char *msg)
{
    safeRead(fp, ethHead, sizeof(*ethHead), msg);
    skipAhead(fp, 0, "", ETHCORRECTION);
    ethHead->ethInfo.type = u16BitSwap(ethHead->ethInfo.type);
}

// Setting the pcap header
int
setPcapHead(
    FILE * fp,
    struct pcapFileH *pHead,
    const char *msg)
{
    int             swap = 0;

    safeRead(fp, pHead, sizeof(*pHead), msg);

    if (pHead->fileType == PCAPFILETYPE)
    {
        pHead->fileType = u32BitSwap(pHead->fileType);
        pHead->majVer = u16BitSwap(pHead->majVer);
        pHead->minVer = u16BitSwap(pHead->minVer);
        pHead->linkType = u32BitSwap(pHead->linkType);
        swap = 1;
    }

    return swap;
}

// Setting the Packet header and swapping in nessasary
int
setPacketHead(
    FILE * fp,
    struct pcapPacketH *pHead,
    int swap)
{
    int             err = 1;

    err = fread(pHead, sizeof(*pHead), 1, fp);
    if (swap)
    {
        pHead->length = u32BitSwap(pHead->length);
    }

    return err;
}

// Seting default values for the PCAP Header
void
setPcapHeadDefault(
    struct pcapFileH *pHead,
    int swap)
{
    if (swap)
    {
        pHead->fileType = u32BitSwap(PCAPFILETYPE);
    }
    else
    {
        pHead->fileType = PCAPFILETYPE;
    }

    pHead->majVer = 2;
    pHead->minVer = 4;
    pHead->gmtOffset = 0;
    pHead->accDelta = 0;
    pHead->maxLength = 0;
    pHead->linkType = 1;
}

// Setting default values for the Packet Header
void
setPacketHeadDefault(
    struct pcapPacketH *pHead,
    unsigned int length)
{
    pHead->unixEpoch = 0;
    pHead->microEpoch = 0;
    pHead->length = length;
    pHead->untrunLength = 0;
}

// Setting default values for the Ethernet Header
void
setEthHeadDefault(
    union ethernetH *ethHead)
{
    strncpy(ethHead->raw, "\0", 16);
    ethHead->ethInfo.type = u16BitSwap(ETHIPV4);
}

// Setting default values for the IP Header
void
setIPHeadDefault(
    struct ipv4H *ipHead,
    unsigned int length)
{
    ipHead->version = 4;
    ipHead->ihl = 5;
    ipHead->dscp = 0;
    ipHead->ecn = 0;
    ipHead->length = u16BitSwap(length);
    ipHead->id = 0;
    ipHead->flags = 0;
    ipHead->Offset = 0;
    ipHead->ttl = 0;
    ipHead->proto = UDP;
    ipHead->checksum = ~u16BitSwap(0x4512 + length);
    ipHead->sip = 0;
    ipHead->dip = 0;
}

// Setting default values for the UDP Header
void
setUDPHeadDefault(
    struct udpH *udpHead,
    unsigned int length)
{
    udpHead->sport = 0;
    udpHead->dport = u16BitSwap(ZERGPORT);
    udpHead->length = u16BitSwap(length);
    udpHead->checksum = ~(ZERGPORT + length);
}
