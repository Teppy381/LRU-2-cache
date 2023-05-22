#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>


#define STARTING_HIST_SIZE  100000
#define STARTING_TABLE_SIZE 8
#define STARTING_LIST_SIZE  4
#define LIST_EMPTY_UNDEX    3810

#define MEM_CHECK(x)                                                                        \
do {                                                                                        \
    if (x == NULL)                                                                          \
    {                                                                                       \
        fprintf(stderr, "\nIn function <%s> memory exhausted\n", __PRETTY_FUNCTION__);      \
        exit(-1);                                                                           \
    }                                                                                       \
} while (0)                                                                                 \

// #define DEBUG
#ifdef DEBUG
    #define ANNOUNCE_CALL()                                                                 \
    do {                                                                                    \
        printf("<%s>\n", __PRETTY_FUNCTION__);                                              \
    } while (0)                                                                             \

    #define DEBUG_PRINT_R(x) printf("\x1b[1m\x1b[31m%s\x1B[0m\n", x)
    #define DEBUG_PRINT_G(x) printf("\x1b[1m\x1b[32m%s\x1B[0m\n", x)
    #define DEBUG_PRINT_Y(x) printf("\x1b[1m\x1b[33m%s\x1B[0m\n", x)
    #define DEBUG_PRINT_B(x) printf("\x1b[1m\x1b[34m%s\x1B[0m\n", x)
#else
    #define ANNOUNCE_CALL()  {}
    #define DEBUG_PRINT_R(x) {}
    #define DEBUG_PRINT_G(x) {}
    #define DEBUG_PRINT_Y(x) {}
    #define DEBUG_PRINT_B(x) {}
#endif



typedef int CacheType;
#define CachePrintType "%i"

struct CyclicQueue;

struct HIST;

struct CacheCell;

struct ListNode;

struct List;

struct Cache;


struct Cache* CacheConstruct(size_t cache_size, size_t K);

int CacheDestruct(struct Cache* cache_p);

size_t FindVictim(const struct Cache* cache_p);

int ReplaceVictim(struct Cache* cache_p, size_t victim, int new_page_num, struct CyclicQueue* new_page_hist);

size_t FindPageInCache(const struct Cache* cache_p, int page_num);

int CacheCall(struct Cache* cache_p, int new_page_num, unsigned long current_time);

int PrintCacheData(const struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color);

int StandartCacheTrial();

int FileCacheTrial(const char* file_name);


int ExpandList(struct List* list_p, size_t new_list_capacity);

size_t FindPosition(const struct List* list_p, CacheType page_num);

size_t LoopSearch(const struct List* list_p);

int AddNode(struct List* list_p, struct CacheCell* data_p);

int DeleteNode(struct List* list_p, size_t node_position);

int PrintList(const struct List* list_p);

int PrintTable(const struct Cache* cache_p);

int TEST();


int ExpandHIST(struct HIST* HIST, size_t new_HIST_size);

int UpdatePageInHist(struct CyclicQueue* page, unsigned long current_time);

struct CyclicQueue* FindPageInHIST(const struct HIST* HIST, CacheType page_num);

struct CyclicQueue* AddPageToHIST(struct HIST* HIST, int page_num);

int PrintHIST(const struct HIST* HIST);


size_t FindCyclicPosition(const struct CyclicQueue* page, int position_relative_to_anchor);

#endif
