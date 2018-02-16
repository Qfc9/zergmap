/*  zergDecode.h  */

#ifndef ZERGDECODE_H
#define ZERGDECODE_H

#include <stdbool.h>

// Reading in ethernet and ip headers and returning true if something is invalid
bool invalidEthOrIp(FILE *fp, unsigned int ppLength, unsigned int *skipBytes);

// Reading in Zerg Header and returning true if header is invalid
bool invalidZergHeader(FILE *fp, union zergH *zHeader, unsigned int *skipBytes);

// Reading in PCAP header and returning true if it's invalid 
bool invalidPCAPHeader(FILE *fp, int *swap);

#endif
