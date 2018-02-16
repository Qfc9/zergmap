/*  zergHeaders.h  */

#ifndef ZERGHEADERS_H
#define ZERGHEADERS_H

#define ZERGPORT 0xea7

const char     *zergHKey[4];
const char     *zergMsgKey[1];
const char     *zergStatKey[5];
const char     *zergGPSKey[6];
const char     *zergComKey[2];

union zergH
{

    struct details
    {
        unsigned int    type:4;
        unsigned int    version:4;
        unsigned int    length:24;
        unsigned int    source:16;
        unsigned int    destination:16;
        unsigned int    sequence:32;
    } details;

    char            raw[12];

} header;

struct statusH
{
    int             hp:24;
    unsigned int    armor:8;
    unsigned int    maxHp:24;
    unsigned int    type:8;
    float           speed;
} status;

struct commandH
{

    unsigned int    command:16;
    unsigned int    par1:16;
    unsigned int    par2;

} command;

struct gpsH
{

    double          longitude;
    double          latitude;
    float           altitude;
    float           bearing;
    float           speed;
    float           accuracy;

} gps;

struct DMS
{
    unsigned int    degrees;
    unsigned int    minutes;
    unsigned int    seconds;
} DMS;

struct GPS
{
    struct DMS      lat;
    struct DMS      lon;
};

void            setZergH(
    FILE * fp,
    union zergH *zHead,
    const char *msg);
int             setZGPS(
    FILE * fp,
    struct gpsH *gps,
    size_t length);
int             setZStatus(
    FILE * fp,
    struct statusH *status,
    size_t length);
int             setZCommand(
    FILE * fp,
    struct commandH *command,
    size_t length);
int             setZMsg(
    char *msg,
    int length,
    FILE * fp);
int             getZType(
    union zergH *zHead);

#endif
