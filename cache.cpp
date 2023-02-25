// #include <stdlib.h>
#include <iostream>
#include "oneapi/tbb/concurrent_hash_map.h"
#include "oneapi/tbb/blocked_range.h"
#include "oneapi/tbb/parallel_for.h"
#include <string>

/* This cache is implemented by using tbb concurrent hash map and doubly linked list
with <string,<string, pointer>> as keys and values
*/

static int CAPACITY = 20;

typedef std::string Key;
typedef std::string Val;
typedef struct node{
    node* next;
    node* prev;
    Key key;
    Val val;
}node;
typedef std::pair<std::string, node*> Hval;



typedef struct list{
    node* head;
    node* tail;
    int size; 
}list;


list* list_new(){
    list* l = (list*)malloc(sizeof(list));
    l -> size = 0;
    l -> head = nullptr;
    l -> tail = nullptr;
    return l;
}

node* list_add(list* l, Key key, Val val)
{
    node* n = (node*)malloc(sizeof(node));
    n -> next = nullptr;
    n -> prev = nullptr;
    n -> val = val;
    n -> key = key;

    if (l -> size == 0)
    {
        l -> head = n;
        l -> tail = n;
        l -> size++;
    }else if (l -> size < CAPACITY){
        n -> next = l -> head;
        l -> head = n;
        l -> size++;
    }else{
        assert(l -> size == CAPACITY);
        // evict the data on the last
        // delete the pointer in the hashmap
    }

    return n;
}

typedef tbb::concurrent_hash_map<Key, Hval> cacheTable;
typedef struct cache{
    cacheTable& cT;
    list* l;
}cache;

cache* cache_new(){
    cache* c = (cache*)malloc(sizeof(cache));
    c -> l = list_new();
    return c;
}

int cache_insert(cache* c, Key key, Val val)
{
    node* n = list_add(c -> l, key, val);
    Hval pair = std::make_pair(val, n);
    cacheTable::value_type val_pair = std::make_pair(key, pair);
    cacheTable::accessor a;
    c ->cT.insert(a, val_pair);
}

Val cache_find(cache* c, Key key)
{
    
}






int main()
{
    return 0;
}