/*  zergDecode.h  */

#ifndef ZERGDECODE_H
#define ZERGDECODE_H

#include <stdbool.h>

bool invalidEthernetHeader(FILE *fp, unsigned int ppLength, unsigned int *skipBytes);
bool invalidZergHeader(FILE *fp, union zergH *zHeader, unsigned int *skipBytes);

#endif
