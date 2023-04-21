#include "functions.h"


int CacheConstruct(struct Cache* cache_p, int cache_size)
{
    cache_p->size = cache_size;
    cache_p->data = (int*) calloc(cache_size, sizeof(int));
    cache_p->HIST = (struct HIST*) calloc(1, sizeof(struct HIST));

    cache_p->HIST->size = STARTING_PAGE_AMOUNT;
    cache_p->HIST->data = (int**) calloc(STARTING_PAGE_AMOUNT, sizeof(int*));

    for (int i = 0; i < STARTING_PAGE_AMOUNT; i++)
    {
        cache_p->HIST->data[i] = (int*) calloc(cache_p->K, sizeof(int));
    }

    return 0;
}

int CacheExpandHIST(struct Cache* cache_p, int new_HIST_size)
{
    cache_p->HIST->data = (int**) realloc(cache_p->HIST, new_HIST_size*sizeof(int*));

    for (int i = cache_p->HIST->size; i < new_HIST_size; i++)
    {
        cache_p->HIST->data[i] = (int*) calloc(cache_p->K, sizeof(int));
    }

    cache_p->HIST->size = new_HIST_size;

    return 0;
}

int CacheDestruct(struct Cache* cache_p)
{
    for (int i = 0; i < cache_p->HIST->size; i++)
    {
        free(cache_p->HIST->data[i]);
    }
    free(cache_p->HIST->data);
    cache_p->HIST->size = 0;

    free(cache_p->HIST);
    free(cache_p->data);
    cache_p->size = 0;

    return 0;
}





