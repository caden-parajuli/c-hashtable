/*
 * Author: Caden Parajuli - caden.parajuli@bc.edu
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

#define FNV_PRIME_64 0x100000001B3
#define FNV_BASIS_64 0xCBF29CE484222325

/** Hash function for use in the hash table */
static uint64_t fnv_hash_64(const char * key) {
    uint64_t hash = FNV_BASIS_64;
    for (const uint8_t * curr_loc = (const uint8_t *)key; *curr_loc; curr_loc++) {
        hash ^= *curr_loc;
        hash *= FNV_PRIME_64;
    }
    return hash;
}

/** Creates a new symbol table.
 *  Returns NULL upon failure */
HashTable * new_hash_table(size_t capacity) {
    /* Round capacity up to power of 2 */
    size_t new_cap = (size_t)(1ULL << (63 - __builtin_clzll((uint64_t)capacity)));
    if (new_cap != capacity) {
        capacity = new_cap << 1;
    }

    /* Allocate table space */
    HashTable * table = malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }
    table->length = 0;
    table->capacity = capacity;

    /* Allocate entry space */
    table->entries = calloc(table->capacity, sizeof(TableEntry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }
    return table;
}

/** Frees a hash table and all of its keys */
void free_hash_table(HashTable * table) {
    /* Free entries and keys */
    for (size_t index = 0; index < table->capacity; index++) {
        if (table->entries[index] != NULL) {
            free((void *)table->entries[index]->key);
            free(table->entries[index]);
        }
    }
    /* Free array of entry pointers and table itself */
    free(table->entries);
    free(table);
}

/** Gets symbol from symbol table.
 *  Returns NULL if key is not in table */
void * get_entry(HashTable * table, const char * key) {
    uint64_t hash = fnv_hash_64(key);
    size_t index = hash & (table->capacity - 1);
    size_t psl = 0;

    /* Loop until there's an empty entry */
    while (table->entries[index] != NULL) {
        if (strcmp(key, table->entries[index]->key) == 0) {
            return table->entries[index]->value;
        }
        else if (table->entries[index]->psl < psl) {
            return NULL;
        }
        /* Wrong key, probe */
        psl++;
        index = (index + 1) & (table->capacity - 1);
    }
    return NULL;
}

/** Internal function for setting symbol value.
 *  Returns 1 if the value was already present, 0 if it was added */
static int set_entry_helper(TableEntry ** entries, TableEntry * entry, size_t capacity) {
    /* Loop until there's an empty entry */
    size_t index = entry->hash & (uint64_t)(capacity - 1);
    while (entries[index] != NULL) {
        if (entry->psl > entries[index]->psl) {
            /* Robin Hood swap */
            TableEntry * temp_entry = entries[index];
            entries[index] = entry;
            entry = temp_entry;
        }
        else if (strcmp(entry->key, entries[index]->key) == 0) {
            /* Key found */
            entries[index]->value = entry->value; /* Used to update value */
            return 1;
        }
        /* Index is taken, probe */
        entry->psl++;
        index = (index + 1) & (capacity - 1);;
    }    
    /* Set empty entry */
    entries[index] = entry;
    return 0;
}

/** Sets entry value.
 *  Returns 1 if value was already present, and 0 if it was added */
int set_entry(HashTable * table, const char * key, void * value) {
    TableEntry * entry = malloc(sizeof(TableEntry));
    if (entry == NULL) {
        return 1;
    }
    entry->key = strdup(key);
    entry->hash = fnv_hash_64(key);
    entry->value = value;
    entry->psl = 0;
    /* resize if necessary */
    if (table->length > (size_t)(HASH_TABLE_MAX_FILL * (float)table->capacity)) {
        if (expand_hash_table(table)) {
            return 1;
        }
    }
    int ret = set_entry_helper(table->entries, entry, table->capacity);
    if (ret == 0) {
        /* Value added */
        table->length++;
    }
    else if (ret == 1) {
        free(entry);
    }
    return ret;
}

/** Expands a table. 
 *  Returns non-zero upon failure */
int expand_hash_table(HashTable * table) {
    TableEntry ** new_entries = calloc(2 * table->capacity, sizeof(TableEntry));
    if (new_entries == NULL) {
        return 1;
    }
    for (size_t index = 0; index < table->capacity; index++) {
        if (table->entries[index] != NULL) {
            table->entries[index]->psl = 0;
            if (set_entry_helper(new_entries, table->entries[index], 2 * table->capacity) == 1) {
                return 1;
            }
        }
    }
    free(table->entries);
    table->entries = new_entries;
    table->capacity *= 2;
    return 0;
}

/** Remove a mapping from a hash table. 
 *  Returns 1 if the entry is not present in the table */
int unset_entry(HashTable * table, const char * key) {
    size_t index = fnv_hash_64(key) & (table->capacity - 1);

    /* Probe for entry */
    size_t psl = 0;
    while (table->entries[index] != NULL) {
        if (table->entries[index]->psl < psl) {
            return 1;
        }
        else if (strcmp(key, table->entries[index]->key) == 0) {
            break;
        }
        /* Wrong key, probe */
        psl++;
        index = (index + 1) & (table->capacity - 1);
    }
    if (table->entries[index] == NULL) {
        return 1;
    }
    
    /* Delete the entry */
    free((void *)table->entries[index]->key);
    free(table->entries[index]);
    index = (index + 1) & (table->capacity - 1);

    /* Backward shift */
    while (table->entries[index] != NULL && table->entries[index]->psl != 0) {
        table->entries[index]->psl -= 1;
        table->entries[index ? (index - 1) : (table->capacity - 1)] = table->entries[index];
        index = (index + 1) & (table->capacity - 1);
    }

    table->entries[index ? (index - 1) : (table->capacity - 1)] = NULL;
    return 0;
}
