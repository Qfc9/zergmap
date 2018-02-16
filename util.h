/*  util.h  */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

// Reading from a file
void            safeRead(
    FILE * fp,
    void *readIt,
    size_t sz,
    const char *msg);

// Writing to a file
void            safeWrite(
    FILE * fp,
    void *writeIt,
    size_t sz,
    const char *msg);

// Skipping ahead in a file
void            skipAhead(
    FILE * fp,
    int err,
    const char *msg,
    int skip);

// Swapping of 64 bit numbers
void            s64BitSwap(
    double *reverseMe);

// Swapping of 32 bit numbers
void            s32BitSwap(
    void *reverseMe);

// Swapping of 24 bit numbers
int             s24BitSwap(
    int *reverseMe);

// Swapping of 32 bit unsigned numbers
unsigned int    u32BitSwap(
    unsigned int swapMe);

// Swapping of 24 bit unsigned numbers
unsigned int    u24BitSwap(
    unsigned int swapMe);

// Swapping of 16 bit unsigned numbers
unsigned int    u16BitSwap(
    unsigned int swapMe);

// Swapping of 8 bit unsigned numbers
unsigned int    u8BitSwap(
    unsigned int swapMe);

// Make everything in a string lowercase
void            toLowerStr(
    char *str);

// Removes non alpha numeric chars
void            removeNonChar(
    char *str);

// Removes all text before the ':'
int             removeHeaderText(
    char *str);

/*
 * To find the distance between two gps coordinates
 * Dist() is from the following website
 * URL: https://rosettacode.org/wiki/Haversine_formula
 * Author: unknown
 * Date: unknown
 */
double          dist(
    struct gpsH *a,
    struct gpsH *b);

// Making the double into Degs/Mins/Secs
void            setGPSDMS(
    double *direction,
    struct DMS *dms);

// Returning if is it a valid GPS struct or not
bool            notValidGPS(
    struct gpsH *gps);

// Checking if float is in bounds for altitude
bool            isAltitude(
    float a);

// Checking if double is in bounds for latitude
bool            isLatitude(
    double l);

// Checking if double is in bounds for longitude
bool            isLongitude(
    double l);

#endif
