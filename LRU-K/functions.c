#include "functions.h"

struct CyclicQueue
{
    CacheType page_num;
    size_t  anchor;
    size_t  size;
    unsigned long* data;  // last K times when page_num was called
};

struct HIST
{
    size_t K;             // - this is LRU-K cache, so we store last K times when each page was called
    size_t size;
    size_t next_free;     // because of that we know which parts of   struct CyclicQueue* pages   actually store data
    struct CyclicQueue* pages;
};

// struct HIST
// {
//     struct TreeNode* first_node;
//     // что то ещё тут
// };
//
// struct TreeNode
// {
//     struct TreeNode* parent;
//     struct TreeNode* left;
//     struct TreeNode* right;
//     int color;
//     struct CyclicQueue page_hist;
// };

struct CacheCell
{
    CacheType page_num;
    struct CyclicQueue* page_hist;
};

struct ListNode
{
    size_t next;
    size_t prev;
    struct CacheCell* data_p;
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
    size_t table_size;
    struct List* table;
    size_t  cache_size;
    struct CacheCell* data;
    struct HIST* HIST;
};



// ========================================= Cache ===================================================
struct Cache* CacheConstruct(size_t cache_size, size_t K)
{
    ANNOUNCE_CALL();

    struct Cache* cache_p = (struct Cache*) calloc(1, sizeof(struct Cache));
    MEM_CHECK(cache_p);

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
    cache_p->HIST->K = K;
    cache_p->HIST->size = 0;
    cache_p->HIST->next_free = 0;

    cache_p->HIST->pages = NULL;
    ExpandHIST(cache_p->HIST, STARTING_HIST_SIZE);

    return cache_p;
}

int CacheDestruct(struct Cache* cache_p)
{
    ANNOUNCE_CALL();

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
    ANNOUNCE_CALL();

    assert(cache_p != NULL);

    size_t victim = 0;

    for (size_t i = 1; i < cache_p->cache_size; i++)
    {
        struct CyclicQueue* page_hist;
        struct CyclicQueue* victim_hist;

        if (cache_p->data[i].page_hist != NULL)
        {
            page_hist = cache_p->data[i].page_hist;
            DEBUG_PRINT_G("page: found hist in cache");
        }
        else
        {
            page_hist = FindPageInHIST(cache_p->HIST, cache_p->data[i].page_num);
            if (page_hist == NULL)
            {
                DEBUG_PRINT_B("page: cannot find hist");
                return i;   // cannot find any info about this page ==> we should replace it
            }
            DEBUG_PRINT_Y("page: found hist");
        }

        if (cache_p->data[victim].page_hist != NULL)
        {
            victim_hist = cache_p->data[victim].page_hist;
            DEBUG_PRINT_G("victim: found hist in cache");
        }
        else
        {
            victim_hist = FindPageInHIST(cache_p->HIST, cache_p->data[victim].page_num);
            if (victim_hist == NULL)
            {
                DEBUG_PRINT_B("victim: cannot find hist");
                return victim;   // cannot find any info about this page ==> we should replace it
            }
            DEBUG_PRINT_Y("victim: found hist");
        }

        if (page_hist->size == victim_hist->size && page_hist->anchor == victim_hist->anchor &&
                memcmp(page_hist->data, victim_hist->data, page_hist->size * sizeof(*page_hist->data)) == 0)
        {
            DEBUG_PRINT_B("memcmp == 0");
            continue;
        }

        for (size_t k = 1; k <= page_hist->size && k <= victim_hist->size; k++)
        {
            unsigned long LRU_time = page_hist->data[FindCyclicPosition(page_hist, -k)];
            unsigned long min_time = victim_hist->data[FindCyclicPosition(victim_hist, -k)];

            // printf("#%zu# (%lu) (%lu) - %zu\n", k, LRU_time, min_time, victim);
            // printf("pos = %zu\n", FindCyclicPosition(victim_hist, -k));
            if (LRU_time == min_time)
            {
                DEBUG_PRINT_R("continue (LRU_time == min_time)");
                continue;
            }
            else if (LRU_time < min_time)   // because current time only increases
            {
                DEBUG_PRINT_Y("break (LRU_time < min_time");
                victim = i;
                break;
            }
            else // (LRU_time > min_time)
            {
                DEBUG_PRINT_Y("break (LRU_time > min_time)");
                break;
            }
        }
    }

    return victim;
}

size_t Hash(CacheType page_num)
{
    ANNOUNCE_CALL();

    return (unsigned) page_num;
}


