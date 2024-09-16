#include "../../../libs/cSpec/export/cSpec.h"
#include "../../../src/hash/komihash/komihash.h"

module(T_komihash, {
  describe("#komihash", {
    it("hashes small strings", {
      assert_that_size_t(komihash_hash("a", 1) equals to 0xa9cb00f1f8e133e3);
      assert_that_size_t(komihash_hash("ab", 2) equals to 0xfb6b11b0cbe38e68);
      assert_that_size_t(komihash_hash("abc", 3) equals to 0xd4106f3ebb74a844);
      assert_that_size_t(komihash_hash("abcd", 4) equals to 0xf9032d5e074193d4);
      assert_that_size_t(komihash_hash("abcde", 5) equals to 0x1c1390c791aba2be
      );
      assert_that_size_t(komihash_hash("abcdef", 6) equals to 0x7674f480a7a03d16
      );
      assert_that_size_t(komihash_hash("abcdefg", 7)
                           equals to 0x5ddabccf460093b1);
      assert_that_size_t(komihash_hash("abcdefgh", 8)
                           equals to 0x5169681af4fab7dc);
    });

    it("hashes medium sized strings", {
      assert_that_size_t(komihash_hash("abcdefghi", 9)
                           equals to 0x379bc6b1199c8011);
      assert_that_size_t(komihash_hash("abcdefghij", 10)
                           equals to 0xde3cb6b9fe957747);
      assert_that_size_t(komihash_hash("abcdefghijk", 11)
                           equals to 0x8da521c32c02eee1);
      assert_that_size_t(komihash_hash("abcdefghijkl", 12)
                           equals to 0x866301bc2bf7e6e4);
      assert_that_size_t(komihash_hash("abcdefghijklm", 13)
                           equals to 0xffc2e6e691c40eb2);
      assert_that_size_t(komihash_hash("abcdefghijklmn", 14)
                           equals to 0x540d0093e0bcb8e3);
      assert_that_size_t(komihash_hash("abcdefghijklmno", 15)
                           equals to 0xe1662727cd666237);
      assert_that_size_t(komihash_hash("abcdefghijklmnop", 16)
                           equals to 0x8fe1f62b64188956);
    });

    it("hashes long strings", {
      assert_that_size_t(komihash_hash("abcdefghijklmnopqrst", 20)
                           equals to 0x7b47a04740a44326);
      assert_that_size_t(komihash_hash(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim "
        "ad minim veniam.",
        148
      ) equals to 0xa503670a6d10c0ba);
    });
  });
})
