#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <climits>

#define STARTING_PAGE_AMOUNT 10


struct CyclicQueue
{
    int  page_num;
    int  anchor;
    int  size;
    int* data;
};

struct HIST
{
    int size;
    int next_free;     // because of that we know which parts of   struct CyclicQueue* pages   actually store data
    struct CyclicQueue* pages;
};

struct Cache
{
    int  K;            // - it's LRU-K cache
    int  size;
    int* data;
    struct HIST* HIST;
};

int CacheConstruct(struct Cache* cache_p, int cache_size, int K);

int CacheExpandHIST(struct Cache* cache_p, int new_HIST_size);

int CacheDestruct(struct Cache* cache_p);

int UpdatePageHist(struct CyclicQueue* page, int current_time);

int FindPageInHIST(struct HIST* HIST, int page_num);

int AddPageToHIST(struct Cache* cache_p, int page_num, int current_time);

int FindVictim(struct Cache* cache_p);

int ReplaceVictim(struct Cache* cache_p, int victim, int new_page_num);

int MainAlgorythm(struct Cache* cache_p, int new_page_num, int current_time);
#endif
