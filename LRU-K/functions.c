#include "functions.h"


struct Cache* CacheConstruct(size_t cache_size, size_t K)
{
    struct Cache* cache_p = (struct Cache*) calloc(1, sizeof(struct Cache));

    cache_p->K = K;
    cache_p->size = cache_size;
    cache_p->data = (int*) calloc(cache_size, sizeof(int));
    cache_p->HIST = (struct HIST*) calloc(1, sizeof(struct HIST));

    cache_p->HIST->size = 0;
    cache_p->HIST->next_free = 1;

    cache_p->HIST->pages = NULL;
    CacheExpandHIST(cache_p, STARTING_PAGE_AMOUNT);

    return cache_p;
}

int CacheExpandHIST(struct Cache* cache_p, size_t new_HIST_size)
{
    if (new_HIST_size < cache_p->HIST->size)
    {
        return -1;
    }
    else if (new_HIST_size == cache_p->HIST->size)
    {
        return 0;
    }

    if (cache_p->HIST->pages == NULL)
    {
        cache_p->HIST->pages = (struct CyclicQueue*) calloc(new_HIST_size, sizeof(struct CyclicQueue));
    }
    else
    {
        cache_p->HIST->pages = (struct CyclicQueue*) realloc(cache_p->HIST->pages, new_HIST_size * sizeof(struct CyclicQueue));
    }

    for (size_t i = cache_p->HIST->size; i < new_HIST_size; i++)
    {
        cache_p->HIST->pages[i].data = (unsigned long*) calloc(cache_p->K, sizeof(unsigned long));
        cache_p->HIST->pages[i].size = cache_p->K;
        cache_p->HIST->pages[i].anchor = 0;
        cache_p->HIST->pages[i].page_num = 0;
    }

    cache_p->HIST->size = new_HIST_size;
    return 0;
}

int CacheDestruct(struct Cache* cache_p)
{
    for (size_t i = 0; i < cache_p->HIST->size; i++)
    {
        free(cache_p->HIST->pages[i].data);
    }

    free(cache_p->HIST->pages);
    free(cache_p->HIST);
    free(cache_p->data);
    free(cache_p);
    return 0;
}

size_t FindCyclicPosition(struct CyclicQueue* page, int position_relative_to_anchor)
{
    int pos = position_relative_to_anchor % page->size;

    if (page->anchor + pos >= page->size)
    {
        return page->anchor + pos - page->size;
    }
    else if (((int) page->anchor) + pos < 0)
    {
        return page->anchor + pos + page->size;
    }
    else
    {
        return page->anchor + pos;
    }
}

// page->data[page->anchor - 1] is the least recent time this page was called
//   page->data[page->anchor]    is the most recent time this page was called
int UpdatePageHist(struct CyclicQueue* page, unsigned long current_time)
{
    page->anchor = FindCyclicPosition(page, -1);
    page->data[page->anchor] = current_time;

    return 0;
}

size_t FindPageInHIST(struct HIST* HIST, int page_num)
{
    for (size_t i = 0; i < HIST->next_free; i++)
    {
        if (HIST->pages[i].page_num == page_num)
        {
            return i;
        }
    }
    return HIST->next_free;
}

int AddPageToHIST(struct Cache* cache_p, int page_num, unsigned long current_time)
{
    if (cache_p->HIST->next_free >= cache_p->HIST->size)
    {
        CacheExpandHIST(cache_p, 2 * cache_p->HIST->size);
    }

    cache_p->HIST->pages[cache_p->HIST->next_free].page_num = page_num;

    UpdatePageHist(&(cache_p->HIST->pages[cache_p->HIST->next_free]), current_time);
    cache_p->HIST->next_free += 1;

    // printf("Page %i added to HIST\n", page_num);
    return 0;
}


size_t FindVictim(struct Cache* cache_p)
{
    size_t victim = 0;

    for (size_t i = 0; i < cache_p->size; i++)
    {
        size_t page_position = FindPageInHIST(cache_p->HIST, cache_p->data[i]);
        if (page_position >= cache_p->HIST->next_free)
        {
            // printf("error: no info about page (%i) in HIST\n", cache_p->data[i]);
            return i;
        }

        size_t victim_position = FindPageInHIST(cache_p->HIST, cache_p->data[victim]);
        if (victim_position >= cache_p->HIST->next_free)
        {
            // this means that we found some garbage in cache ==> we should replace it
            return victim;
        }

        struct CyclicQueue page = cache_p->HIST->pages[page_position];
        struct CyclicQueue victim_page = cache_p->HIST->pages[victim_position];

        for (size_t k = 1; k <= page.size && k <= victim_page.size; k++)
        {
            unsigned long LRU_time = page.data[FindCyclicPosition(&page, -k)];
            unsigned long min_time = victim_page.data[FindCyclicPosition(&victim_page, -k)];

            // printf("#%zu# (%lu) (%lu) - %zu\n", k, LRU_time, min_time, victim);

            if (LRU_time == min_time)
            {
                // printf("continue\n");
                continue;
            }
            else if (LRU_time < min_time)   // because current time only increases
            {
                victim = i;
                break;
            }
            else // (LRU_time > min_time)
            {
                break;
            }
        }
    }

    return victim;
}

int ReplaceVictim(struct Cache* cache_p, size_t victim, int new_page_num)
{
    cache_p->data[victim] = new_page_num;
    return 0;
}

size_t FindPageInCache(struct Cache* cache_p, int page_num)
{
    for (size_t i = 0; i < cache_p->size; i++)
    {
        if (cache_p->data[i] == page_num)
        {
            return i;   // cache hit
        }
    }
    return cache_p->size;      // cache miss
}

// 1 = hit   0 = miss
int CacheCall(struct Cache* cache_p, int new_page_num, unsigned long current_time)
{
    size_t cache_position = FindPageInCache(cache_p, new_page_num);
    if (cache_position < cache_p->size) // cache hit
    {
        size_t page_position = FindPageInHIST(cache_p->HIST, new_page_num);
        if (page_position >= cache_p->HIST->next_free)
        {
            return 1;
        }
        UpdatePageHist(&(cache_p->HIST->pages[page_position]), current_time);

        #ifdef DEMO_MODE
        PrintCacheData(cache_p, current_time, cache_position, "\x1b[1m\x1B[32m");
        #endif

        return 1;   // cache_hit
    }

    size_t victim = FindVictim(cache_p);
    // printf("victim = %zu\n", victim);
    ReplaceVictim(cache_p, victim, new_page_num);
    AddPageToHIST(cache_p, new_page_num, current_time);

    #ifdef DEMO_MODE
    PrintCacheData(cache_p, current_time, victim, "\x1b[1m\x1B[35m");
    #endif

    return 0;   // cache_miss
}

// print all data which is currently stored in cache
int PrintCacheData(struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color)
{
    printf("(%lu) ", current_time);
    for (size_t i = 0; i < highlight_element; i++)
    {
        printf("%i ", cache_p->data[i]);
    }
    printf("%s%i\x1B[0m ", color, cache_p->data[highlight_element]);
    for (size_t i = highlight_element + 1; i < cache_p->size; i++)
    {
        printf("%i ", cache_p->data[i]);
    }

    printf("\n");
    return 0;
}
