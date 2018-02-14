/*  zergDecode.h  */

#ifndef ZERGDECODE_H
#define ZERGDECODE_H

#include <stdbool.h>

bool invalidEthernetHeader(FILE *fp, unsigned int ppLength, unsigned int *skipBytes);

#endif
