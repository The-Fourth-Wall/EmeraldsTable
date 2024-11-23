#ifndef __TABLE_H_
#define __TABLE_H_

#include "../../libs/EmeraldsBool/export/EmeraldsBool.h"
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h"
#include "../hash/komihash/komihash.h"

/** @brief Can dynamically redefine those constants Since values are integers,
 * NULL is not allowed and we define a NaN boxed undefined value */
#ifndef TABLE_UNDEFINED
  #define TABLE_UNDEFINED 0xfffc000000000000
#endif

#ifndef TABLE_LOAD_FACTOR
  #define TABLE_LOAD_FACTOR 0.78
#endif

#ifndef TABLE_INITIAL_SIZE
  #define TABLE_INITIAL_SIZE (1 << 10)
#endif

#ifndef TABLE_HASH_FUNCTION
  #define TABLE_HASH_FUNCTION komihash_hash
#endif

/**
 * @brief Data oriented table with open addressing and linear probing
 * @param keys -> The keys of the hash table
 * @param values -> The values of the hash table
 * @param hashes -> The hash values of the keys
 * @param states -> The state of each bucket (empty or filled)
 * @param size -> The number of elements in the hash table
 * @param tombstones -> The number of tombstones in the hash table
 */
typedef struct EmeraldsTable {
  const char **keys;
  size_t *values;
  size_t *hashes;
  uint8_t *states;
  size_t size;
  size_t tombstones;
} EmeraldsTable;

/**
 * @brief Initializes the hash table
 * @param self
 */
void table_init(EmeraldsTable *self);

/**
 * @brief Inserts a key-value pair into the hash table (open addressing)
 * @param self -> The hash table
 * @param key -> The key
 * @param value -> The value
 */
void table_add(EmeraldsTable *self, const char *key, size_t value);

/**
 * @brief Adds all entries from src to dest
 * @param src -> Initial table
 * @param dest -> New table
 */
void table_add_all(EmeraldsTable *src, EmeraldsTable *dst);

/**
 * @brief Adds all entries from src to dst except for values starting with `@::`
 * @param src -> Initial table
 * @param dst -> New table
 */
void table_add_all_non_labels(EmeraldsTable *src, EmeraldsTable *dst);

/**
 * @brief Linear probing lookup
 * @param self -> The hash table
 * @param key -> The key
 * @return size_t -> Either the value found or 0xfffc000000000000 if not found
 */
size_t table_get(EmeraldsTable *self, const char *key);

/**
 * @brief Removes a key-value pair from the hash table
 * @param self -> The hash table
 * @param key -> The key
 */
void table_remove(EmeraldsTable *self, const char *key);

/**
 * @brief Returns the size of the hash table
 * @param self -> The hash table
 * @return size_t -> The size of the hash table
 */
size_t table_size(EmeraldsTable *self);

/**
 * @brief Deallocates all vectors (hashtable exists on stack)
 * @param self -> The hash table
 */
void table_deinit(EmeraldsTable *self);

#endif
