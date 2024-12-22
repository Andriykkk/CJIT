#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

unsigned long hash_string(const char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

unsigned long get_hash_index(const char *key)
{
    return hash_string(key) % HASHMAP_SIZE;
}

HashMap *init_hashmap()
{
    HashMap *hashmap = malloc(sizeof(HashMap));
    hashmap->buckets = malloc(HASHMAP_SIZE * sizeof(HashMapEntry *));

    for (int i = 0; i < HASHMAP_SIZE; i++)
    {
        hashmap->buckets[i] = NULL;
    }

    return hashmap;
}

void hashmap_insert(HashMap *hashmap, char *key, void *value)
{
    unsigned long index = get_hash_index(key);
    HashMapEntry *entry = malloc(sizeof(HashMapEntry));
    entry->key = strdup(key);
    entry->value = value;
    entry->next = hashmap->buckets[index];
    hashmap->buckets[index] = entry;
}

void *hashmap_find(HashMap *map, const char *key)
{
    unsigned long index = get_hash_index(key);
    HashMapEntry *entry = map->buckets[index];

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void free_hashmap(HashMap *map, void (*free_entry)(void *))
{
    for (int i = 0; i < HASHMAP_SIZE; i++)
    {
        HashMapEntry *entry = map->buckets[i];

        while (entry != NULL)
        {
            HashMapEntry *temp = entry;
            entry = temp->next;
            free(entry->key);
            free_entry(entry->value);
        }
    }

    free(map->buckets);
    free(map);
}