typedef struct HashMapEntry
{
    char *key;
    void *value;
    struct HashMapEntry *next;
} HashMapEntry;

#define HASHMAP_SIZE 10240
typedef struct
{
    HashMapEntry **buckets;
} HashMap;

HashMap *init_hashmap();
unsigned long hash_string(const char *key);
unsigned long get_hash_index(const char *key);
void hashmap_insert(HashMap *hashmap, char *key, void *value);
void *hashmap_find(HashMap *map, const char *key);
void free_hashmap(HashMap *map, void (*free_entry)(void *));