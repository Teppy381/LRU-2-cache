#include "functions.h"


// for demonstration mode use   make LRU-K-DEMO

int main()
{
    size_t cache_size, requests_num, hit_counter = 0;
    int buffer;
    scanf("%li%li", &cache_size, &requests_num);

    struct Cache* Bobs_cache = CacheConstruct(cache_size, 2);

    for (unsigned long time = 1; time < requests_num + 1; time++)
    {
        scanf("%i", &buffer);
        hit_counter += CacheCall(Bobs_cache, buffer, time);
    }
    CacheDestruct(Bobs_cache);
    printf("%li\n", hit_counter);
}
