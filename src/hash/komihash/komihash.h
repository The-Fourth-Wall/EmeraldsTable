#ifndef __KOMIHASH_H_
#define __KOMIHASH_H_

#include "komihash_implementation.h"

/**
 * @brief Wrapper for the komihash hash function
 * @param key
 * @param size
 * @return size_t hash
 */
#define komihash_hash(key, size) (komihash((key), size, 0x0123456789abcdef))

#endif
