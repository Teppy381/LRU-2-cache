#include <stdio.h>
#include <stdlib.h>

struct Cache
{
    int  size;
    int  next_empty_slot;
    int* LAST;
    int* data;
};

int IncreaseEverythingButI(int* array, int array_size, int dont_increace_num)
{
    for (int i = 0; i < dont_increace_num; i++)
    {
        array[i] += 1;
    }
    array[dont_increace_num] = 0;
    for (int i = dont_increace_num + 1; i < array_size; i++)
    {
        array[i] += 1;
    }
    return 0;
}


int CacheCreate(struct Cache* cache_p, int cache_size)
{
    cache_p->size = cache_size;
    cache_p->next_empty_slot = 0;

    cache_p->data = (int*) calloc(cache_size, sizeof(int));
    cache_p->LAST = (int*) calloc(cache_size, sizeof(int));

    return 0;
}


int CacheCall(struct Cache* cache_p, int elem)
{
    // 1 - cache hit
    // 0 - cache miss

    for (int i = 0; i < cache_p->size; i++)
    {
        if (cache_p->data[i] == elem)
        {
            IncreaseEverythingButI(cache_p->LAST, cache_p->size, i);
            return 1;
        }
    }


    if (cache_p->next_empty_slot < cache_p->size)
    {
        cache_p->data[cache_p->next_empty_slot] = elem;
        IncreaseEverythingButI(cache_p->LAST, cache_p->size, cache_p->next_empty_slot);
        cache_p->next_empty_slot += 1;
        return 0;
    }
    else
    {
        int last_recent_elem = 0;
        int last_recent_num = 0;
        for (int i = 0; i < cache_p->size; i++)
        {
            if (cache_p->LAST[i] > last_recent_elem)
            {
                last_recent_elem = cache_p->LAST[i];
                last_recent_num = i;
            }
        }

        IncreaseEverythingButI(cache_p->LAST, cache_p->size, last_recent_num);
        cache_p->data[last_recent_num] = elem;
        cache_p->LAST[last_recent_num] = 0;

        return 0;
    }

}


int main()
{
    int cache_size, requests_num, buffer, hits_num = 0;
    struct Cache my_cache = {};

    scanf("%i%i", &cache_size, &requests_num);

    CacheCreate(&my_cache, cache_size);

    for (int i = 0; i < requests_num; i++)
    {
        scanf("%i", &buffer);
        hits_num += CacheCall(&my_cache, buffer);
    }

    // printf("requests_num = %i\n", requests_num);
    // printf("hits_num = %i\n", hits_num);
    printf("%i\n", hits_num);
}
