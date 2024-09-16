#include "../src/EmeraldsHashtable.h"

int main(void) {
  EmeraldsHashtable table = {0};
  table_init(&table);
  table_add(&table, "key1", 100);
  table_add(&table, "key2", 200);
  table_add(&table, "key3", 300);

  (void)table_get(&table, "key1");
  (void)table_get(&table, "key2");
  (void)table_get(&table, "key3");
  (void)table_get(&table, "key4");

  table_remove(&table, "key2");
  table_get(&table, "key2");

  table_deinit(&table);
}
