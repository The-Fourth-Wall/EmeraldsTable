#include "table.h"

#include "../../libs/EmeraldsString/export/EmeraldsString.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */
#include "komihash.h"

/**
 * @brief Initializes a vector with a given size
 * @param v -> The vector to initialize
 * @param n -> The size of the vector
 */
#define vector_initialize_n(v, n)   \
  do {                              \
    _vector_maybegrow(v, n);        \
    for(size_t i = 0; i < n; i++) { \
      v[i] = 0;                     \
    }                               \
  } while(0)

#define STATE_EMPTY   0
#define STATE_FILLED  1
#define STATE_REMOVED 2

// NOTE - Align bucket to 64 by tagging the state
#define BITS_62_MASK      (size_t)(0x3fffffffffffffff)
#define BITS_2_MASK       (size_t)(0xc000000000000000)
#define BUCKET_HASH(b)    ((*b) & BITS_62_MASK)
#define BUCKET_STATE(b)   (((*b) & BITS_2_MASK) >> 62)
#define BUCKET_PACK(h, s) (((h) & BITS_62_MASK) | ((size_t)((s) & 0x03) << 62))

// TODO - Remove this
#define INITIAL_SIZE 16

static size_t _table_hash_key(const char *key) {
  return komihash(key, sizeof(key), 0x0123456789ABCDEF);
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

void table_rehash(EmeraldsHashtable *self, size_t bucket_count_new) {
#define MAX(a, b) ((a) > (b) ? (a) : (b))
  // NOTE - Can't rehash down to smaller than current size or initial size
  bucket_count_new = MAX(MAX(bucket_count_new, self->size), INITIAL_SIZE);
#undef MAX

  size_t *buckets_new = NULL;
  vector_initialize_n(buckets_new, bucket_count_new);
  const char **keys_new = NULL;
  vector_initialize_n(keys_new, bucket_count_new);
  size_t *values_new = NULL;
  vector_initialize_n(values_new, bucket_count_new);

  for(size_t i = 0; i < vector_capacity(self->buckets); i++) {
    size_t *b = &self->buckets[i];
    if(BUCKET_STATE(b) != STATE_FILLED) {
      continue;
    }

    // Hash the key and find the starting bucket
    size_t hash         = BUCKET_HASH(b);
    size_t bucket_start = hash & (bucket_count_new - 1);

    size_t *target       = NULL;
    size_t bucket_target = 0;
    for(size_t j = bucket_start; j < bucket_count_new; j++) {
      size_t *bNew = &buckets_new[j];
      if(BUCKET_STATE(bNew) != STATE_FILLED) {
        target        = bNew;
        bucket_target = j;
        break;
      }
    }
    if(!target) {
      for(size_t j = bucket_start; j < 0; --j) {
        size_t *bNew = &buckets_new[j];
        if(BUCKET_STATE(bNew) != STATE_FILLED) {
          target        = bNew;
          bucket_target = j;
          break;
        }
      }
    }

    if(target != NULL) {
      *target                   = BUCKET_PACK(hash, STATE_FILLED);
      keys_new[bucket_target]   = self->keys[i];
      values_new[bucket_target] = self->values[i];
    }
  }

  // TODO - This (potentially) is a memory leak
  self->buckets = buckets_new;
  self->keys    = keys_new;
  self->values  = values_new;
}

void table_add(EmeraldsHashtable *self, const char *key, size_t value) {
  if(self->size * 3 > vector_capacity(self->buckets) * 2) {
    // NOTE - For slow key wrapping only rehash by a factor of 2
    table_rehash(self, vector_capacity(self->buckets) * 2);
  }

  size_t hash         = _table_hash_key(key) & BITS_62_MASK;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);

  size_t *target       = NULL;
  size_t bucket_target = 0;
  for(size_t i = bucket_start; i < vector_capacity(self->buckets); i++) {
    size_t *b = &self->buckets[i];
    if(BUCKET_STATE(b) != STATE_FILLED) {
      target        = b;
      bucket_target = i;
      break;
    }
  }
  if(!target) {
    for(size_t i = bucket_start; i < 0; --i) {
      size_t *b = &self->buckets[i];
      if(BUCKET_STATE(b) != STATE_FILLED) {
        target        = b;
        bucket_target = i;
        break;
      }
    }
  }

  if(target != NULL) {
    *target                     = BUCKET_PACK(hash, STATE_FILLED);
    self->keys[bucket_target]   = key;
    self->values[bucket_target] = value;
    self->size++;
  }
}

size_t *table_get(EmeraldsHashtable *self, const char *key) {
  size_t hash         = _table_hash_key(key) & BITS_62_MASK;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);

  for(size_t i = bucket_start; i < vector_capacity(self->buckets); i++) {
    size_t *b = &self->buckets[i];
    switch(BUCKET_STATE(b)) {
    case STATE_EMPTY:
      return NULL;
    case STATE_FILLED:
      if(BUCKET_HASH(b) == hash && string_equals(self->keys[i], key)) {
        return &self->values[i];
      }
      break;
    default:
      break;
    }
  }
  for(size_t i = bucket_start; i < 0; --i) {
    size_t *b = &self->buckets[i];
    switch(BUCKET_STATE(b)) {
    case STATE_EMPTY:
      return NULL;
    case STATE_FILLED:
      if(BUCKET_HASH(b) == hash && string_equals(self->keys[i], key)) {
        return &self->values[i];
      }
      break;
    default:
      break;
    }
  }

  return NULL;
}

bool table_remove(struct EmeraldsHashtable *self, char *key) {
  size_t hash         = _table_hash_key(key) & BITS_62_MASK;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);

  for(size_t i = bucket_start; i < vector_capacity(self->buckets); i++) {
    size_t *b = &self->buckets[i];
    switch(BUCKET_STATE(b)) {
    case STATE_EMPTY:
      return false;
    case STATE_FILLED:
      if(BUCKET_HASH(b) == hash && string_equals(self->keys[i], key)) {
        *b = BUCKET_PACK(0, STATE_REMOVED);
        --self->size;
        return true;
      }
      break;
    default:
      break;
    }
  }
  for(size_t i = bucket_start; i < 0; --i) {
    size_t *b = &self->buckets[i];
    switch(BUCKET_STATE(b)) {
    case STATE_EMPTY:
      return false;
    case STATE_FILLED:
      if(BUCKET_HASH(b) == hash && string_equals(self->keys[i], key)) {
        *b = BUCKET_PACK(0, STATE_REMOVED);
        --self->size;
        return true;
      }
      break;
    default:
      break;
    }
  }

  return false;
}

#undef STATE_EMPTY
#undef STATE_FILLED
#undef STATE_REMOVED

#undef BITS_62_MASK
#undef BITS_2_MASK
#undef BUCKET_HASH
#undef BUCKET_STATE
#undef BUCKET_PACK

#undef INITIAL_SIZE
