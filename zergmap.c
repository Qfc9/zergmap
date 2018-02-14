#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zergHeaders.h"
#include "netHeaders.h"
#include "zergDecode.h"
#include "util.h"
#include "graph.h"

#define MINPCAPLENGTH 54

int main(int argc, char *argv[])
{
    // Initializing Variables
    FILE *fp;
    struct pcapFileH pHeader;
    struct pcapPacketH ppHeader;
    union zergH zHeader;
    struct gpsH zGPS;
    struct statusH zStatus;
    int err = 0;
    int swap = 0;
    long int dataLength = 0;
    unsigned int skipBytes = 0;

    // Setting getopt to not display errors
    opterr = 0;
    int optCode;
    int minHp = 10;

    // Looping through each flag
    while ((optCode = getopt(argc, argv, "h:")) != -1)
    {
        switch (optCode)
        {
        case 'h':
            minHp = strtol(optarg, NULL, 10);
            break;
        default:
            fprintf(stderr, "Unknown flag -%c\n", optopt);
            return 1;
        }
    }

    // Checking for valid amount for args
    if((argc - optind) == 0)
    {
        fprintf(stderr, "Invalid amount of args\n");
        return 1;
    }

    // Creating the graph
    graph zergGraph = graphCreate();
    if (!zergGraph)
    {
        fprintf(stderr, "Calloc Error!\n");
        return 1;
    }

    for (int i = optind; i < argc; i++)
    {
        // Attempting to open the file given
        fp = fopen(argv[i], "r");
        if(fp == NULL)
        {
            fprintf(stderr, "Unable to open the file: %s\n", argv[1]);
            graphDestroy(zergGraph);
            return 1;
        }

        // Reading the first header of the file
        if(setPcapHead(fp, &pHeader, "Packet is corrupted or empty"))
        {
            swap = 1;
        }

        // Checking for valid PCAP Header
        if((pHeader.majVer != PCAPHEADMAJ) || (pHeader.minVer != PCAPHEADMIN) || (pHeader.linkType != PCAPHEADLINK))
        {
            fprintf(stderr, "Invalid PCAP Version\n");
            graphDestroy(zergGraph);
            fclose(fp);
            return 1;
        }

        // Main reading loop
        while(setPacketHead(fp, &ppHeader, swap))
        {
            skipBytes = ppHeader.length;
            dataLength = ftell(fp);

            if(invalidEthernetHeader(fp, ppHeader.length, &skipBytes))
            {
                continue;
            }

            if(invalidZergHeader(fp, &zHeader, &skipBytes))
            {
                continue;
            }

            // Printing the correct payload
            switch(getZType(&zHeader))
            {
                case 1:
                    if((zHeader.details.length - 24) < 0)
                    {
                        err = 1;
                        break;
                    }

                    // Adding a status to the graph
                    setZStatus(fp, &zStatus, sizeof(zStatus));
                    err = graphAddStatus(zergGraph, zHeader, zStatus);
                    break;
                case 3:
                    // Adding a Zerg to the graph
                    setZGPS(fp, &zGPS, sizeof(zGPS));
                    err = graphAddNode(zergGraph, zHeader, &zGPS);
                    break;

                default:
                    fprintf(stderr, "Invalid Zerg payload, Skipping packet\n");
            }

            // Checking if there were any errors in printing
            if(err == 2)
            {
                fprintf(stderr, "Duplicate Zerg Ids! Exiting...\n"); 
                break;
            }
            else if(err > 0)
            {
                fprintf(stderr, "A payload error occurred, Skipping packet\n");
            }

            // Reading any extra data 
            dataLength = (ftell(fp) - dataLength);
            if(dataLength != ppHeader.length)
            {
                skipAhead(fp, 0, "", (ppHeader.length - dataLength));
            }

        }
        
        fclose(fp);
        if(err == 2)
        {
            graphDestroy(zergGraph);
            return 2;
        }
    }

    // Removing incomplete zerg items
    graphRemoveBadNodes(zergGraph);

    // Analyzing the graph
    graphAnalyzeGraph(zergGraph);

    // Printing Graph information
    graphPrint(zergGraph);
    graphPrintLowHP(zergGraph, minHp);

    // Disassembling the graph
    graphDestroy(zergGraph);

    return 0;
}
