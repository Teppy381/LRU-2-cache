#include "functions.h"

int main()
{
    struct Cache* Bobs_cache = CacheConstruct(5, 2);

    int hit_counter = 0;
    for (int i = 0; i < 10; i++)
    {
        printf("(%i) ", i);
        hit_counter += MainAlgorythm(Bobs_cache, 10 - i, i + 1000);
        PrintCacheData(Bobs_cache);
    }

    printf("\n");
    printf("hit_counter = %i\n", hit_counter);
    printf("next_free = %i\n", Bobs_cache->HIST->next_free);


    CacheDestruct(Bobs_cache);
    printf("Work in progress...\n");
}
