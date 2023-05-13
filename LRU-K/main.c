#include "functions.h"

int main()
{
    size_t cache_size, requests_num, hit_counter = 0;
    int buffer;
    scanf("%li%li", &cache_size, &requests_num);

    struct Cache* Bobs_cache = CacheConstruct(cache_size, 2);

    for (size_t time = 0; time < requests_num; time++)
    {
        scanf("%i", &buffer);
        hit_counter += MainAlgorythm(Bobs_cache, buffer, time);
    }

    CacheDestruct(Bobs_cache);

    printf("%li\n", hit_counter);
}
