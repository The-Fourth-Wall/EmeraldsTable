#include "../../libs/cSpec/export/cSpec.h"
#include "../../libs/EmeraldsReadHandler/export/EmeraldsReadHandler.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsString/export/EmeraldsString.h" /* IWYU pragma: keep */
#include "../../libs/EmeraldsVector/export/EmeraldsVector.h" /* IWYU pragma: keep */
#include "../../src/table/table.h"

module(T_table, {
  describe("#table", {
    it("", {
      EmeraldsHashtable *table =
        (EmeraldsHashtable *)malloc(sizeof(EmeraldsHashtable));
      hashtable_init(table);
      char key1[] = "key1";
      char key2[] = "key2";
      char key3[] = "key3";

      hashtable_insert(table, key1, 100);
      hashtable_insert(table, key2, 200);
      hashtable_insert(table, key3, 300);

      size_t *value1 = hashtable_lookup(table, key1);
      size_t *value2 = hashtable_lookup(table, key2);
      size_t *value3 = hashtable_lookup(table, key3);
      size_t *value4 = hashtable_lookup(table, (char *)"key4");

      if(value1) {
        printf("Found key1: %zu\n", *value1);
      }
      if(value2) {
        printf("Found key2: %zu\n", *value2);
      }
      if(value3) {
        printf("Found key3: %zu\n", *value3);
      }
      if(!value4) {
        printf("key4 not found\n");
      }

      hashtable_remove(table, (char *)key2);
      value2 = hashtable_lookup(table, (char *)key2);
      if(!value2) {
        printf("key2 successfully removed\n");
      }

      char *words = string_new(
        read_handler_load(read_handler_new(), "examples/random_words.txt")
      );
      char **arr = string_split(words, '\n');
      printf("array size: %zu\n", vector_size(arr));

      hashtable_free(table);

      // TODO - FIX I HAVE PROBLEM WITH STRINGS OR POINTERS OR STH LOSING MEMORY

      // for(size_t i = 0; i < vector_size(arr); ++i) {
      for(size_t i = 0; i < 10; ++i) {
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

      // assert_that_int(*v1 equals to 1);
      // assert_that_int(*v2 equals to 9168);
      // assert_that_int(*v3 equals to 28683);
      // assert_that_int(*v4 equals to 65065);
      // assert_that_int(*v5 equals to 94607);
      // assert_that_int(*v6 equals to 100000);
    });
  });
})
