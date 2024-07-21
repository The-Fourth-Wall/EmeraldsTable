#include "../../libs/cSpec/export/cSpec.h"
#include "../../src/table/table.h"

module(T_table, {
  describe("#table", {
    it("returns `Hello, World!`", {
      assert_that_charptr(table() equals to "Hello, World!");
    });
  });
})
