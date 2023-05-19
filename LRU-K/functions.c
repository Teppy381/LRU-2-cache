#include "functions.h"

struct CyclicQueue
{
    CacheType  page_num;
    size_t  anchor;
    size_t  size;
    unsigned long* data;  // last K times when page_num was called
};

struct HIST
{
    size_t size;
    size_t next_free;     // because of that we know which parts of   struct CyclicQueue* pages   actually store data
    struct CyclicQueue* pages;
};

struct CacheCell
{
    CacheType page_num;
    struct CyclicQueue* page_hist;
};

struct ListNode
{
    size_t next;
    size_t prev;
    CacheType* data_p;
};

struct List
{
    size_t size;
    size_t capacity;
    size_t next_free;
    size_t last_free;
    size_t head;
    size_t tail;
    struct ListNode* data;
};

struct Cache
{
    size_t  K;            // - this is LRU-K cache
    size_t table_size;
    struct List* table;
    size_t  cache_size;
    struct CacheCell* data;
    struct HIST* HIST;
};



// ========================================= Cache ===================================================
struct Cache* CacheConstruct(size_t cache_size, size_t K)
{
    struct Cache* cache_p = (struct Cache*) calloc(1, sizeof(struct Cache));
    MEM_CHECK(cache_p);

    cache_p->K = K;
    cache_p->cache_size = cache_size;
    cache_p->data = (struct CacheCell*) calloc(cache_size, sizeof(struct CacheCell));
    MEM_CHECK(cache_p->data);

    cache_p->table_size = STARTING_TABLE_SIZE;
    cache_p->table = (struct List*) calloc(STARTING_TABLE_SIZE, sizeof(struct List));
    MEM_CHECK(cache_p->table);
    for (size_t i = 0; i < cache_p->table_size; i++)
    {
        ExpandList(&(cache_p->table[i]), STARTING_LIST_SIZE);
    }

    cache_p->HIST = (struct HIST*) calloc(1, sizeof(struct HIST));
    MEM_CHECK(cache_p->HIST);
    cache_p->HIST->size = 0;
    cache_p->HIST->next_free = 1;

    cache_p->HIST->pages = NULL;
    ExpandHIST(cache_p, STARTING_PAGE_AMOUNT);

    return cache_p;
}

int CacheDestruct(struct Cache* cache_p)
{
    assert(cache_p != NULL);

    for (size_t i = 0; i < cache_p->HIST->size; i++)
    {
        free(cache_p->HIST->pages[i].data);
    }

    for (size_t i = 0; i < cache_p->table_size; i++)
    {
        free(cache_p->table[i].data);
    }

    free(cache_p->table);
    free(cache_p->HIST->pages);
    free(cache_p->HIST);
    free(cache_p->data);
    free(cache_p);
    return 0;
}

