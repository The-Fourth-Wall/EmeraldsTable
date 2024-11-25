#include "table.h"

/**
 * @brief Generic bucket finder
 * @param hashes -> The hashes array
 * @param states -> The states array
 * @param hash -> The hash of the key
 * @param keys -> The keys vector
 * @param key -> The key to find
 * @param keylen -> The length of the key
 * @param find_empty -> A flag for when we are adding new keys
 * @return size_t -> The index of the bucket or TABLE_UNDEFINED if not found
 */
p_inline size_t _table_find_bucket(
  size_t *hashes,
  uint8_t *states,
  size_t hash,
  const char **keys,
  const char *key,
  size_t keylen,
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
              strncmp(keys[bucket_index], key, keylen) == 0) {
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
p_inline void _table_rehash(EmeraldsTable *self) {
  size_t i;
  size_t *hashes_new    = NULL;
  uint8_t *states_new   = NULL;
  const char **keys_new = NULL;
  size_t *values_new    = NULL;
  size_t capacity       = vector_capacity(self->keys);
  size_t capacity_new   = vector_capacity(self->keys) * TABLE_GROW_FACTOR;
  if(capacity_new < TABLE_INITIAL_SIZE) {
    capacity_new = TABLE_INITIAL_SIZE;
  }
  vector_initialize_n(hashes_new, capacity_new);
  vector_initialize_n(states_new, capacity_new);
  vector_initialize_n(keys_new, capacity_new);
  vector_initialize_n(values_new, capacity_new);
  for(i = 0; i < capacity; i++) {
    if(self->states[i] == TABLE_STATE_FILLED) {
      size_t hash         = self->hashes[i];
      size_t bucket_index = hash & (capacity_new - 1);
      while(states_new[bucket_index] == TABLE_STATE_FILLED) {
        bucket_index = (bucket_index + 1) & (capacity_new - 1);
      }
      hashes_new[bucket_index] = hash;
      states_new[bucket_index] = TABLE_STATE_FILLED;
      keys_new[bucket_index]   = self->keys[i];
      values_new[bucket_index] = self->values[i];
    }
  }
  vector_free(self->keys);
  vector_free(self->hashes);
  vector_free(self->values);
  vector_free(self->states);
  self->keys       = keys_new;
  self->hashes     = hashes_new;
  self->values     = values_new;
  self->states     = states_new;
  self->tombstones = 0;
}

void table_init(EmeraldsTable *self) {
  vector_initialize_n(self->keys, TABLE_INITIAL_SIZE);
  vector_initialize_n(self->values, TABLE_INITIAL_SIZE);
  vector_initialize_n(self->hashes, TABLE_INITIAL_SIZE);
  vector_initialize_n(self->states, TABLE_INITIAL_SIZE);
  self->size       = 0;
  self->tombstones = 0;
}

void table_add(EmeraldsTable *self, const char *key, size_t value) {
  size_t hash;
  size_t bucket_index;
  size_t prev_state;
  size_t keylen;
  if(self->size + (self->tombstones) >
     vector_capacity(self->keys) * TABLE_LOAD_FACTOR) {
    _table_rehash(self);
  }
  keylen       = strlen(key);
  hash         = TABLE_HASH_FUNCTION(key, keylen);
  bucket_index = _table_find_bucket(
    self->hashes, self->states, hash, self->keys, key, keylen, true
  );
  prev_state = self->states[bucket_index];
  if(bucket_index != TABLE_UNDEFINED) {
    self->hashes[bucket_index] = hash;
    self->keys[bucket_index]   = key;
    self->values[bucket_index] = value;
    self->states[bucket_index] = TABLE_STATE_FILLED;
    if(prev_state != TABLE_STATE_FILLED) {
      self->size++;
      if(prev_state == TABLE_STATE_DELETED) {
        self->tombstones--;
      }
    }
  }
}

void table_add_all(EmeraldsTable *src, EmeraldsTable *dst) {
  size_t i;
  for(i = 0; i < vector_capacity(src->keys); i++) {
    if(src->states[i] == TABLE_STATE_FILLED) {
      table_add(dst, src->keys[i], src->values[i]);
    }
  }
}

void table_add_all_non_labels(EmeraldsTable *src, EmeraldsTable *dst) {
  size_t i;
  for(i = 0; i < vector_capacity(src->keys); i++) {
    if(src->states[i] == TABLE_STATE_FILLED) {
      if(src->keys[i] && !(src->keys[i][0] == '@' && src->keys[i][1] == ':' &&
                           src->keys[i][2] == ':')) {
        table_add(dst, src->keys[i], src->values[i]);
      }
    }
  }
}

size_t table_get(EmeraldsTable *self, const char *key) {
  size_t keylen       = strlen(key);
  size_t hash         = TABLE_HASH_FUNCTION(key, keylen);
  size_t bucket_index = _table_find_bucket(
    self->hashes, self->states, hash, self->keys, key, keylen, false
  );

  if(bucket_index != TABLE_UNDEFINED) {
    return self->values[bucket_index];
  } else {
    return TABLE_UNDEFINED;
  }
}

void table_remove(EmeraldsTable *self, const char *key) {
  size_t keylen       = strlen(key);
  size_t hash         = TABLE_HASH_FUNCTION(key, keylen);
  size_t bucket_index = _table_find_bucket(
    self->hashes, self->states, hash, self->keys, key, keylen, false
  );
  if(bucket_index != TABLE_UNDEFINED) {
    self->states[bucket_index] = TABLE_STATE_DELETED;
    self->size--;
    self->tombstones++;
  }
}

size_t table_size(EmeraldsTable *self) { return self->size; }

void table_deinit(EmeraldsTable *self) {
  vector_free(self->hashes);
  vector_free(self->states);
  vector_free(self->keys);
  vector_free(self->values);
}
