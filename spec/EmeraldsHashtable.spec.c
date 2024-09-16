#include "../libs/cSpec/export/cSpec.h"
#include "hash/komihash/komihash.module.spec.h"
#include "hash/xxh3/xxh3.module.spec.h"
#include "table/benchmarks/table_general_benchmark.spec.h"
#include "table/table.module.spec.h"

int main(void) {
  cspec_run_suite("all", {
    T_komihash();
    T_xxh3();
    T_table_general_benchmark();
    T_table();
  });
}
