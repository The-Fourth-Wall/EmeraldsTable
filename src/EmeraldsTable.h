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

#define TABLE_STATE_EMPTY   (0)
#define TABLE_STATE_FILLED  (1)
#define TABLE_STATE_DELETED (2)

#define TABLE_GROW_FACTOR (2)

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
 * @brief Generic bucket finder
 * @param hashes -> The hashes array
 * @param states -> The states array
 * @param hash -> The hash of the key
 * @param keys -> The keys vector
 * @param key -> The key to find
 * @param find_empty -> A flag for when we are adding new keys
 * @return size_t -> The index of the bucket or TABLE_UNDEFINED if not found
 */
p_inline size_t _table_find_bucket(
  size_t *hashes,
  uint8_t *states,
  size_t hash,
  const char **keys,
  const char *key,
  bool find_empty
) {
  size_t i;
  size_t bucket_count  = vector_capacity(keys);
  size_t bucket_index  = hash & (bucket_count - 1);
  size_t first_deleted = TABLE_UNDEFINED;

  for(i = 0; i < bucket_count; i++) {
    if(states[bucket_index] == TABLE_STATE_EMPTY) {
      if(find_empty) {
        return (first_deleted != TABLE_UNDEFINED) ? first_deleted
                                                  : bucket_index;
      } else {
        return TABLE_UNDEFINED;
      }
    } else if(states[bucket_index] == TABLE_STATE_DELETED) {
      if(find_empty && first_deleted == TABLE_UNDEFINED) {
        first_deleted = bucket_index;
      }
    } else if(hashes[bucket_index] == hash &&
              strcmp(keys[bucket_index], key) == 0) {
      return bucket_index;
    }

    bucket_index = (bucket_index + 1) & (bucket_count - 1);
  }

  return TABLE_UNDEFINED;
}

/**
 * @brief Rehashes when bucket count reaches the load factor
 * @param self -> The hash table
 */
#define _table_rehash(self)                                                    \
  do {                                                                         \
    size_t i;                                                                  \
    size_t *hashes_new    = NULL;                                              \
    uint8_t *states_new   = NULL;                                              \
    const char **keys_new = NULL;                                              \
    size_t *values_new    = NULL;                                              \
    size_t capacity       = vector_capacity((self)->keys);                     \
    size_t capacity_new   = vector_capacity((self)->keys) * TABLE_GROW_FACTOR; \
    if(capacity_new < TABLE_INITIAL_SIZE) {                                    \
      capacity_new = TABLE_INITIAL_SIZE;                                       \
    }                                                                          \
    vector_initialize_n(hashes_new, capacity_new);                             \
    vector_initialize_n(states_new, capacity_new);                             \
    vector_initialize_n(keys_new, capacity_new);                               \
    vector_initialize_n(values_new, capacity_new);                             \
    for(i = 0; i < capacity; i++) {                                            \
      if((self)->states[i] == TABLE_STATE_FILLED) {                            \
        size_t hash         = (self)->hashes[i];                               \
        size_t bucket_index = hash & (capacity_new - 1);                       \
        while(states_new[bucket_index] == TABLE_STATE_FILLED) {                \
          bucket_index = (bucket_index + 1) & (capacity_new - 1);              \
        }                                                                      \
        hashes_new[bucket_index] = hash;                                       \
        states_new[bucket_index] = TABLE_STATE_FILLED;                         \
        keys_new[bucket_index]   = (self)->keys[i];                            \
        values_new[bucket_index] = (self)->values[i];                          \
      }                                                                        \
    }                                                                          \
    vector_free((self)->keys);                                                 \
    vector_free((self)->hashes);                                               \
    vector_free((self)->values);                                               \
    vector_free((self)->states);                                               \
    (self)->keys       = keys_new;                                             \
    (self)->hashes     = hashes_new;                                           \
    (self)->values     = values_new;                                           \
    (self)->states     = states_new;                                           \
    (self)->tombstones = 0;                                                    \
  } while(0)

/**
 * @brief Initializes the hash table
 * @param self
 */
#define table_init(self)                                     \
  do {                                                       \
    vector_initialize_n((self)->keys, TABLE_INITIAL_SIZE);   \
    vector_initialize_n((self)->values, TABLE_INITIAL_SIZE); \
    vector_initialize_n((self)->hashes, TABLE_INITIAL_SIZE); \
    vector_initialize_n((self)->states, TABLE_INITIAL_SIZE); \
    (self)->size       = 0;                                  \
    (self)->tombstones = 0;                                  \
  } while(0)

/**
 * @brief Inserts a key-value pair into the hash table (open addressing)
 * @param self -> The hash table
 * @param key -> The key
 * @param value -> The value
 */
#define table_add(self, key, value)                                   \
  do {                                                                \
    size_t hash;                                                      \
    size_t bucket_index;                                              \
    size_t prev_state;                                                \
    if((self)->size + (self)->tombstones >                            \
       vector_capacity((self)->keys) * TABLE_LOAD_FACTOR) {           \
      _table_rehash((self));                                          \
    }                                                                 \
    hash         = TABLE_HASH_FUNCTION(key, strlen((key)));           \
    bucket_index = _table_find_bucket(                                \
      (self)->hashes, (self)->states, hash, (self)->keys, (key), true \
    );                                                                \
    prev_state = (self)->states[bucket_index];                        \
    if(bucket_index != TABLE_UNDEFINED) {                             \
      (self)->hashes[bucket_index] = hash;                            \
      (self)->keys[bucket_index]   = (key);                           \
      (self)->values[bucket_index] = (value);                         \
      (self)->states[bucket_index] = TABLE_STATE_FILLED;              \
      if(prev_state != TABLE_STATE_FILLED) {                          \
        (self)->size++;                                               \
        if(prev_state == TABLE_STATE_DELETED) {                       \
          (self)->tombstones--;                                       \
        }                                                             \
      }                                                               \
    }                                                                 \
  } while(0)

/**
 * @brief Adds all entries from src to dest
 * @param src -> Initial table
 * @param dest -> New table
 */
#define table_add_all(src, dst)                             \
  do {                                                      \
    size_t i;                                               \
    for(i = 0; i < vector_capacity((src)->keys); i++) {     \
      if((src)->states[i] == TABLE_STATE_FILLED) {          \
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
p_inline size_t table_get(EmeraldsTable *self, const char *key) {
  size_t hash         = TABLE_HASH_FUNCTION(key, strlen(key));
  size_t bucket_index = _table_find_bucket(
    self->hashes, self->states, hash, self->keys, key, false
  );

  if(bucket_index != TABLE_UNDEFINED) {
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
#define table_remove(self, key)                                        \
  do {                                                                 \
    size_t hash         = TABLE_HASH_FUNCTION(key, strlen((key)));     \
    size_t bucket_index = _table_find_bucket(                          \
      (self)->hashes, (self)->states, hash, (self)->keys, (key), false \
    );                                                                 \
    if(bucket_index != TABLE_UNDEFINED) {                              \
      (self)->states[bucket_index] = TABLE_STATE_DELETED;              \
      (self)->size--;                                                  \
      (self)->tombstones++;                                            \
    }                                                                  \
  } while(0)

#define table_size(self) ((self)->size)

/**
 * @brief Deallocates all vectors (hashtable exists on stack)
 * @param self -> The hash table
 */
#define table_deinit(self)       \
  do {                           \
    vector_free((self)->hashes); \
    vector_free((self)->states); \
    vector_free((self)->keys);   \
    vector_free((self)->values); \
  } while(0)

#endif