size_t FindVictim(const struct Cache* cache_p)
{
    assert(cache_p != NULL);

    size_t victim = 0;

    for (size_t i = 0; i < cache_p->cache_size; i++)
    {
        size_t page_position = FindPageInHIST(cache_p->HIST, cache_p->data[i].page_num);
        if (page_position >= cache_p->HIST->next_free)
        {
            // printf("error: no info about page (" CachePrintType ") in HIST\n", cache_p->data[i].page_num);
            return i;
        }

        size_t victim_position = FindPageInHIST(cache_p->HIST, cache_p->data[victim].page_num);
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

int ReplaceVictim(struct Cache* cache_p, size_t victim, CacheType new_page_num)
{
    assert(cache_p != NULL);

    cache_p->data[victim].page_num = new_page_num;
    return 0;
}

size_t FindPageInCache(const struct Cache* cache_p, CacheType page_num)
{
    assert(cache_p != NULL);

    for (size_t i = 0; i < cache_p->cache_size; i++)
    {
        if (cache_p->data[i].page_num == page_num)
        {
            return i;   // cache hit
        }
    }
    return cache_p->cache_size;      // cache miss
}

// 1 = hit   0 = miss
int CacheCall(struct Cache* cache_p, CacheType new_page_num, unsigned long current_time)
{
    assert(cache_p != NULL);

    size_t cache_position = FindPageInCache(cache_p, new_page_num);
    if (cache_position < cache_p->cache_size) // cache hit
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
int PrintCacheData(const struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color)
{
    assert(cache_p != NULL);

    printf("(%lu) ", current_time);
    for (size_t i = 0; i < highlight_element; i++)
    {
        printf(CachePrintType " ", cache_p->data[i].page_num);
    }
    printf("%s" CachePrintType "\x1B[0m ", color, cache_p->data[highlight_element].page_num);
    for (size_t i = highlight_element + 1; i < cache_p->cache_size; i++)
    {
        printf(CachePrintType " ", cache_p->data[i].page_num);
    }

    printf("\n");
    return 0;
}

int StandartCacheTrial()
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
    return 0;
}
// ===================================================================================================



// ========================================== List ===================================================
int ExpandList(struct List* list_p, size_t new_list_capacity)
{
    assert(list_p != NULL);
    assert(new_list_capacity > list_p->capacity);

    if (list_p->data == NULL)
    {
        list_p->data = (struct ListNode*) calloc(new_list_capacity, sizeof(struct ListNode));
    }
    else
    {
        list_p->data = (struct ListNode*) realloc(list_p->data, new_list_capacity * sizeof(struct ListNode));
    }
    MEM_CHECK(list_p->data);

    if (list_p->data[list_p->last_free].data_p == NULL)
    {
        list_p->data[list_p->last_free].next = list_p->capacity;
    }
    for (size_t i = list_p->capacity; i < new_list_capacity - 1; i++)
    {
        list_p->data[i].prev = i - 1;
        list_p->data[i].next = i + 1;
        list_p->data[i].data_p = NULL;
    }
    if (list_p->data[list_p->last_free].data_p == NULL)
    {
        list_p->data[list_p->capacity].prev = list_p->last_free;
    }
    list_p->data[new_list_capacity - 1].prev = new_list_capacity - 2;
    list_p->data[new_list_capacity - 1].next = 0;
    list_p->data[new_list_capacity - 1].data_p = NULL;

    list_p->last_free = new_list_capacity - 1;
    list_p->capacity = new_list_capacity;
    return 0;
}

size_t FindPosition(const struct List* list_p, CacheType* data_p)
{
    assert(list_p != NULL);

    size_t node_adr = list_p->head;
    while(list_p->data[node_adr].data_p != NULL)    // might be loop here
    {
        if (list_p->data[node_adr].data_p == data_p)
        {
            return node_adr;
        }
        node_adr = list_p->data[node_adr].next;
    }
    return list_p->capacity;
}

int AddNode(struct List* list_p, CacheType* data_p)
{
    assert(list_p != NULL);

    // printf("AddNode called with data_p = %p\n", data_p);

    if (list_p->size == list_p->capacity - 1)
    {
        ExpandList(list_p, list_p->capacity * 2);
    }

    size_t free_adress = list_p->next_free;
    size_t next_adress = list_p->data[free_adress].next;
    size_t prev_adress = list_p->tail;

    list_p->data[free_adress].data_p = data_p;
    list_p->data[free_adress].next = 0;

    if (free_adress != prev_adress)
    {
        list_p->data[free_adress].prev = prev_adress;
        list_p->data[prev_adress].next = free_adress;
    }
    list_p->tail = free_adress;
    list_p->next_free = next_adress;
    list_p->size += 1;
    return 0;
}

int DeleteNode(struct List* list_p, size_t node_position)
{
    assert(list_p != NULL);

    size_t next_adress = list_p->data[node_position].next;
    size_t prev_adress = list_p->data[node_position].prev;

    list_p->data[prev_adress].next = next_adress;
    list_p->data[next_adress].prev = prev_adress;

    list_p->data[node_position].data_p = NULL;
    if (node_position == list_p->head) { list_p->head = next_adress; }
    if (node_position == list_p->tail) { list_p->tail = prev_adress; }

    list_p->data[node_position].next = list_p->next_free;
    list_p->data[list_p->next_free].prev = node_position;
    list_p->next_free = node_position;
    list_p->size -= 1;
    return 0;
}

int PrintList(const struct List* list_p)
{
    assert(list_p != NULL);

    printf("Head: %zu  Tail: %zu\nNextFree: %zu  LastFree: %zu\n", list_p->head, list_p->tail, list_p->next_free, list_p->last_free);
    for (size_t i = 0; i < list_p->capacity; i++)
    {
        printf("(%zu) %zu %zu %p\n", i, list_p->data[i].prev, list_p->data[i].next, list_p->data[i].data_p);
    }
    return 0;
}

int PrintTable(const struct Cache* cache_p)
{
    assert(cache_p != NULL);

    for (size_t i = 0; i < cache_p->table_size; i++)
    {
        printf("<<%zu>>\n", i);
        PrintList(&(cache_p->table[i]));
        printf("\n");
    }

    return 0;
}

int TEST(struct Cache* cache_p)
{
    AddNode(&(cache_p->table[7]), (CacheType*) 3);
    AddNode(&(cache_p->table[7]), (CacheType*) 235);
    AddNode(&(cache_p->table[7]), (CacheType*) 34523);
    AddNode(&(cache_p->table[6]), (CacheType*) 134523);
    AddNode(&(cache_p->table[7]), (CacheType*) 134);
    DeleteNode(&(cache_p->table[7]), 2);
    return 0;
}
// ===================================================================================================



// ========================================== HIST ===================================================
int ExpandHIST(struct Cache* cache_p, size_t new_HIST_size)
{
    assert(cache_p != NULL);
    assert(new_HIST_size > cache_p->HIST->size);

    if (cache_p->HIST->pages == NULL)
    {
        cache_p->HIST->pages = (struct CyclicQueue*) calloc(new_HIST_size, sizeof(struct CyclicQueue));
    }
    else
    {
        cache_p->HIST->pages = (struct CyclicQueue*) realloc(cache_p->HIST->pages, new_HIST_size * sizeof(struct CyclicQueue));
    }
    MEM_CHECK(cache_p->HIST->pages);

    for (size_t i = cache_p->HIST->size; i < new_HIST_size; i++)
    {
        cache_p->HIST->pages[i].data = (unsigned long*) calloc(cache_p->K, sizeof(unsigned long));
        MEM_CHECK(cache_p->HIST->pages[i].data);
        cache_p->HIST->pages[i].size = cache_p->K;
        cache_p->HIST->pages[i].anchor = 0;
        cache_p->HIST->pages[i].page_num = 0;
    }

    cache_p->HIST->size = new_HIST_size;
    return 0;
}

// page->data[page->anchor - 1] is the least recent time this page was called
//   page->data[page->anchor]    is the most recent time this page was called
int UpdatePageHist(struct CyclicQueue* page, unsigned long current_time)
{
    assert(page != NULL);

    page->anchor = FindCyclicPosition(page, -1);
    page->data[page->anchor] = current_time;

    return 0;
}

size_t FindPageInHIST(const struct HIST* HIST, CacheType page_num)
{
    assert(HIST != NULL);

    for (size_t i = 0; i < HIST->next_free; i++)
    {
        if (HIST->pages[i].page_num == page_num)
        {
            return i;
        }
    }
    return HIST->next_free;
}

int AddPageToHIST(struct Cache* cache_p, CacheType page_num, unsigned long current_time)
{
    assert(cache_p != NULL);

    if (cache_p->HIST->next_free >= cache_p->HIST->size)
    {
        ExpandHIST(cache_p, 2 * cache_p->HIST->size);
    }

    cache_p->HIST->pages[cache_p->HIST->next_free].page_num = page_num;

    UpdatePageHist(&(cache_p->HIST->pages[cache_p->HIST->next_free]), current_time);
    cache_p->HIST->next_free += 1;

    // printf("Page " CachePrintType " added to HIST\n", page_num);
    return 0;
}
// ===================================================================================================



// ===================================== CyclicQueue =================================================
size_t FindCyclicPosition(const struct CyclicQueue* page, int position_relative_to_anchor)
{
    assert(page != NULL);
    assert(page->size != 0);

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
// ===================================================================================================



