/*  zergHeaders.c  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "zergHeaders.h"
#include "zergPrint.h"
#include "util.h"

// Initializing variables
const char *zergHKey[4] = { "version", "sequence", "from", "to" };
const char *zergMsgKey[1] = { "message" };
const char *zergStatKey[5] = { "name", "hp", "type", "armor", "maxspeed" };
const char *zergGPSKey[6] = { "latitude", "longitude", "altitude", "bearing", "speed", "accuracy" };
const char *zergComKey[2] = { "par1", "par2" };

// Reading in data based off it's length
int setZMsg(char *msg, int length, FILE *fp)
{
    char *tmp = malloc(length);
    if(!tmp)
    {
        free(tmp);
        return 1;
    }

    fread(tmp, length, 1, fp);
    strncpy(msg, tmp, length);

    free(tmp);

    return 0;
}

// Reading in and setting Command Struct
int setZCommand(FILE *fp, struct commandH *command, size_t length)
{
    fread(command, length, 1, fp);

    command->command = u16BitSwap(command->command);
    if((command->command % 2) == 0 || command->command == 0)
    {
        fseek(fp, -6, SEEK_CUR);
    }
    command->par1 = u16BitSwap(command->par1);
    s32BitSwap(&command->par2);

    return 0;
}

// Reading in and setting Header Stuct
void setZergH(FILE *fp, union zergH *zHead, const char *msg)
{
 
    safeRead(fp, zHead, sizeof(*zHead), msg);

    zHead->details.length = u24BitSwap(zHead->details.length);
    zHead->details.source = u16BitSwap(zHead->details.source);
    zHead->details.destination = u16BitSwap(zHead->details.destination);
    zHead->details.sequence = u32BitSwap(zHead->details.sequence);
}

// Reading in and setting Status Header
int setZStatus(FILE *fp, struct statusH *status, size_t length)
{
 
    fread(status, length, 1, fp);

    status->hp = u24BitSwap(status->hp);
    status->armor = u8BitSwap(status->armor);
    status->maxHp = u24BitSwap(status->maxHp);
    s32BitSwap(&status->speed);

    return 0;

}

// Reading in and setting Zerg Header
int setZGPS(FILE *fp, struct gpsH *gps, size_t length)
{

    fread(gps, length, 1, fp);

    s64BitSwap(&gps->longitude);
    s64BitSwap(&gps->latitude);
    s32BitSwap(&gps->altitude);
    s32BitSwap(&gps->bearing);
    s32BitSwap(&gps->speed);
    s32BitSwap(&gps->accuracy);

    return 0;

}

// Returing the header Type
int getZType(union zergH *zHead)
{
    return zHead->details.type;
}
