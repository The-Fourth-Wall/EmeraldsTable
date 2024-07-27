#include "table.h"

#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */

#include <stdio.h>

/**
 * @brief Initializes a vector with a given size
 * @param v -> The vector to initialize
 * @param n -> The size of the vector
 */
#define vector_initialize_n(v, n) _vector_maybegrow(v, n)

/**
 * @brief Swaps the contents of two vectors
 * @param a -> The first vector
 * @param b -> The second vector
 */
#define vector_swap(a, b)        \
  do {                           \
    void *tmp    = *(void **)&a; \
    *(void **)&a = *(void **)&b; \
    *(void **)&b = tmp;          \
  } while(0)

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxh3.h"

static const int initial_size = 16;
static const size_t bits_62   = 0x3fffffffffffffffULL;

size_t hash_key(const char *key) {
  return (size_t)XXH3_64bits(&key, sizeof(key));
}

void hashtable_init(EmeraldsHashtable *self) {
  vector_initialize_n(self->buckets, initial_size);
  vector_initialize_n(self->keys, initial_size);
  vector_initialize_n(self->values, initial_size);
  self->size = 0;
}

void hashtable_rehash(EmeraldsHashtable *self, size_t bucket_count_new) {
#define MAX(a, b) ((a) > (b) ? (a) : (b))
  // NOTE - Can't rehash down to smaller than current size or initial size
  bucket_count_new = MAX(MAX(bucket_count_new, self->size), initial_size);
#undef MAX

  EmeraldsHashtableBucket *buckets_new = NULL;
  vector_initialize_n(buckets_new, bucket_count_new);
  const char **keys_new = NULL;
  vector_initialize_n(keys_new, bucket_count_new);
  size_t *values_new = NULL;
  vector_initialize_n(values_new, bucket_count_new);

  for(size_t i = 0; i < vector_capacity(self->buckets); ++i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    if(b->state != EMERALDS_HASHTABLE_STATE_FILLED) {
      continue;
    }

    // Hash the key and find the starting bucket
    size_t hash         = b->hash;
    size_t bucket_start = hash & (bucket_count_new - 1);

    EmeraldsHashtableBucket *target = NULL;
    size_t bucket_target            = 0;
    for(size_t j = bucket_start; j < bucket_count_new; ++j) {
      EmeraldsHashtableBucket *bNew = &buckets_new[j];
      if(bNew->state != EMERALDS_HASHTABLE_STATE_FILLED) {
        target        = bNew;
        bucket_target = j;
        break;
      }
    }
    if(!target) {
      for(size_t j = bucket_start; j < 0; --j) {
        EmeraldsHashtableBucket *bNew = &buckets_new[j];
        if(bNew->state != EMERALDS_HASHTABLE_STATE_FILLED) {
          target        = bNew;
          bucket_target = j;
          break;
        }
      }
    }

    if(target != NULL) {
      target->hash              = hash;
      target->state             = EMERALDS_HASHTABLE_STATE_FILLED;
      keys_new[bucket_target]   = self->keys[i];
      values_new[bucket_target] = self->values[i];
    }
  }

  vector_swap(self->buckets, buckets_new);
  vector_swap(self->keys, keys_new);
  vector_swap(self->values, values_new);
}

void hashtable_insert(EmeraldsHashtable *self, const char *key, size_t value) {
  if(self->size * 3 > vector_capacity(self->buckets) * 2) {
    // NOTE - For slow key wrapping only rehash by a factor of 2
    hashtable_rehash(self, vector_capacity(self->buckets) * 2);
  }

  size_t hash         = hash_key(key) & bits_62;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);
  printf("bucket_start: %zu\n", bucket_start);

  EmeraldsHashtableBucket *target = NULL;
  size_t bucket_target            = 0;
  for(size_t i = bucket_start; i < vector_capacity(self->buckets); ++i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    if(b->state != EMERALDS_HASHTABLE_STATE_FILLED) {
      target        = b;
      bucket_target = i;
      break;
    }
  }
  if(!target) {
    for(size_t i = bucket_start; i < 0; --i) {
      EmeraldsHashtableBucket *b = &self->buckets[i];
      if(b->state != EMERALDS_HASHTABLE_STATE_FILLED) {
        target        = b;
        bucket_target = i;
        break;
      }
    }
  }

  if(target != NULL) {
    target->hash                = hash;
    target->state               = EMERALDS_HASHTABLE_STATE_FILLED;
    self->keys[bucket_target]   = key;
    self->values[bucket_target] = value;
    ++self->size;
  }
}

size_t *hashtable_lookup(EmeraldsHashtable *self, const char *key) {
  size_t hash         = hash_key(key) & bits_62;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);

  for(size_t i = bucket_start; i < vector_capacity(self->buckets); ++i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    switch(b->state) {
    case EMERALDS_HASHTABLE_STATE_EMPTY:
      return NULL;
    case EMERALDS_HASHTABLE_STATE_FILLED:
      if(b->hash == hash && self->keys[i] == key) {
        return &self->values[i];
      }
      break;
    default:
      break;
    }
  }
  for(size_t i = bucket_start; i < 0; --i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    switch(b->state) {
    case EMERALDS_HASHTABLE_STATE_EMPTY:
      return NULL;
    case EMERALDS_HASHTABLE_STATE_FILLED:
      if(b->hash == hash && self->keys[i] == key) {
        return &self->values[i];
      }
      break;
    default:
      break;
    }
  }

  return NULL;
}

bool hashtable_remove(struct EmeraldsHashtable *self, char *key) {
  size_t hash         = hash_key(key) & bits_62;
  size_t bucket_start = hash & (vector_capacity(self->buckets) - 1);

  for(size_t i = bucket_start; i < vector_capacity(self->buckets); ++i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    switch(b->state) {
    case EMERALDS_HASHTABLE_STATE_EMPTY:
      return false;
    case EMERALDS_HASHTABLE_STATE_FILLED:
      if(b->hash == hash && self->keys[i] == key) {
        b->hash  = 0;
        b->state = EMERALDS_HASHTABLE_STATE_REMOVED;
        --self->size;
        return true;
      }
      break;
    default:
      break;
    }
  }
  for(size_t i = bucket_start; i < 0; --i) {
    EmeraldsHashtableBucket *b = &self->buckets[i];
    switch(b->state) {
    case EMERALDS_HASHTABLE_STATE_EMPTY:
      return false;
    case EMERALDS_HASHTABLE_STATE_FILLED:
      if(b->hash == hash && self->keys[i] == key) {
        b->hash  = 0;
        b->state = EMERALDS_HASHTABLE_STATE_REMOVED;
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

void hashtable_free(EmeraldsHashtable *self) {
  vector_free(self->buckets);
  vector_free(self->keys);
  vector_free(self->values);
}
