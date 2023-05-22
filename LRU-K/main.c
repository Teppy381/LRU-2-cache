#include "functions.h"


// for demonstration mode use   make LRU-K-DEMO

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        FileCacheTrial(argv[1]);
    }
    else
    {
        StandartCacheTrial();
    }
}

