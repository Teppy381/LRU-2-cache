#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define STARTING_PAGE_AMOUNT 10


struct CyclicQueue
{
    int  page_num;
    size_t  anchor;
    size_t  size;
    unsigned long* data;  // last K times when page_num was called
};

struct HIST
{
    size_t size;
    size_t next_free;     // because of that we know which parts of   struct CyclicQueue* pages   actually store data
    struct CyclicQueue* pages;
};

struct Cache
{
    size_t  K;            // - it's LRU-K cache
    size_t  size;
    int* data;
    struct HIST* HIST;
};

struct Cache* CacheConstruct(size_t cache_size, size_t K);

int CacheExpandHIST(struct Cache* cache_p, size_t new_HIST_size);

int CacheDestruct(struct Cache* cache_p);

size_t FindCyclicPosition(struct CyclicQueue* page, int position_relative_to_anchor);

int UpdatePageHist(struct CyclicQueue* page, unsigned long current_time);

size_t FindPageInHIST(struct HIST* HIST, int page_num);

int AddPageToHIST(struct Cache* cache_p, int page_num, unsigned long current_time);

size_t FindVictim(struct Cache* cache_p);

int ReplaceVictim(struct Cache* cache_p, size_t victim, int new_page_num);

size_t FindPageInCache(struct Cache* cache_p, int page_num);

int CacheCall(struct Cache* cache_p, int new_page_num, unsigned long current_time);

int PrintCacheData(struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color);

#endif
