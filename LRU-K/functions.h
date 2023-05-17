#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>


#define STARTING_PAGE_AMOUNT 8
#define STARTING_TABLE_SIZE  8
#define STARTING_LIST_SIZE   4

#define MEM_CHECK(x)                                                                        \
do {                                                                                        \
    if (x == NULL)                                                                          \
    {                                                                                       \
        fprintf(stderr, "\nIn function <%s> memory exhausted\n", __PRETTY_FUNCTION__);      \
        exit(-1);                                                                           \
    }                                                                                       \
} while (0)                                                                                 \


typedef int CacheType;
#define CachePrintType "%i"

struct CyclicQueue;

struct HIST;

struct CacheCell;

struct ListNode;

struct List;

struct Cache;


struct Cache* CacheConstruct(size_t cache_size, size_t K);

int ExpandList(struct List* list_p, size_t new_list_size);

int ExpandHIST(struct Cache* cache_p, size_t new_HIST_size);

int CacheDestruct(struct Cache* cache_p);

size_t FindCyclicPosition(const struct CyclicQueue* page, int position_relative_to_anchor);

int UpdatePageHist(struct CyclicQueue* page, unsigned long current_time);

size_t FindPageInHIST(const struct HIST* HIST, int page_num);

int AddPageToHIST(struct Cache* cache_p, int page_num, unsigned long current_time);

size_t FindVictim(const struct Cache* cache_p);

int ReplaceVictim(struct Cache* cache_p, size_t victim, int new_page_num);

size_t FindPageInCache(const struct Cache* cache_p, int page_num);

int CacheCall(struct Cache* cache_p, int new_page_num, unsigned long current_time);

int PrintCacheData(const struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color);

#endif
