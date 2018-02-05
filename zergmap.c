#include <stdio.h>

#include "decode.h"
#include "graph.h"

// This is my main function
int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Invalid amount of args\n");
        return 1;
    }

    decode(argc, argv);

    return 0;
}
