#ifndef __TABLE_H_
#define __TABLE_H_

#include "../../libs/EmeraldsBool/export/EmeraldsBool.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

#define EMERALDS_HASHTABLE_STATE_EMPTY   0
#define EMERALDS_HASHTABLE_STATE_FILLED  1
#define EMERALDS_HASHTABLE_STATE_REMOVED 2

/**
 * @brief An entry bucket of the hash table
 * @param hash -> A 64 hash truncated to 62 bits
 * @param state -> The filled state
 */
typedef struct EmeraldsHashtableBucket {
  // NOTE - Align bucket to 64 by tagging the state
  size_t hash  : 62;
  uint8_t state: 2;
} EmeraldsHashtableBucket;

/**
 * @brief Data oriented table with open addressing and linear probing
 * @param keys -> The keys of the hash table
 * @param values -> The values of the hash table
 * @param buckets -> The buckets of the hash table
 * @param size -> The size of the hash table
 */
typedef struct EmeraldsHashtable {
  const char **keys;
  size_t *values;
  EmeraldsHashtableBucket *buckets;
  size_t size;
} EmeraldsHashtable;

void hashtable_init(EmeraldsHashtable *self);
void hashtable_rehash(EmeraldsHashtable *self, size_t bucket_count_new);
void hashtable_insert(EmeraldsHashtable *self, const char *key, size_t value);
size_t *hashtable_lookup(EmeraldsHashtable *self, const char *key);
bool hashtable_remove(struct EmeraldsHashtable *self, char *key);
void hashtable_free(EmeraldsHashtable *self);

#endif