int ReplaceVictim(struct Cache* cache_p, size_t victim, CacheType new_page_num, struct CyclicQueue* new_page_hist)
{
    ANNOUNCE_CALL();

    assert(cache_p != NULL);

    size_t victim_hash = Hash(cache_p->data[victim].page_num) % cache_p->table_size;
    size_t pos = FindPosition(&(cache_p->table[victim_hash]), cache_p->data[victim].page_num);
    if (pos < cache_p->table[victim_hash].capacity)
    {
        DeleteNode(&(cache_p->table[victim_hash]), pos);
    }
    size_t new_page_hash = Hash(new_page_num) % cache_p->table_size;
    AddNode(&(cache_p->table[new_page_hash]), &(cache_p->data[victim]));

    cache_p->data[victim].page_num = new_page_num;
    cache_p->data[victim].page_hist = new_page_hist;
    return 0;
}

size_t FindPageInCache(const struct Cache* cache_p, CacheType page_num)
{
    ANNOUNCE_CALL();

    assert(cache_p != NULL);

    size_t page_hash = Hash(page_num) % cache_p->table_size;
    size_t pos = FindPosition(&(cache_p->table[page_hash]), page_num);

    if (pos < cache_p->table[page_hash].capacity)
    {
        return (cache_p->table[page_hash].data[pos].data_p - cache_p->data);
    }
    return cache_p->cache_size;      // cache miss
}

// 1 = hit   0 = miss
int CacheCall(struct Cache* cache_p, CacheType new_page_num, unsigned long current_time)
{
    ANNOUNCE_CALL();

    assert(cache_p != NULL);


    // if (current_time >= 138)
    // {
    //     PrintList(&(cache_p->table[2]));
    // }

    struct CyclicQueue* page_hist;

    size_t cache_position = FindPageInCache(cache_p, new_page_num);
    if (cache_position < cache_p->cache_size) // cache hit
    {
        page_hist = cache_p->data[cache_position].page_hist;
        if (page_hist == NULL)
        {
            fprintf(stderr, "Null pointer to HIST of page " CachePrintType "\n", cache_p->data[cache_position].page_num);
            return 1;
        }
        UpdatePageInHist(page_hist, current_time);

        #ifdef DEMO_MODE
        PrintCacheData(cache_p, current_time, cache_position, "\x1b[1m\x1B[32m");
        #endif

        return 1;   // cache_hit
    }

    page_hist = FindPageInHIST(cache_p->HIST, new_page_num);
    if (page_hist == NULL)
    {
        page_hist = AddPageToHIST(cache_p->HIST, new_page_num);
    }
    UpdatePageInHist(page_hist, current_time);

    size_t victim = FindVictim(cache_p);
    ReplaceVictim(cache_p, victim, new_page_num, page_hist);

    #ifdef DEMO_MODE
    PrintCacheData(cache_p, current_time, victim, "\x1b[1m\x1B[35m");
    #endif

    return 0;   // cache_miss
}

// print all data which is currently stored in cache
int PrintCacheData(const struct Cache* cache_p, unsigned long current_time, size_t highlight_element, const char* color)
{
    ANNOUNCE_CALL();

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
    ANNOUNCE_CALL();

    size_t cache_size, requests_num, hit_counter = 0;
    int buffer, scan;

    scan = scanf("%li%li", &cache_size, &requests_num);
    assert(scan == 2);

    struct Cache* Bobs_cache = CacheConstruct(cache_size, 2);

    for (unsigned long time = 1; time < requests_num + 1; time++)
    {
        scan = scanf("%i", &buffer);
        if (scan != 1)
        {
            break;
        }
        hit_counter += CacheCall(Bobs_cache, buffer, time);
    }

    // PrintHIST(Bobs_cache->HIST);

    CacheDestruct(Bobs_cache);
    printf("%li\n", hit_counter);
    return 0;
}

int FileCacheTrial(const char* file_name)
{
    ANNOUNCE_CALL();

    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Cannot find file <%s>\n", file_name);
        return 0;
    }
    else
    {
        printf("File <%s> opened succsessfully\n", file_name);
    }

    size_t cache_size, requests_num, hit_counter = 0;
    int buffer, scan;
    scan = fscanf(file, "%li%li", &cache_size, &requests_num);
    assert(scan == 2);

    struct Cache* Bobs_cache = CacheConstruct(cache_size, 2);

    // printf("%zu %zu\n", cache_size, requests_num);
    unsigned long time = 1;
    for (; time < requests_num + 1; time++)
    {
        // printf("time: %lu\n", time);
        scan = fscanf(file, "%i", &buffer);
        if (scan != 1)
        {
            break;
        }
        hit_counter += CacheCall(Bobs_cache, buffer, time);
    }

    CacheDestruct(Bobs_cache);
    printf("%li hits out of %lu (%.1f%%)\n", hit_counter, time - 1, 100 * (double) hit_counter/ (double) ( time - 1));
    return 0;
}
// ===================================================================================================



