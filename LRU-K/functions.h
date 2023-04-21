#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>

#define STARTING_PAGE_AMOUNT 10

struct HIST
{
    int   size;
    int   next_free;
    int** data;
};

struct Cache
{
    int  K;            // - it's LRU-K cache
    struct HIST* HIST;
    int  size;
    int* data;
};

int CacheConstruct(struct Cache* cache_p, int cache_size);

int CacheExpandHIST(struct Cache* cache_p, int new_HIST_size);

int CacheDestruct(struct Cache* cache_p);


#endif
