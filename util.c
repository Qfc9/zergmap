/*  util.c  */
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "zergHeaders.h"
#include "util.h"

#define R 6371.0
#define TO_RAD (3.1415926536 / 180)

// Checking if double is in bounds for longitude
bool
isLongitude(
    double l)
{
    if (l > 180.0 || l < -180.0 || isnan(l))
    {
        return true;
    }

    return false;
}

// Checking if double is in bounds for latitude
bool
isLatitude(
    double l)
{
    if (l > 90.0 || l < -90.0 || isnan(l))
    {
        return true;
    }

    return false;
}

// Checking if float is in bounds for altitude
bool
isAltitude(
    float a)
{
    if (a > 11265.4 || a < -11265.4 || isnan(a))
    {
        return true;
    }

    return false;
}

// Returning if is it a valid GPS struct or not
bool
notValidGPS(
    struct gpsH * gps)
{
    if (!gps)
    {
        return true;
    }
    if (isLatitude(gps->latitude) || isLongitude(gps->longitude) ||
        isAltitude(gps->altitude))
    {
        return true;
    }
    return false;
}

/*
 * To find the distance between two gps coordinates
 * Dist() is from the following website
 * URL: https://rosettacode.org/wiki/Haversine_formula
 * Author: unknown
 * Date: Feb 5th 2018
 */
double
dist(
    struct gpsH *a,
    struct gpsH *b)
{
    double          th1 = a->latitude;
    double          ph1 = a->longitude;
    double          th2 = b->latitude;
    double          ph2 = b->longitude;

    double          dx, dy, dz;

    ph1 -= ph2;
    ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

    dz = sin(th1) - sin(th2);
    dx = cos(ph1) * cos(th1) - cos(th2);
    dy = sin(ph1) * cos(th1);

    return (asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R) * 1000;
}

// Making the double into Degs/Mins/Secs
void
setGPSDMS(
    double *direction,
    struct DMS *dms)
{
    dms->degrees = *direction;
    dms->minutes = 60 * (*direction - dms->degrees);
    dms->seconds =
        round(3600 * (*direction - dms->degrees) - 60 * dms->minutes);
}

// Writing to a file
void
safeWrite(
    FILE * fp,
    void *writeIt,
    size_t sz,
    const char *msg)
{
    if (fwrite(writeIt, sz, 1, fp) == 0)
    {
        fprintf(stderr, "READ ERROR AT: %s\n", msg);
        fclose(fp);
        exit(1);
    }
}

// Reading from a file
void
safeRead(
    FILE * fp,
    void *readIt,
    size_t sz,
    const char *msg)
{
    if (fread(readIt, sz, 1, fp) == 0)
    {
        fprintf(stderr, "READ ERROR AT: %s\n", msg);
        fclose(fp);
        exit(1);
    }
}

// Skipping ahead in a file
void
skipAhead(
    FILE * fp,
    int err,
    const char *msg,
    int skip)
{
    if (err)
    {
        fprintf(stderr, "Skipping Packet: %s\n", msg);
    }

    if (fseek(fp, skip, SEEK_CUR))
    {
        fprintf(stderr, "Read Error Occurred\n");
        fclose(fp);
        exit(1);
    }
}

// Swapping of 8 bit unsigned numbers
unsigned int
u8BitSwap(
    unsigned int swapMe)
{
    return (swapMe >> 4) | (swapMe << 4);
}

// Swapping of 16 bit unsigned numbers
unsigned int
u16BitSwap(
    unsigned int swapMe)
{
    return (swapMe >> 8) | (swapMe << 8);
}

// Swapping of 24 bit unsigned numbers
unsigned int
u24BitSwap(
    unsigned int swapMe)
{
    return (((swapMe >> 24) & 0xff) | ((swapMe << 8) & 0xff0000) |
            ((swapMe >> 8) & 0xff00) | ((swapMe << 24) & 0xff000000)) >> 8;
}

// Swapping of 32 bit unsigned numbers
unsigned int
u32BitSwap(
    unsigned int swapMe)
{
    return ((swapMe >> 24) & 0xff) |    // move byte 3 to byte 0
        ((swapMe << 8) & 0xff0000) |    // move byte 1 to byte 2
        ((swapMe >> 8) & 0xff00) |  // move byte 2 to byte 1
        ((swapMe << 24) & 0xff000000);  // byte 0 to byte 3
}

// Swapping of 64 bit numbers
void
s64BitSwap(
    double *reverseMe)
{

    char            data[8];

    memcpy(data, reverseMe, 8);

    double          result;

    char           *dest = (char *) &result;

    for (unsigned int i = 0; i < sizeof(double); i++)
    {
        dest[i] = data[sizeof(double) - i - 1];
    }

    *reverseMe = result;
}

// Swapping of 32 bit numbers
void
s32BitSwap(
    void *reverseMe)
{

    char            data[4];

    memcpy(data, reverseMe, 4);

    unsigned int    result;

    char           *dest = (char *) &result;

    for (unsigned int i = 0; i < 4; i++)
    {
        dest[i] = data[4 - i - 1];
    }

    memcpy(reverseMe, &result, 4);
}

// Swapping of 24 bit numbers
int
s24BitSwap(
    int *reverseMe)
{
    char            data[4];

    memcpy(data, reverseMe, 4);

    int             result;

    char           *dest = (char *) &result;

    for (unsigned int i = 0; i < sizeof(int); i++)
    {
        dest[i] = data[sizeof(int) - i - 1];
    }
    return result >> 8;
}

// Make everything in a string lowercase
void
toLowerStr(
    char *str)
{
    for (unsigned int i = 0; i < strlen(str); i++)
    {
        if (isalpha(str[i]))
        {
            str[i] = tolower(str[i]);
        }
    }
}

// Removes non alpha numeric chars
void
removeNonChar(
    char *str)
{
    for (unsigned int i = 0; i < strlen(str); i++)
    {
        if (isalnum(str[i]) == 0)
        {
            str[i] = 0;
        }
    }
}

// Removes all text before the ':'
int
removeHeaderText(
    char *str)
{
    bool            found = false;
    bool            isContent = false;
    int             counter = 0;

    for (unsigned int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ':' && isContent == false)
        {
            found = true;

        }
        else if ((found && isascii(str[i])) && (isspace(str[i]) == 0))
        {
            isContent = true;
        }

        if (isContent)
        {
            str[counter] = str[i];
            counter++;
        }
    }

    if (found)
    {
        str[counter] = '\0';
        return 0;
    }

    return 1;
}
