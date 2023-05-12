#include "functions.h"

int main()
{
    struct Cache Bobs_cache;
    struct Cache* BC = &Bobs_cache;

    CacheConstruct(BC, 5, 2);

    int hit_counter = 0;
    for (int i = 0; i < 100; i++)
    {
        hit_counter += MainAlgorythm(BC, 1, i + 1000);
    }
    // for (int i = 100; i < 200; i++)
    // {
    //     hit_counter += MainAlgorythm(&Bobs_cache, 200 - i, i + 1000);
    // }

    printf("hit_counter = %i\n", hit_counter);
    printf("next_free = %i\n", BC->HIST->next_free);


    CacheDestruct(BC);
    printf("Work in progress...\n");
}
