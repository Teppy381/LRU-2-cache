#include "functions.h"


int CacheConstruct(struct Cache* cache_p, int cache_size, int K)
{
    cache_p->K = K;
    cache_p->size = cache_size;
    cache_p->data = (int*) calloc(cache_size, sizeof(int));
    cache_p->HIST = (struct HIST*) calloc(1, sizeof(struct HIST));

    cache_p->HIST->size = 0;
    cache_p->HIST->next_free = 0;

    cache_p->HIST->pages = NULL;
    CacheExpandHIST(cache_p, STARTING_PAGE_AMOUNT);

    return 0;
}

int CacheExpandHIST(struct Cache* cache_p, int new_HIST_size)
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

    for (int i = cache_p->HIST->size; i < new_HIST_size; i++)
    {
        cache_p->HIST->pages[i].data = (int*) calloc(cache_p->K, sizeof(int));
        cache_p->HIST->pages[i].size = cache_p->K;
        cache_p->HIST->pages[i].anchor = 0;
        cache_p->HIST->pages[i].page_num = 0;

        for (int j = 0; j < cache_p->K; j++)
        {
            cache_p->HIST->pages[i].data[j] = INT_MIN;
        }
    }

    cache_p->HIST->size = new_HIST_size;

    return 0;
}

int CacheDestruct(struct Cache* cache_p)
{
    for (int i = 0; i < cache_p->HIST->size; i++)
    {
        free(cache_p->HIST->pages[i].data);
    }

    free(cache_p->HIST->pages);
    cache_p->HIST->size = 0;

    free(cache_p->HIST);
    free(cache_p->data);
    cache_p->size = 0;

    return 0;
}

// page->data[page->anchor - 1] is the least recent time this page was called
//   page->data[page->anchor]    is the most recent time this page was called
int UpdatePageHist(struct CyclicQueue* page, int current_time)
{
    if (page->anchor > 0)
    {
        page->anchor -= 1;
    }
    else if (page->anchor == 0)
    {
        page->anchor = page->size - 1;
    }
    else
    {
        return -1; // error
    }
    page->data[page->anchor] = current_time;

    return 0;
}

int FindPageInHIST(struct HIST* HIST, int page_num)
{
    for (int i = 0; i < HIST->next_free; i++)
    {
        if (HIST->pages[i].page_num == page_num)
        {
            return i;
        }
    }
    return -1;
}

int AddPageToHIST(struct Cache* cache_p, int page_num, int current_time)
{
    if (cache_p->HIST->next_free >= cache_p->HIST->size)
    {
        CacheExpandHIST(cache_p, 2 * cache_p->HIST->size);
    }

    cache_p->HIST->pages[cache_p->HIST->next_free].page_num = page_num;
    cache_p->HIST->next_free += 1;

    UpdatePageHist(&(cache_p->HIST->pages[cache_p->HIST->next_free]), current_time);

    printf("Page %i added to HIST\n", page_num);

    return 0;
}


int FindVictim(struct Cache* cache_p)
{
    int min_time = 0;
    int victim = 0;

    for (int i = 0; i < cache_p->size; i++)
    {
        int page_position = FindPageInHIST(cache_p->HIST, cache_p->data[i]);

        if (page_position < 0 || page_position >= cache_p->HIST->size)
        {
            return -1;      // error: no info about this page in HIST
        }

        struct CyclicQueue page = cache_p->HIST->pages[page_position];
        int LRU_time = 0;

        if (page.anchor > 0)
        {
            LRU_time = page.data[page.anchor - 1];
        }
        else if (page.anchor == 0)
        {
            LRU_time = page.data[page.size - 1];
        }
        else return -1;                 // error: anchor < 0

        if (LRU_time < min_time)      // because global time only increases
        {
            min_time = LRU_time;
            victim = i;
        }
    }

    return victim;
}

int ReplaceVictim(struct Cache* cache_p, int victim, int new_page_num)
{
    if (victim < 0 || victim >= cache_p->size)
    {
        return -1;  // error
    }
    cache_p->data[victim] = new_page_num;

    return 0;
}

int MainAlgorythm(struct Cache* cache_p, int new_page_num, int current_time)
{
    int page_position = FindPageInHIST(cache_p->HIST, new_page_num);
    if (page_position >= 0)
    {
        UpdatePageHist(&(cache_p->HIST->pages[page_position]), current_time);
        return 1;   // cache_hit
    }

    int victim = FindVictim(cache_p);
    ReplaceVictim(cache_p, victim, new_page_num);
    AddPageToHIST(cache_p, new_page_num, current_time);

    return 0;   // cache_miss
}

//add cache_hit/cache_miss
//add CreatePageHist? - done (AddPageToHIST)




