#include "functions.h"

int main()
{
    struct Cache Bobs_cache;

    printf("%i\n", CacheConstruct(&Bobs_cache, 5, 2));

    printf("%i\n", CacheExpandHIST(&Bobs_cache, 20));

    printf("%i\n", FindVictim(&Bobs_cache));

    printf("%i\n", CacheDestruct(&Bobs_cache));

    printf("Work in progress...\n");
}
