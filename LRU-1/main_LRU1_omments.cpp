#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
//Узел нашего двусвязного списка
struct node {

    struct node *next;
    struct node *prev;
    unsigned long long val;

};
//Объединение хэша и листа, size это размер кэша, а inserts это количество внесений в кэш страниц
struct Hashtable {

    struct node **lists_arr;
    struct node *list_head;
    struct node *list_tail;

    unsigned long long size;
    unsigned long long inserts;

};

//Инициализация хэша и листа
 struct Hashtable *HashtableInit(unsigned long long size) {

    struct Hashtable *HashT = (struct Hashtable *)calloc(1, sizeof(struct Hashtable));
    HashT->lists_arr = (struct node **)calloc(size, sizeof(struct node *));

    assert(HashT != NULL);
    assert(HashT->lists_arr != NULL);

    HashT->size = size;
    HashT->inserts = 0;

    return HashT;

}
//Функция, которая на k-ом шаге вносит в наш лист страницу
int HashtableInsert(struct Hashtable *HashT, int m, unsigned long long val) {

    //hits - попадания за 1 шаг
    int hits = 0;
    unsigned long long hash1 = val;
    int capacity = m;

    //Если страницы нет в кэше
    if(HashT->lists_arr[hash1] == NULL) {

        struct node *node = (struct node *)calloc(1, sizeof(struct node));
        assert(node != NULL);
        node->val = val;

        if(HashT->inserts >= capacity) {

            unsigned long long hash2 = HashT->list_tail->val;
            HashT->lists_arr[hash2] = NULL;
            HashT->list_tail = HashT->list_tail->prev;
            free(HashT->list_tail->next);
            HashT->list_tail->next = NULL;

        }

        HashT->lists_arr[hash1] = node;
        node->next = HashT->list_head;

        if(HashT->inserts == 0) {

            HashT->list_head = node;
            HashT->list_tail = node;

        }

        else {

            HashT->list_head->prev = node;
            HashT->list_head = node;

        }

        HashT->inserts += 1;

    }

    //Если страница есть в кэше
    else {

        if(HashT->lists_arr[hash1] != HashT->list_head) {

            HashT->lists_arr[hash1]->prev->next = HashT->lists_arr[hash1]->next;


            if(HashT->lists_arr[hash1]->next != NULL) {
                HashT->lists_arr[hash1]->next->prev = HashT->lists_arr[hash1]->prev;
            }
            else {
                HashT->list_tail = HashT->list_tail->prev;
            }

            HashT->lists_arr[hash1]->next = HashT->list_head;
            HashT->lists_arr[hash1]->prev = NULL;
            HashT->list_head->prev = HashT->lists_arr[hash1];
            HashT->list_head = HashT->lists_arr[hash1];

        }
        hits = 1;
    }

    return hits;
}

//Тупо удаляет список через указательна на head
void DeleteList(struct node *top) {

    struct node *cur_node = top;
    struct node *next_node = top;

    while(cur_node->next != NULL) {

        next_node = cur_node->next;
        free(cur_node);
        cur_node = next_node;
    }

    free(cur_node);
}
//Удаляет и таблицу и список
void DeleteHashtable(struct Hashtable *HashT) {

    DeleteList(HashT->list_head);
    free(HashT->lists_arr);
    free(HashT);

}

//Подсчитывает количество попаданий в целом
int RequestInput(struct Hashtable *HashT, int m, int n) {

    unsigned long long request = 0;
    int res = 0;
    int hits = 0;
    int i = 0;

    for(i = 0; i < n; ++i) {

        res = scanf("%llu", &request);
        assert(res);
        hits = hits + HashtableInsert(HashT, m, request);

    }
    return hits;

}

int main() {

    int res = 0;
    int m = 0;
    int n = 0;
    int hits = 0;
    struct Hashtable *HashT = NULL;
    unsigned long long Hash_size = 500000;

    res = scanf("%d", &m);
    assert(res);
    res = scanf("%d", &n);
    assert(res);

    HashT = HashtableInit(Hash_size);

    hits = RequestInput(HashT, m, n);

    printf("%d", hits);

    DeleteHashtable(HashT);

    return 0;

}
