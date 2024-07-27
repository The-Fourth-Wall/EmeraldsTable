#include "../../libs/cSpec/export/cSpec.h"
#include "../../libs/EmeraldsReadHandler/export/EmeraldsReadHandler.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsString/export/EmeraldsString.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */
#include "../../src/table/table.h"

module(T_table, {
  it("handles simple inserts, lookups and removals", {
    EmeraldsHashtable *table = hashtable_new();
    char *key1               = string_new("key1");
    char *key2               = string_new("key2");
    char *key3               = string_new("key3");

    hashtable_insert(table, key1, 100);
    hashtable_insert(table, key2, 200);
    hashtable_insert(table, key3, 300);

    size_t *value1 = hashtable_lookup(table, key1);
    size_t *value2 = hashtable_lookup(table, key2);
    size_t *value3 = hashtable_lookup(table, key3);
    size_t *value4 = hashtable_lookup(table, string_new("key4"));

    assert_that(value1 isnot NULL);
    assert_that(*value1 is 100);
    assert_that(value2 isnot NULL);
    assert_that(*value2 is 200);
    assert_that(value3 isnot NULL);
    assert_that(*value3 is 300);
    assert_that(value4 is NULL);

    hashtable_remove(table, key2);
    value2 = hashtable_lookup(table, key2);
    assert_that(value2 is NULL);
  });

  it("reads a file with 100000 random words", {
    EmeraldsHashtable *table = hashtable_new();

    char *words = string_new(
      read_handler_load(read_handler_new(), "examples/random_words.txt")
    );
    char **arr = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      hashtable_insert(table, string_new(arr[i]), i + 1);
    }

    size_t *v1 = hashtable_lookup(table, string_new("bfs6Zsw"));
    size_t *v2 = hashtable_lookup(table, string_new("ECrPiBm43eIJ0xN"));
    size_t *v3 = hashtable_lookup(table, string_new("EPYDHcSveb7sD"));
    size_t *v4 = hashtable_lookup(table, string_new("CRNiqP3OKSKA4"));
    size_t *v5 = hashtable_lookup(table, string_new("ummXz6BpkGfRRq"));
    size_t *v6 = hashtable_lookup(table, string_new("tP7hbqI"));

    assert_that(v1 isnot NULL);
    assert_that(v2 isnot NULL);
    assert_that(v3 isnot NULL);
    assert_that(v4 isnot NULL);
    assert_that(v5 isnot NULL);
    assert_that(v6 isnot NULL);

    assert_that_int(*v1 equals to 1);
    assert_that_int(*v2 equals to 9168);
    assert_that_int(*v3 equals to 28683);
    assert_that_int(*v4 equals to 65065);
    assert_that_int(*v5 equals to 94607);
    assert_that_int(*v6 equals to 100000);
  });

  it("reads a big file with 1000000 random words", {
    EmeraldsHashtable *table = hashtable_new();

    char *words =
      string_new(read_handler_load(read_handler_new(), "examples/big_list.txt")
      );
    char **arr = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      hashtable_insert(table, string_new(arr[i]), i + 1);
    }

    size_t *v1 = hashtable_lookup(table, string_new("VJC3HJ1X"));
    size_t *v2 = hashtable_lookup(table, string_new("VSqxvKVdVVx9a"));
    size_t *v3 = hashtable_lookup(table, string_new("qjihpIWBF4D"));
    size_t *v4 = hashtable_lookup(table, string_new("rAwK4B12daB4fi1"));
    size_t *v5 = hashtable_lookup(table, string_new("qcyZ1aMb5tNAvp"));
    size_t *v6 = hashtable_lookup(table, string_new("wdLIfi3G7Aa4Oks"));

    assert_that(v1 isnot NULL);
    assert_that(v2 isnot NULL);
    assert_that(v3 isnot NULL);
    assert_that(v4 isnot NULL);
    assert_that(v5 isnot NULL);
    assert_that(v6 isnot NULL);

    assert_that_int(*v1 equals to 222222);
    assert_that_int(*v2 equals to 333333);
    assert_that_int(*v3 equals to 555555);
    assert_that_int(*v4 equals to 777777);
    assert_that_int(*v5 equals to 888888);
    assert_that_int(*v6 equals to 999999);
  });
})
