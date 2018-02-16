/*  netHeaders.h  */

#ifndef NETHEADERS_H
#define NETHEADERS_H

#define PCAPFILETYPE 0xD4C3B2A1
#define PCAPHEADMAJ 2
#define PCAPHEADMIN 4
#define PCAPHEADLINK 1

#define ETHCORRECTION -2
#define ETH8021CORRECTION -10
#define ETH8021Q 0x8100
#define ETH8021Q4 0x88A8
#define ETHIPV4 0x0800
#define ETHIPV6 0x86dd

#define IPV4 0x4
#define UDP 0x11
#define IP6INIP4 0x29
#define IHLDEFAULT 0x5

struct pcapFileH
{
    unsigned int    fileType;
    unsigned int    majVer:16;
    unsigned int    minVer:16;
    unsigned int    gmtOffset;
    unsigned int    accDelta;
    unsigned int    maxLength;
    unsigned int    linkType;
} pcapFileH;

struct pcapPacketH
{
    unsigned int    unixEpoch;
    unsigned int    microEpoch;
    unsigned int    length;
    unsigned int    untrunLength;
} pcapPacketH;


union ethernetH
{
    struct ethInfo
    {
        char            dmac[6];
        char            smac[6];
        unsigned int    type:16;
    } ethInfo;
    char            raw[16];

} ethernetH;

struct ipv4H
{
    unsigned int    ihl:4;
    unsigned int    version:4;
    unsigned int    dscp:6;
    unsigned int    ecn:2;
    unsigned int    length:16;
    unsigned int    id:16;
    unsigned int    flags:3;
    unsigned int    Offset:13;
    unsigned int    ttl:8;
    unsigned int    proto:8;
    unsigned int    checksum:16;
    unsigned int    sip:32;
    unsigned int    dip:32;
} ipv4;

struct ipv6H
{
    unsigned int    version:4;
    unsigned int    traffic:8;
    unsigned int    flow:20;
    unsigned int    length:16;
    unsigned int    nextHead:8;
    unsigned int    hop:8;
    char            sip[16];
    char            dip[16];
} ipv6H;

struct udpH
{
    unsigned int    sport:16;
    unsigned int    dport:16;
    unsigned int    length:16;
    unsigned int    checksum:16;
} udp;

void            setPcapHeadDefault(
    struct pcapFileH *pHead,
    int swap);
void            setPacketHeadDefault(
    struct pcapPacketH *pHead,
    unsigned int length);
void            setEthHeadDefault(
    union ethernetH *ethHead);
void            setIPHeadDefault(
    struct ipv4H *ipHead,
    unsigned int length);
void            setUDPHeadDefault(
    struct udpH *udpHead,
    unsigned int length);

int             setPcapHead(
    FILE * fp,
    struct pcapFileH *pHead,
    const char *msg);
int             setPacketHead(
    FILE * fp,
    struct pcapPacketH *pHead,
    int swap);
void            setEthHead(
    FILE * fp,
    union ethernetH *ethHead,
    const char *msg);
void            setIPv4Head(
    FILE * fp,
    struct ipv4H *ipHead,
    const char *msg);
void            setIPv6Head(
    FILE * fp,
    struct ipv6H *ipHead,
    const char *msg);
void            setUDPHead(
    FILE * fp,
    struct udpH *udpHead,
    const char *msg);

void            setAllHeaders(
    int payloadLenth,
    FILE * fp,
    struct pcapPacketH *packetHead,
    union ethernetH *ethHead,
    struct ipv4H *ipHead,
    struct udpH *udpHead);

#endif
