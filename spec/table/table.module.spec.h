#include "../../libs/cSpec/export/cSpec.h"
#include "../../libs/EmeraldsFileHandler/export/EmeraldsFileHandler.h"
#include "../../libs/EmeraldsString/export/EmeraldsString.h"
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h"
#include "../../src/table/table.h"

module(T_table, {
  it("inserts the empty string into the hash table", {
    EmeraldsHashtable table = {0};
    table_init(&table);
    table_add(&table, "", 42);
    size_t value = table_get(&table, "");
    assert_that_size_t(value equals to 42);
  });

  it("initializes the hash table", {
    EmeraldsHashtable table = {0};
    table_init(&table);

    assert_that_size_t((&table)->size equals to 0);
    assert_that_size_t(vector_capacity((&table)->buckets) equals to 1024);
    assert_that_size_t(vector_capacity((&table)->keys) equals to 1024);
    assert_that_size_t(vector_capacity((&table)->values) equals to 1024);

    table_add(&table, "key1", 100);
    assert_that_size_t((&table)->size equals to 1);

    table_deinit(&table);

    assert_that(table.buckets is NULL);
    assert_that(table.keys is NULL);
    assert_that(table.values is NULL);
  });

  it("handles simple inserts, lookups and removals", {
    EmeraldsHashtable table = {0};
    table_init(&table);

    table_add(&table, "key1", 100);
    table_add(&table, "key2", 200);
    table_add(&table, "key3", 300);

    size_t value1 = table_get(&table, "key1");
    size_t value2 = table_get(&table, "key2");
    size_t value3 = table_get(&table, "key3");
    size_t value4 = table_get(&table, "key4");

    assert_that(value1 isnot TABLE_UNDEFINED);
    assert_that(value2 isnot TABLE_UNDEFINED);
    assert_that(value3 isnot TABLE_UNDEFINED);
    assert_that(value1 is 100);
    assert_that(value2 is 200);
    assert_that(value3 is 300);

    assert_that(value4 is TABLE_UNDEFINED);

    table_remove(&table, "key2");
    value2 = table_get(&table, "key2");
    assert_that(value2 is TABLE_UNDEFINED);

    table_deinit(&table);
    assert_that(table.buckets is NULL);
    assert_that(table.keys is NULL);
    assert_that(table.values is NULL);
  });

  it("adds all the keys and values of one hash table to another", {
    EmeraldsHashtable table1 = {0};
    table_init(&table1);
    table_add(&table1, "key1", 100);
    table_add(&table1, "key2", 200);
    table_add(&table1, "key3", 300);

    EmeraldsHashtable table2 = {0};
    table_init(&table2);
    table_add(&table2, "key14", 42);
    table_add_all(&table1, &table2);

    assert_that_size_t(table2.size equals to 4);
    assert_that_size_t(table_get(&table2, "key1") equals to 100);
    assert_that_size_t(table_get(&table2, "key2") equals to 200);
    assert_that_size_t(table_get(&table2, "key3") equals to 300);
    assert_that_size_t(table_get(&table2, "key14") equals to 42);
  });

  it("reads a file with 100000 random words", {
    EmeraldsHashtable table = {0};
    table_init(&table);

    char *words = string_new(file_handler_read("examples/random_words.txt"));
    char **arr  = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      table_add(&table, arr[i], i + 1);
    }

    size_t v1 = table_get(&table, "bfs6Zsw");
    size_t v2 = table_get(&table, "ECrPiBm43eIJ0xN");
    size_t v3 = table_get(&table, "EPYDHcSveb7sD");
    size_t v4 = table_get(&table, "CRNiqP3OKSKA4");
    size_t v5 = table_get(&table, "ummXz6BpkGfRRq");
    size_t v6 = table_get(&table, "tP7hbqI");

    assert_that_int(v1 equals to 1);
    assert_that_int(v2 equals to 9168);
    assert_that_int(v3 equals to 28683);
    assert_that_int(v4 equals to 65065);
    assert_that_int(v5 equals to 94607);
    assert_that_int(v6 equals to 100000);
  });

  it("reads a big file with 1000000 random words", {
    EmeraldsHashtable table = {0};
    table_init(&table);

    char *words = string_new(file_handler_read("examples/big_list.txt"));
    char **arr  = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      table_add(&table, arr[i], i + 1);
    }

    size_t v1 = table_get(&table, "VJC3HJ1X");
    size_t v2 = table_get(&table, "VSqxvKVdVVx9a");
    size_t v3 = table_get(&table, "qjihpIWBF4D");
    size_t v4 = table_get(&table, "rAwK4B12daB4fi1");
    size_t v5 = table_get(&table, "qcyZ1aMb5tNAvp");
    size_t v6 = table_get(&table, "wdLIfi3G7Aa4Oks");

    assert_that_int(v1 equals to 222222);
    assert_that_int(v2 equals to 333333);
    assert_that_int(v3 equals to 555555);
    assert_that_int(v4 equals to 777777);
    assert_that_int(v5 equals to 888888);
    assert_that_int(v6 equals to 999999);
  });
})
