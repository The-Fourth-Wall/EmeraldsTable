#include "table.h"

size_t table_get(EmeraldsHashtable *self, const char *key) {
  size_t hash = TABLE_HASH_FUNCTION(key) & TABLE_BITS_63_MASK;
  size_t *bucket =
    _table_find_bucket(self->buckets, hash, self->keys, key, false);

  if(bucket != NULL && TABLE_BUCKET_IS_FILLED(bucket)) {
    size_t bucket_index = bucket - self->buckets;
    return self->values[bucket_index];
  } else {
    return TABLE_UNDEFINED;
  }
}
