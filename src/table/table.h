#ifndef __TABLE_H_
#define __TABLE_H_

#include <stddef.h> /* size_t */

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
  size_t *buckets;
  size_t size;
} EmeraldsHashtable;

/**
 * @brief Creates a new hash table
 * @return EmeraldsHashtable* -> A new allocation
 */
EmeraldsHashtable *table_new(void);

/**
 * @brief Inserts a key-value pair into the hash table (open addressing)
 * @param self -> The hash table
 * @param key -> The key
 * @param value -> The value
 */
void table_add(EmeraldsHashtable *self, const char *key, size_t value);

/**
 * @brief Linear probing lookup
 * @param self -> The hash table
 * @param key -> The key
 * @return size_t -> Either the value found or 0xfffc000000000000 if not found
 */
size_t table_get(EmeraldsHashtable *self, const char *key);

/**
 * @brief Removes a key-value pair from the hash table
 * @param self -> The hash table
 * @param key -> The key
 */
void table_remove(struct EmeraldsHashtable *self, const char *key);

/**
 * @brief Deallocates all vectors
 * @param self -> The hash table
 */
#define table_free(self)        \
  do {                          \
    vector_free(self->buckets); \
    vector_free(self->keys);    \
    vector_free(self->values);  \
    free(self);                 \
    self = NULL;                \
  } while(0)

#endif
