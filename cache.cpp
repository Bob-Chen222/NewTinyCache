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
    if(n == nullptr)
    {
        return -1;
    }
    Hval pair = std::make_pair(val, n);
    cacheTable::value_type val_pair = std::make_pair(key, pair);
    cacheTable::accessor w;
    if(c -> cT.insert(w, val_pair))
    {
        //means success
        return 0;
    }else{
        //means failure
        return -1;
    }
}

bool cache_find(cache* c, Key key, Val& val)
{
    cacheTable::const_accessor const_a;
    if (c -> cT.find(const_a, key))
    {
        val = const_a -> second.first;
        return true;
    }else{
        return false;
    }
}






int main()
{
    //test
    tbb::concurrent_hash_map<std::string, std::string> ct;
    tbb::concurrent_hash_map<std::string, std::string>::accessor a;
    tbb::concurrent_hash_map<std::string, std::string>::const_accessor c;
    std::pair<std::string, std::string> p = std::make_pair("1", "that is it");
    ct.insert(a, p);
    // ct.find(a, "1");
    std::pair<std::string, std::string> another = std::make_pair("3", "4");
    std::cout << a -> second << std::endl;
    ct.find(c, "3");
    ct.insert(c, "5");
    std::cout << c -> first + "empty" << std::endl;

    // std::cout << a -> second;
    return 0;
}