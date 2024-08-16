#include "../libs/cSpec/export/cSpec.h"
#include "table/table.module.spec.h"

int main(void) {
  cspec_run_suite("all", { T_table(); });
}
