#include "../../libs/cSpec/export/cSpec.h"
#include "../../libs/EmeraldsReadHandler/export/EmeraldsReadHandler.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsString/export/EmeraldsString.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */
#include "../../src/table/table.h"

#define TABLE_UNDEFINED 0xfffc000000000000

module(T_table, {
  it("handles simple inserts, lookups and removals", {
    EmeraldsHashtable *table = table_new();
    char *key1               = string_new("key1");
    char *key2               = string_new("key2");
    char *key3               = string_new("key3");

    table_add(table, key1, 100);
    table_add(table, key2, 200);
    table_add(table, key3, 300);

    size_t value1 = table_get(table, key1);
    size_t value2 = table_get(table, key2);
    size_t value3 = table_get(table, key3);
    size_t value4 = table_get(table, string_new("key4"));

    assert_that(value1 isnot TABLE_UNDEFINED);
    assert_that(value2 isnot TABLE_UNDEFINED);
    assert_that(value3 isnot TABLE_UNDEFINED);
    assert_that(value1 is 100);
    assert_that(value2 is 200);
    assert_that(value3 is 300);

    assert_that(value4 is TABLE_UNDEFINED);

    table_remove(table, key2);
    value2 = table_get(table, key2);
    assert_that(value2 is TABLE_UNDEFINED);

    table_free(table);
    assert_that(table is NULL);
  });

  it("reads a file with 100000 random words", {
    EmeraldsHashtable *table = table_new();

    char *words = string_new(
      read_handler_load(read_handler_new(), "examples/random_words.txt")
    );
    char **arr = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      table_add(table, string_new(arr[i]), i + 1);
    }

    size_t v1 = table_get(table, string_new("bfs6Zsw"));
    size_t v2 = table_get(table, string_new("ECrPiBm43eIJ0xN"));
    size_t v3 = table_get(table, string_new("EPYDHcSveb7sD"));
    size_t v4 = table_get(table, string_new("CRNiqP3OKSKA4"));
    size_t v5 = table_get(table, string_new("ummXz6BpkGfRRq"));
    size_t v6 = table_get(table, string_new("tP7hbqI"));

    assert_that_int(v1 equals to 1);
    assert_that_int(v2 equals to 9168);
    assert_that_int(v3 equals to 28683);
    assert_that_int(v4 equals to 65065);
    assert_that_int(v5 equals to 94607);
    assert_that_int(v6 equals to 100000);
  });

  it("reads a big file with 1000000 random words", {
    EmeraldsHashtable *table = table_new();

    char *words =
      string_new(read_handler_load(read_handler_new(), "examples/big_list.txt")
      );
    char **arr = string_split(words, '\n');

    for(size_t i = 0; i < vector_size(arr); i++) {
      table_add(table, string_new(arr[i]), i + 1);
    }

    size_t v1 = table_get(table, string_new("VJC3HJ1X"));
    size_t v2 = table_get(table, string_new("VSqxvKVdVVx9a"));
    size_t v3 = table_get(table, string_new("qjihpIWBF4D"));
    size_t v4 = table_get(table, string_new("rAwK4B12daB4fi1"));
    size_t v5 = table_get(table, string_new("qcyZ1aMb5tNAvp"));
    size_t v6 = table_get(table, string_new("wdLIfi3G7Aa4Oks"));

    assert_that_int(v1 equals to 222222);
    assert_that_int(v2 equals to 333333);
    assert_that_int(v3 equals to 555555);
    assert_that_int(v4 equals to 777777);
    assert_that_int(v5 equals to 888888);
    assert_that_int(v6 equals to 999999);
  });
})
