/*  zergPrint.h  */

#ifndef ZERGPRINT_H
#define ZERGPRINT_H

#include <stdbool.h>

const char *zergBreeds[16];
const char *zergCommands[8];

int printZStatus(union zergH *zHead, FILE *fp);
int printZMessage(union zergH *zHead, FILE *fp);
int printZGPS(union zergH *zHead, FILE *fp);
int printZCommand(union zergH *zHead, FILE *fp);
void printZHeader(union zergH *zHead);

#endif
