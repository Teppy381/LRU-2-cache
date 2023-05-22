#include "functions.h"
#include <unistd.h>
#include <time.h>

// for demonstration mode use   make LRU-K-DEMO

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        clock_t begin = clock();
        FileCacheTrial(argv[1]);
        clock_t end = clock();
        printf("Time spent %g seconds\n", (double) (end - begin) / CLOCKS_PER_SEC);
    }
    else
    {
        StandartCacheTrial();
    }
}

