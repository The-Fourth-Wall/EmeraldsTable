#include "../../libs/cSpec/export/cSpec.h"
#include "../../src/get_value/get_value.h"

module(T_get_value, {
  describe("#get_value", {
    it("returns `Hello, World!`", {
      assert_that_charptr(get_value() equals to "Hello, World!");
    });
  });
})