// ========================================== List ===================================================
int ExpandList(struct List* list_p, size_t new_list_capacity)
{
    ANNOUNCE_CALL();

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
    for (size_t i = list_p->capacity; i < new_list_capacity; i++)
    {
        list_p->data[i].prev = LIST_EMPTY_UNDEX;
        list_p->data[i].next = i + 1;
        list_p->data[i].data_p = NULL;
    }
    // if (list_p->data[list_p->last_free].data_p == NULL)
    // {
    //     list_p->data[list_p->capacity].prev = list_p->last_free;
    // }

    list_p->last_free = new_list_capacity - 1;
    list_p->capacity = new_list_capacity;
    return 0;
}

size_t FindPosition(const struct List* list_p, CacheType page_num)
{
    ANNOUNCE_CALL();

    assert(list_p != NULL);

    // size_t loop_node = LoopSearch(list_p);
    // if (loop_node < list_p->capacity)
    // {
    //     printf("\x1b[1m\x1b[31mLoop found in list! Begins on node (%zu)\x1B[0m\n", loop_node);
    //     PrintList(list_p);
    //     exit(-1);
    // }

    size_t node_adr = list_p->head;
    while(node_adr < list_p->capacity && list_p->data[node_adr].data_p != NULL && node_adr != list_p->tail)    // might be loop here
    {
        if (list_p->data[node_adr].data_p->page_num == page_num)
        {
            return node_adr;
        }
        node_adr = list_p->data[node_adr].next;
    }
    return list_p->capacity;
}

size_t LoopSearch(const struct List* list_p)
{
    ANNOUNCE_CALL();

    assert(list_p != NULL);

    size_t slow = list_p->head;
    size_t fast = list_p->head;

    while (slow < list_p->capacity && fast < list_p->capacity && list_p->data[fast].next < list_p->capacity &&
        list_p->data[fast].data_p != NULL && list_p->data[list_p->data[fast].next].data_p != NULL)
    {
        slow = list_p->data[slow].next;
        fast = list_p->data[fast].next;
        fast = list_p->data[fast].next;
        if (slow == fast)
        {
            slow = list_p->head;
            while (slow != fast)
            {
                slow = list_p->data[slow].next;
                fast = list_p->data[fast].next;
            }
            return fast;
        }
    }
    return list_p->capacity;
}

int AddNode(struct List* list_p, struct CacheCell* data_p)
{
    ANNOUNCE_CALL();

    assert(list_p != NULL);

    if (list_p->size == list_p->capacity - 1)
    {
        ExpandList(list_p, list_p->capacity * 2);
    }

    size_t free_adress = list_p->next_free;
    size_t next_adress = list_p->data[free_adress].next;
    size_t prev_adress = list_p->tail;

    list_p->data[free_adress].data_p = data_p;

    if (free_adress != prev_adress)
    {
        list_p->data[free_adress].prev = prev_adress;
        list_p->data[free_adress].next = LIST_EMPTY_UNDEX;
        list_p->data[prev_adress].next = free_adress;
    }
    list_p->tail = free_adress;
    list_p->next_free = next_adress;
    list_p->size += 1;
    return 0;
}

int DeleteNode(struct List* list_p, size_t node_position)
{
    ANNOUNCE_CALL();

    assert(list_p != NULL);

    size_t next_adress = list_p->data[node_position].next;
    size_t prev_adress = list_p->data[node_position].prev;

    if (prev_adress < list_p->capacity)
    {
        list_p->data[prev_adress].next = next_adress;
    }
    if (next_adress < list_p->capacity)
    {
        list_p->data[next_adress].prev = prev_adress;
    }
    list_p->data[node_position].data_p = NULL;
    if (node_position == list_p->head) { list_p->head = next_adress; }
    if (node_position == list_p->tail) { list_p->tail = prev_adress; }

    list_p->data[node_position].next = list_p->next_free;
    list_p->data[node_position].prev = LIST_EMPTY_UNDEX;
    // list_p->data[list_p->next_free].prev = node_position;
    list_p->next_free = node_position;
    list_p->size -= 1;
    return 0;
}

int PrintList(const struct List* list_p)
{
    ANNOUNCE_CALL();

    assert(list_p != NULL);

    printf("Head: %zu  Tail: %zu\nNextFree: %zu  LastFree: %zu\n", list_p->head, list_p->tail, list_p->next_free, list_p->last_free);
    for (size_t i = 0; i < list_p->capacity; i++)
    {
        if (list_p->data[i].data_p == NULL)
        {
            printf("\x1b[30m(%zu) %zu %zu %p\x1B[0m\n", i, list_p->data[i].prev, list_p->data[i].next, list_p->data[i].data_p);
        }
        else
        {
            printf("(%zu) %zu %zu %p\n", i, list_p->data[i].prev, list_p->data[i].next, list_p->data[i].data_p);
        }
    }
    return 0;
}

