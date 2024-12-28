/*
 * Author: Caden Parajuli - caden.parajuli@bc.edu
 */

/* Header guards */
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define HASH_TABLE_DEFAULT_SIZE 16
#define HASH_TABLE_MAX_FILL 0.7

#include <stdint.h>
#include <stdlib.h>

typedef struct HashTable HashTable;
typedef struct TableEntry TableEntry;

struct HashTable {
    TableEntry ** entries;
    size_t capacity;
    size_t length;
};

struct TableEntry {
    const char * key;
    void * value;
    uint64_t hash;
    size_t psl;
};

HashTable * new_hash_table(size_t capacity);
void free_hash_table(HashTable * table);
void * get_entry(HashTable * table, const char * key);
int set_entry(HashTable * table, const char * key, void * value);
int expand_hash_table(HashTable * table);
int unset_entry(HashTable * table, const char * key);

#endif
