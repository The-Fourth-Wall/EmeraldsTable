#include "table.h"

#include "../../libs/EmeraldsBool/export/EmeraldsBool.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsHash/export/EmeraldsHash.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */

#define BITS_63_MASK      (0x7fffffffffffffff)
#define BITS_1_MASK       (0x8000000000000000)
#define BUCKET_HASH(b)    ((*b) & BITS_63_MASK)
#define BUCKET_STATE(b)   (((*b) & BITS_1_MASK) >> 63)
#define BUCKET_PACK(h, s) (((h) & BITS_63_MASK) | ((size_t)((s) & 1) << 63))

#define STATE_EMPTY         0
#define STATE_FILLED        1
#define BUCKET_IS_EMPTY(b)  (BUCKET_STATE(b) == STATE_EMPTY)
#define BUCKET_IS_FILLED(b) (BUCKET_STATE(b) == STATE_FILLED)

#define INITIAL_SIZE  16
#define LOAD_FACTOR   0.75
#define GROW_FACTOR   2
#define HASH_FUNCTION komihash_hash

/** @brief Since values are integers, NULL is not allowed and we define a NaN
 * boxed undefined value */
#define TABLE_UNDEFINED 0xfffc000000000000

/**
 * @brief Generic bucket finder
 * @param buckets -> The buckets vector
 * @param hash -> The hash of the key
 * @param keys -> The keys vector
 * @param key -> The key to find
 * @param find_empty -> A flag for when we are adding new keys
 * @return size_t* -> The pointer to the bucket or NULL if not found
 */
static size_t *_table_find_bucket(
  size_t *buckets,
  size_t hash,
  const char **keys,
  const char *key,
  bool find_empty
) {
  size_t bucket_count = vector_capacity(buckets);
  size_t bucket_start = hash & (bucket_count - 1);
  size_t *target      = NULL;

  for(size_t i = bucket_start; i < bucket_count; i++) {
    size_t *bucket = &buckets[i];
    if(BUCKET_IS_EMPTY(bucket) && find_empty) {
      return bucket;
    } else if(BUCKET_IS_FILLED(bucket) && BUCKET_HASH(bucket) == hash &&
              strcmp(keys[i], key) == 0) {
      return bucket;
    } else if(find_empty && !BUCKET_IS_FILLED(bucket)) {
      target = bucket;
      break;
    }
  }

  for(size_t i = bucket_start; i > 0; i--) {
    size_t *bucket = &buckets[i];
    if(BUCKET_IS_EMPTY(bucket) && find_empty) {
      return bucket;
    } else if(BUCKET_IS_FILLED(bucket) && BUCKET_HASH(bucket) == hash &&
              strcmp(keys[i], key) == 0) {
      return bucket;
    } else if(find_empty && !BUCKET_IS_FILLED(bucket)) {
      target = bucket;
      break;
    }
  }

  return target;
}

/**
 * @brief Rehashes when bucket count reaches the load factor
 * @param self -> The hash table
 */
static void table_rehash(EmeraldsHashtable *self) {
  size_t bucket_size_new = vector_capacity(self->buckets) * GROW_FACTOR;
  if(bucket_size_new < INITIAL_SIZE) {
    bucket_size_new = INITIAL_SIZE;
  }

  size_t *buckets_new = NULL;
  vector_initialize_n(buckets_new, bucket_size_new);
  const char **keys_new = NULL;
  vector_initialize_n(keys_new, bucket_size_new);
  size_t *values_new = NULL;
  vector_initialize_n(values_new, bucket_size_new);

  for(size_t i = 0; i < vector_capacity(self->buckets); i++) {
    size_t *bucket = &self->buckets[i];
    if(!BUCKET_IS_FILLED(bucket)) {
      continue;
    }

    size_t hash = BUCKET_HASH(bucket);
    size_t *target =
      _table_find_bucket(buckets_new, hash, keys_new, self->keys[i], true);
    if(target != NULL) {
      *target                   = BUCKET_PACK(hash, STATE_FILLED);
      size_t bucket_target      = target - buckets_new;
      keys_new[bucket_target]   = self->keys[i];
      values_new[bucket_target] = self->values[i];
    }
  }

  vector_free(self->buckets);
  vector_free(self->keys);
  vector_free(self->values);
  self->buckets = buckets_new;
  self->keys    = keys_new;
  self->values  = values_new;
}

EmeraldsHashtable *table_new(void) {
  EmeraldsHashtable *self =
    (EmeraldsHashtable *)malloc(sizeof(EmeraldsHashtable));

  vector_initialize_n(self->buckets, INITIAL_SIZE);
  vector_initialize_n(self->keys, INITIAL_SIZE);
  vector_initialize_n(self->values, INITIAL_SIZE);
  self->size = 0;

  return self;
}

void table_init(EmeraldsHashtable *self) {
  vector_initialize_n(self->buckets, INITIAL_SIZE);
  vector_initialize_n(self->keys, INITIAL_SIZE);
  vector_initialize_n(self->values, INITIAL_SIZE);
  self->size = 0;
}

void table_add(EmeraldsHashtable *self, const char *key, size_t value) {
  if(self->size > vector_capacity(self->buckets) * LOAD_FACTOR) {
    table_rehash(self);
  }

  size_t hash = HASH_FUNCTION(key) & BITS_63_MASK;
  size_t *bucket =
    _table_find_bucket(self->buckets, hash, self->keys, key, true);

  if(bucket != NULL) {
    *bucket                    = BUCKET_PACK(hash, STATE_FILLED);
    size_t bucket_index        = bucket - self->buckets;
    self->keys[bucket_index]   = key;
    self->values[bucket_index] = value;
    self->size++;
  }
}

void table_add_all(EmeraldsHashtable *src, EmeraldsHashtable *dst) {
  for(size_t i = 0; i < src->size; i++) {
    table_add(dst, src->keys[i], src->values[i]);
  }
}

size_t table_get(EmeraldsHashtable *self, const char *key) {
  size_t hash = HASH_FUNCTION(key) & BITS_63_MASK;
  size_t *bucket =
    _table_find_bucket(self->buckets, hash, self->keys, key, false);

  if(bucket != NULL && BUCKET_IS_FILLED(bucket)) {
    size_t bucket_index = bucket - self->buckets;
    return self->values[bucket_index];
  } else {
    return TABLE_UNDEFINED;
  }
}

void table_remove(struct EmeraldsHashtable *self, const char *key) {
  size_t hash = HASH_FUNCTION(key) & BITS_63_MASK;
  size_t *bucket =
    _table_find_bucket(self->buckets, hash, self->keys, key, false);

  if(bucket != NULL && BUCKET_IS_FILLED(bucket)) {
    *bucket = BUCKET_PACK(0, STATE_EMPTY);
    self->size--;
  }
}
