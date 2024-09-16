#ifndef __XXH3_H_
#define __XXH3_H_

/**
 * @brief Wrapper for the xxh3 hash function
 * @param key
 * @return size_t hash
 */
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxh3_implementation.h"

#define xxh3_hash(key, size) (XXH3_64bits((key), (size)))

#endif
