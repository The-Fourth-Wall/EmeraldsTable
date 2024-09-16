/**
 * DOP table, open addressing, linear probing, komihash hashing.
 * Copyright (C) 2024  oblivious

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __EMERALDS_HASHTABLE_H_
#define __EMERALDS_HASHTABLE_H_

#include "../libs/EmeraldsBool/export/EmeraldsBool.h"
#include "../libs/EmeraldsVector/export/EmeraldsVector.h"
#include "hash/komihash/komihash.h"

#define TABLE_BITS_63_MASK    (0x7fffffffffffffff)
#define TABLE_BITS_1_MASK     (0x8000000000000000)
#define TABLE_BUCKET_HASH(b)  ((*b) & TABLE_BITS_63_MASK)
#define TABLE_BUCKET_STATE(b) (((*b) & TABLE_BITS_1_MASK) >> 63)
#define TABLE_BUCKET_PACK(h, s) \
  (((h) & TABLE_BITS_63_MASK) | ((size_t)((s) & 1) << 63))

#define TABLE_STATE_EMPTY         (0)
#define TABLE_STATE_FILLED        (1)
#define TABLE_BUCKET_IS_EMPTY(b)  (TABLE_BUCKET_STATE(b) == TABLE_STATE_EMPTY)
#define TABLE_BUCKET_IS_FILLED(b) (TABLE_BUCKET_STATE(b) == TABLE_STATE_FILLED)

#define TABLE_INITIAL_SIZE  (1024)
#define TABLE_LOAD_FACTOR   (0.75)
#define TABLE_GROW_FACTOR   (2)
#define TABLE_HASH_FUNCTION komihash_hash

/** @brief Since values are integers, NULL is not allowed and we define a NaN
 * boxed undefined value */
#define TABLE_UNDEFINED (0xfffc000000000000)

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
 * @brief Generic bucket finder
 * @param buckets -> The buckets vector
 * @param hash -> The hash of the key
 * @param keys -> The keys vector
 * @param key -> The key to find
 * @param find_empty -> A flag for when we are adding new keys
 * @return size_t* -> The pointer to the bucket or NULL if not found
 */
p_inline size_t *_table_find_bucket(
  size_t *buckets,
  size_t hash,
  const char **keys,
  const char *key,
  bool find_empty
) {
  size_t bucket_count = vector_capacity(buckets);
  size_t bucket_index = hash & (bucket_count - 1);

  for(size_t i = 0; i < bucket_count; ++i) {
    size_t *bucket = &buckets[bucket_index];
    size_t state   = TABLE_BUCKET_STATE(bucket);

    if(state == TABLE_STATE_EMPTY && !find_empty) {
      return NULL;
    } else if(state == TABLE_STATE_EMPTY && find_empty) {
      return bucket;
    } else if(state == TABLE_STATE_FILLED &&
              TABLE_BUCKET_HASH(bucket) == hash &&
              strcmp(keys[bucket_index], key) == 0) {
      return bucket;
    }

    bucket_index = (bucket_index + 1) & (bucket_count - 1);
  }

  return NULL;
}

/**
 * @brief Rehashes when bucket count reaches the load factor
 * @param self -> The hash table
 */
#define _table_rehash(self)                                                      \
  do {                                                                           \
    size_t i;                                                                    \
    size_t *buckets_new   = NULL;                                                \
    const char **keys_new = NULL;                                                \
    size_t *values_new    = NULL;                                                \
    size_t bucket_size_new =                                                     \
      vector_capacity((self)->buckets) * TABLE_GROW_FACTOR;                      \
    if(bucket_size_new < TABLE_INITIAL_SIZE) {                                   \
      bucket_size_new = TABLE_INITIAL_SIZE;                                      \
    }                                                                            \
    vector_initialize_n(buckets_new, bucket_size_new);                           \
    vector_initialize_n(keys_new, bucket_size_new);                              \
    vector_initialize_n(values_new, bucket_size_new);                            \
    for(i = 0; i < vector_capacity((self)->buckets); i++) {                      \
      size_t hash;                                                               \
      size_t *target = NULL;                                                     \
      size_t *bucket = &(self)->buckets[i];                                      \
      if(!TABLE_BUCKET_IS_FILLED(bucket)) {                                      \
        continue;                                                                \
      }                                                                          \
      hash   = TABLE_BUCKET_HASH(bucket);                                        \
      target = _table_find_bucket(                                               \
        buckets_new, hash, keys_new, (self)->keys[i], true                       \
      );                                                                         \
      if(target != NULL) {                                                       \
        size_t bucket_target;                                                    \
        *target                   = TABLE_BUCKET_PACK(hash, TABLE_STATE_FILLED); \
        bucket_target             = target - buckets_new;                        \
        keys_new[bucket_target]   = (self)->keys[i];                             \
        values_new[bucket_target] = (self)->values[i];                           \
      }                                                                          \
    }                                                                            \
    vector_free((self)->buckets);                                                \
    vector_free((self)->keys);                                                   \
    vector_free((self)->values);                                                 \
    (self)->buckets = buckets_new;                                               \
    (self)->keys    = keys_new;                                                  \
    (self)->values  = values_new;                                                \
  } while(0)

