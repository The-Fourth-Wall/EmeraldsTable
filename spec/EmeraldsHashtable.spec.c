#include "../libs/cSpec/export/cSpec.h"
#include "table/benchmarks/table_general_benchmark.spec.h"
#include "table/table.module.spec.h"

int main(void) {
  cspec_run_suite("all", {
    T_table_general_benchmark();
    T_table();
  });
}
