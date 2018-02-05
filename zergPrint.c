/*  zergPrint.c  */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "zergHeaders.h"
#include "util.h"
#include "zergPrint.h"

// Initializing Variables
const char *zergBreeds[16] = { "Overmind", "Larva", "Cerebrate", "Overload", 
                    "Queen", "Drone", "Zergling", "Lurker", "Broodling",
                    "Hyralisk", "Guardian", "Scourge", "Ultralisk", 
                    "Mutalisk", "Defiler", "Devourer" };

const char *zergCommands[8] = { "GET_STATUS", "GOTO", "GET_GPS", "RESERVED",
                                "RETURN", "SET_GROUP", "STOP", "REPEAT" };

// Printing Message payload
int printZMessage(union zergH *zHead, FILE *fp)
{
    int length = (zHead->details.length - 12);
    if(length <= 0)
    {
        return 1;
    }

    char *message = malloc(length + 1);
    if(!message)
    {
        return 1;
    }

    strncpy(message, "a", length + 1);
    
    fread(message, length, 1, fp);

    printZHeader(zHead);

    printf("Message : ");
    for(int i = 0; i < length; i++)
    {
        if(message[i] == 10)
        {
            printf("\nMessage : ");
        }
        else
        {
            printf("%c", message[i]);
        }
    }
    printf("\n\n");

    free(message);

    return 0;
}

// Printing Command payload
int printZCommand(union zergH *zHead, FILE *fp)
{
    struct commandH command;
    float theF = 0;

    if(setZCommand(fp, &command, sizeof(command)))
    {
        return 1;
    }

    memcpy(&theF, &command.par2, 4);

    printZHeader(zHead);
    printf("%s\n", zergCommands[command.command]);
    if(command.command == 1)
    {
        printf("Par1    :%d\n", command.par1);
        printf("Par2    :%f\n", theF);
    }
    else if(command.command == 5)
    {
        if(command.par1)
        {
            printf("Par1    : True\n");
        }
        else
        {
            printf("Par1    : False\n");
        }
        printf("Par2    : %d\n", (signed int) command.par2);
    }
    else if(command.command == 7)
    {
        printf("Par1    : %d\n", command.par1);
        printf("Par2    : %u\n", command.par2);
    }
    printf("\n");
    

    return 0;
}

// Printing Status Payload
int printZStatus(union zergH *zHead, FILE *fp)
{
    struct statusH status;
    int length = (zHead->details.length - 24);
    if(length <= 0)
    {
        return 1;
    }

    char *name = malloc(length + 1);
    if(setZStatus(fp, &status, sizeof(status)) | (!name))
    {
        return 1;
    }

    strncpy(name, "a", length + 1);

    fread(name, length, 1, fp);

    printZHeader(zHead);
    printf("Name    : %s\n", name);
    printf("HP      : %d/%d\n", status.hp, status.maxHp);
    printf("Type    : %s\n", zergBreeds[status.type]);
    printf("Armor   : %d\n", status.armor);
    printf("MaxSpeed: %.4fm/s\n\n", status.speed);

    free(name);

    return 0;
}

// Printing GPS Payload
int printZGPS(union zergH *zHead, FILE *fp)
{
    struct gpsH gpsHead;
    struct GPS gps;

    char latChar = 'N';
    char lonChar = 'E';

    if(setZGPS(fp, &gpsHead, sizeof(gpsHead)))
    {
        return 1;
    }

    if(gpsHead.latitude < 0)
    {
        latChar = 'S';
        gpsHead.latitude *= -1;
    }
    if(gpsHead.longitude < 0)
    {
        lonChar = 'W';
        gpsHead.longitude *= -1;
    }

    setGPSDMS(&gpsHead.latitude, &gps.lat);
    setGPSDMS(&gpsHead.longitude, &gps.lon);

    printZHeader(zHead);
    printf("Latitude : %u° %u' %u\" %c\n", gps.lat.degrees, gps.lat.minutes, gps.lat.seconds, latChar);
    printf("Longitude: %u° %u' %u\" %c\n", gps.lon.degrees, gps.lon.minutes, gps.lon.seconds, lonChar);
    printf("Altitude : %.1fm\n", (gpsHead.altitude * 1.8288));
    printf("Bearing  : %.9f deg.\n", gpsHead.bearing);
    printf("Speed    : %.fkm/h\n", (gpsHead.speed) * 3.6);
    printf("Accuracy : %.fm\n\n", gpsHead.accuracy);

    return 0;
}

// Printing Zerg Header
void printZHeader(union zergH *zHead)
{
    printf("Version : %u\n", zHead->details.version);
    printf("Sequence: %u\n", zHead->details.sequence);
    printf("From    : %u\n", zHead->details.source);
    printf("To      : %u\n", zHead->details.destination);
}