#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct HashMapEntry
{
    char *key;
    void *value;
    struct HashMapEntry *next;
} HashMapEntry;

typedef struct
{
    HashMapEntry **buckets;
} HashMap;

HashMap *init_hashmap();
unsigned long hash_string(const char *key);
unsigned long get_hash_index(const char *key);
void hashmap_insert(HashMap *hashmap, char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);
void free_hashmap(HashMap *map, void (*free_entry)(void *));

#endif