/**
 * @brief Initializes the hash table
 * @param self
 */
#define table_init(self)                                      \
  do {                                                        \
    vector_initialize_n((self)->buckets, TABLE_INITIAL_SIZE); \
    vector_initialize_n((self)->keys, TABLE_INITIAL_SIZE);    \
    vector_initialize_n((self)->values, TABLE_INITIAL_SIZE);  \
    (self)->size = 0;                                         \
  } while(0)

/**
 * @brief Inserts a key-value pair into the hash table (open addressing)
 * @param self -> The hash table
 * @param key -> The key
 * @param value -> The value
 */
#define table_add(self, key, value)                                             \
  do {                                                                          \
    size_t hash;                                                                \
    size_t *bucket = NULL;                                                      \
    if((self)->size > vector_capacity((self)->buckets) * TABLE_LOAD_FACTOR) {   \
      _table_rehash((self));                                                    \
    }                                                                           \
    hash = TABLE_HASH_FUNCTION(key, strlen((key))) & TABLE_BITS_63_MASK;        \
    bucket =                                                                    \
      _table_find_bucket((self)->buckets, hash, (self)->keys, (key), true);     \
    if(bucket != NULL) {                                                        \
      size_t bucket_index;                                                      \
      *bucket                    = TABLE_BUCKET_PACK(hash, TABLE_STATE_FILLED); \
      bucket_index               = bucket - (self)->buckets;                    \
      (self)->keys[bucket_index] = (key);                                       \
      (self)->values[bucket_index] = (value);                                   \
      (self)->size++;                                                           \
    }                                                                           \
  } while(0)

/**
 * @brief Adds all entries from src to dest
 * @param src -> Initial table
 * @param dest -> New table
 */
#define table_add_all(src, dst)                             \
  do {                                                      \
    size_t i;                                               \
    for(i = 0; i < vector_capacity((src)->buckets); i++) {  \
      size_t *bucket = &(src)->buckets[i];                  \
      if(TABLE_BUCKET_IS_FILLED(bucket)) {                  \
        table_add((dst), (src)->keys[i], (src)->values[i]); \
      }                                                     \
    }                                                       \
  } while(0)

/**
 * @brief Linear probing lookup
 * @param self -> The hash table
 * @param key -> The key
 * @return size_t -> Either the value found or 0xfffc000000000000 if not found
 */
p_inline size_t table_get(EmeraldsHashtable *self, const char *key) {
  size_t hash = TABLE_HASH_FUNCTION(key, strlen(key)) & TABLE_BITS_63_MASK;
  size_t *bucket =
    _table_find_bucket(self->buckets, hash, self->keys, key, false);

  if(bucket != NULL && TABLE_BUCKET_IS_FILLED(bucket)) {
    size_t bucket_index = bucket - self->buckets;
    return self->values[bucket_index];
  } else {
    return TABLE_UNDEFINED;
  }
}

/**
 * @brief Removes a key-value pair from the hash table
 * @param self -> The hash table
 * @param key -> The key
 */
#define table_remove(self, key)                                              \
  do {                                                                       \
    size_t hash =                                                            \
      TABLE_HASH_FUNCTION(key, strlen((key))) & TABLE_BITS_63_MASK;          \
    size_t *bucket =                                                         \
      _table_find_bucket((self)->buckets, hash, (self)->keys, (key), false); \
    if(bucket != NULL && TABLE_BUCKET_IS_FILLED(bucket)) {                   \
      *bucket = TABLE_BUCKET_PACK(0, TABLE_STATE_EMPTY);                     \
      (self)->size--;                                                        \
    }                                                                        \
  } while(0)

/**
 * @brief Deallocates all vectors (hashtable exists on stack)
 * @param self -> The hash table
 */
#define table_deinit(self)        \
  do {                            \
    vector_free((self)->buckets); \
    vector_free((self)->keys);    \
    vector_free((self)->values);  \
  } while(0)

#endif
