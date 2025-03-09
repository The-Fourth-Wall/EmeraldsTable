#ifndef __TABLE_GENERAL_BENCHMARK_SPEC_H_
#define __TABLE_GENERAL_BENCHMARK_SPEC_H_

#include "../../../libs/cSpec/export/cSpec.h"
#include "../../../src/EmeraldsTable.h"

#if defined(_WIN32)
  #include <windows.h>
static double get_time() {
  LARGE_INTEGER freq, count;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&count);
  return (double)count.QuadPart / freq.QuadPart;
}
#else
  #include <sys/time.h>
static double get_time() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec * 1e-6;
}
#endif

static char *generate_random_string(size_t length) {
  static const char charset[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  char *random_string = (char *)malloc(length + 1);
  if(random_string) {
    for(size_t n = 0; n < length; n++) {
      int key          = rand() % (int)(sizeof(charset) - 1);
      random_string[n] = charset[key];
    }
    random_string[length] = '\0';
  }
  return random_string;
}

static void
benchmark_insertion(EmeraldsTable *table, char **keys, size_t count) {
  double start_time = get_time();
  for(size_t i = 0; i < count; i++) {
    table_add(table, keys[i], i);
  }
  double end_time = get_time();
  printf(
    "Insertion of %zu items took %f seconds.\n", count, end_time - start_time
  );
}

static void benchmark_lookup(EmeraldsTable *table, char **keys, size_t count) {
  double start_time = get_time();
  size_t not_found  = 0;
  for(size_t i = 0; i < count; i++) {
    size_t value = table_get(table, keys[i]);
    if(value == TABLE_UNDEFINED) {
      not_found++;
    }
  }
  double end_time = get_time();
  printf(
    "Lookup of %zu items took %f seconds (%zu not found).\n",
    count,
    end_time - start_time,
    not_found
  );
}

static void
benchmark_deletion(EmeraldsTable *table, char **keys, size_t count) {
  double start_time = get_time();
  for(size_t i = 0; i < count; i++) {
    table_remove(table, keys[i]);
  }
  double end_time = get_time();
  printf(
    "Deletion of %zu items took %f seconds.\n", count, end_time - start_time
  );
}

static void benchmark_remove_nonexistent(EmeraldsTable *table, size_t count) {
  double start_time = get_time();
  for(size_t i = 0; i < count; i++) {
    char key[50];
    snprintf(key, sizeof(key), "nonexistent_%zu", i);
    table_remove(table, key);
  }
  double end_time = get_time();
  printf(
    "Attempted removal of %zu non-existent items took %f seconds.\n",
    count,
    end_time - start_time
  );
}

static void
benchmark_insert_duplicates(EmeraldsTable *table, char **keys, size_t count) {
  for(size_t i = 0; i < count; i++) {
    table_add(table, keys[i], i);
  }

  /* Same keys, */
  double start_time = get_time();
  for(size_t i = 0; i < count; i++) {
    table_add(table, keys[i], i + 1000); /* different values */
  }
  double end_time = get_time();
  printf(
    "Insertion of %zu duplicate items took %f seconds.\n",
    count,
    end_time - start_time
  );
}

#define ITEM_COUNT 10000000
#define ITEM_SIZE  10

module(T_table_general_benchmark, {
  it("benchmarks the table module", {
    srand((unsigned int)time(NULL));

    EmeraldsTable table = {0};
    table_init(&table);

    char **keys = malloc(sizeof(char *) * ITEM_COUNT);
    for(size_t i = 0; i < ITEM_COUNT; i++) {
      keys[i] = generate_random_string(ITEM_SIZE);
    }

    printf("RUNNING BENCHMARKS\n");

    benchmark_insertion(&table, keys, ITEM_COUNT);
    benchmark_lookup(&table, keys, ITEM_COUNT);
    benchmark_deletion(&table, keys, ITEM_COUNT);
    benchmark_remove_nonexistent(&table, ITEM_COUNT);
    benchmark_insert_duplicates(&table, keys, ITEM_COUNT);
  });
})

#endif
