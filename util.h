/*  util.h  */

#ifndef UTIL_H
#define UTIL_H

void safeRead(FILE *fp, void *readIt, size_t sz, const char *msg);
void safeWrite(FILE *fp, void *writeIt, size_t sz, const char *msg);
void skipAhead(FILE *fp, int err, const char *msg, int skip);
void s64BitSwap(double *reverseMe);
void s32BitSwap(void *reverseMe);
int s24BitSwap(int *reverseMe);
unsigned int u32BitSwap(unsigned int swapMe);
unsigned int u24BitSwap(unsigned int swapMe);
unsigned int u16BitSwap(unsigned int swapMe);
unsigned int u8BitSwap(unsigned int swapMe);
void toLowerStr(char *str);
void removeNonChar(char *str);
int removeHeaderText(char *str);

#endif