int PrintTable(const struct Cache* cache_p)
{
    ANNOUNCE_CALL();

    assert(cache_p != NULL);

    for (size_t i = 0; i < cache_p->table_size; i++)
    {
        printf("<<%zu>>\n", i);
        PrintList(&(cache_p->table[i]));
        printf("\n");
    }

    return 0;
}

int TEST()
{
    ANNOUNCE_CALL();

    struct List* bebr = (struct List*) calloc(1, sizeof(struct List));

    ExpandList(bebr, 10);

    AddNode(bebr, (struct CacheCell*) 1001010);
    AddNode(bebr, (struct CacheCell*) 1011010);
    AddNode(bebr, (struct CacheCell*) 1011011);
    PrintList(bebr);

    DeleteNode(bebr, 1);
    PrintList(bebr);

    AddNode(bebr, (struct CacheCell*) 1011011);
    PrintList(bebr);

    free(bebr->data);
    free(bebr);
    return 0;
}
// ===================================================================================================



// ========================================== HIST ===================================================
int ExpandHIST(struct HIST* HIST, size_t new_HIST_size)
{
    ANNOUNCE_CALL();

    assert(HIST != NULL);
    assert(new_HIST_size > HIST->size);

    if (HIST->pages == NULL)
    {
        HIST->pages = (struct CyclicQueue*) calloc(new_HIST_size, sizeof(struct CyclicQueue));
    }
    else
    {
        HIST->pages = (struct CyclicQueue*) realloc(HIST->pages, new_HIST_size * sizeof(struct CyclicQueue));
    }
    MEM_CHECK(HIST->pages);

    for (size_t i = HIST->size; i < new_HIST_size; i++)
    {
        HIST->pages[i].data = (unsigned long*) calloc(HIST->K, sizeof(unsigned long));
        MEM_CHECK(HIST->pages[i].data);
        HIST->pages[i].size = HIST->K;
        HIST->pages[i].anchor = 0;
        HIST->pages[i].page_num = 0;
    }

    HIST->size = new_HIST_size;
    return 0;
}

// page->data[page->anchor - 1] is the least recent time this page was called
//   page->data[page->anchor]    is the most recent time this page was called
int UpdatePageInHist(struct CyclicQueue* page, unsigned long current_time)
{
    ANNOUNCE_CALL();

    assert(page != NULL);

    page->anchor = FindCyclicPosition(page, -1);
    page->data[page->anchor] = current_time;

    // printf("anchor = %zu\n", page->anchor);
    // printf("data = %lu\n", page->data[page->anchor]);
    return 0;
}

struct CyclicQueue* FindPageInHIST(const struct HIST* HIST, CacheType page_num)
{
    ANNOUNCE_CALL();

    assert(HIST != NULL);

    // printf("FindPageInHIST\n");
    for (size_t i = 0; i < HIST->next_free; i++)
    {
        if (HIST->pages[i].page_num == page_num)
        {
            return &(HIST->pages[i]);
        }
    }
    return NULL;
}

struct CyclicQueue* AddPageToHIST(struct HIST* HIST, CacheType page_num)
{
    ANNOUNCE_CALL();

    assert(HIST != NULL);

    if (HIST->next_free >= HIST->size)
    {
        ExpandHIST(HIST, 2 * HIST->size);
    }

    HIST->pages[HIST->next_free].page_num = page_num;
    HIST->next_free += 1;

    return &(HIST->pages[HIST->next_free - 1]);
}

int PrintHIST(const struct HIST* HIST)
{
    ANNOUNCE_CALL();

    for (size_t i = 0; i < HIST->next_free; i++)
    {
        printf("\n(" CachePrintType ")\n", HIST->pages[i].page_num);
        for (size_t j = 0; j < HIST->pages[i].size; j++)
        {
            printf("%lu ", HIST->pages[i].data[j]);
        }
        printf("\n");
    }
    return 0;
}
// ===================================================================================================



// ===================================== CyclicQueue =================================================
size_t FindCyclicPosition(const struct CyclicQueue* page, int position_relative_to_anchor)
{
    ANNOUNCE_CALL();

    assert(page != NULL);
    assert(page->size != 0);

    int pos = position_relative_to_anchor + (int) page->anchor;
    while (pos < 0)
    {
        pos += page->size;
    }
    pos = pos % page->size;

    return pos;
}
// ===================================================================================================



