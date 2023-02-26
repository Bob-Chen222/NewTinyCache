// #include <stdlib.h>
#include <iostream>
#include "oneapi/tbb/concurrent_hash_map.h"
#include "oneapi/tbb/blocked_range.h"
#include "oneapi/tbb/parallel_for.h"
#include <string>
#include <mutex>

/* This cache is implemented by using tbb concurrent hash map and doubly linked list
with <string,<string, pointer>> as keys and values
*/

//randomly setting this as 20
static int CAPACITY = 20;
std::mutex mtx;
static bool evicting;

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

typedef tbb::concurrent_hash_map<Key, Hval> cacheTable;
typedef struct cache{
    cacheTable& cT;
    list* l;
}cache;

//evict data if there are more data than the cache can hold
bool evict(cache* c)
{
    if (!evicting)
    {
        evicting = true;

        if (c -> l -> size == 0) return false;

        node* last = c -> l -> tail;
        c -> l -> tail = last -> prev;
        c -> l -> tail -> next = nullptr;
        c -> l -> size--;

        c -> cT.erase(last -> key);

        delete(last);
        return true;
    }
}

list* list_new(){
    list* l = (list*)malloc(sizeof(list));
    l -> size = 0;
    l -> head = nullptr;
    l -> tail = nullptr;
    return l;
}
node* list_add(cache* c, list* l, Key key, Val val)
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
        mtx.lock();
        //other threads might already evicted the data
        if (l -> size >= CAPACITY)
        {
            evict(c);
        }
        mtx.unlock();

        n -> next = l -> head;
        l -> head = n;
        l -> size++;
    }

    return n;
}

void to_front(list* l, node* n)
{
  if(l -> size == 1){
    return;
  }else if (l -> tail == n){
    l -> tail = n -> prev;
    l -> tail -> next = nullptr;

    n -> prev = nullptr;
    n -> next = l -> head;

    l -> head = n;
    return;
  }else if (l -> head == n){
    return;
  }else{
    n -> prev -> next = n -> next;
    n -> next -> prev = n -> prev;

    n -> prev = nullptr;
    n -> next = l -> head;

    l -> head = n;
  }
}


cache* cache_new(){
    cache* c = (cache*)malloc(sizeof(cache));
    c -> l = list_new();
    return c;
}

bool cache_insert(cache* c, Key key, Val val)
{
    node* n = list_add(c, c -> l, key, val);
    if(n == nullptr)
    {
        return false;
    }
    Hval pair = std::make_pair(val, n);
    cacheTable::value_type val_pair = std::make_pair(key, pair);
    cacheTable::accessor w;
    if(c -> cT.insert(w, val_pair))
    {
        //means success
        return true;
    }else{
        //means failure
        return false;
    }
}

bool cache_find(cache* c, Key key, Val& val)
{
    cacheTable::const_accessor const_a;
    if (c -> cT.find(const_a, key))
    {
        val = const_a -> second.first;
        mtx.lock();
        to_front(c -> l, const_a -> second.second);
        mtx.unlock();
        return true;
    }else{
        return false;
    }
}


int main()
{
    return 0;
}