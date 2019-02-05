#include "driver.h"
#include "staticset.h"

#include <list>
#include <set>
#include <vector>

describe("basic interface", []() {
  it("can be declared with a single template argument indicating the contained type", []() {
    const StaticSet<int> foo;
    (void)foo;

    const StaticSet<long long> bar;
    (void)bar;

    const StaticSet<std::vector<int>> baz;
    (void)baz;

    struct SomeStruct {
      int a, b;
    };
    const StaticSet<SomeStruct> bif;
    (void)bif;
  });

  describe("constructors", []() {
    it("implements a constructor taking an initializer list", []() {
      const auto init_list = {2, 3, 5, 7, 11};
      const StaticSet<int> foo = init_list;

      for (const auto value : init_list) {
        expect(foo.contains(value));
      }
    });

    it("implements a construction taking a pair of begin/end iterators", []() {
      const std::list<long> list = {9LL, 2LL, 0LL};
      const StaticSet<long> foo(list.begin(), list.end());

      for (const auto value : list) {
        expect(foo.contains(value));
      }

      const std::vector<char> vector = {'h', 'e', 'y', 'm', 'a', 'n'};
      const StaticSet<char> bar(vector.begin(), vector.end());

      for (const auto value : vector) {
        expect(bar.contains(value));
      }

      const std::set<std::pair<int, int>> set = {{1, 1}, {3, 3}, {5, 5}};
      const StaticSet<std::pair<int, int>> baz(set.begin(), set.end());

      for (const auto value : set) {
        expect(baz.contains(value));
      }

      const char *pointers[2] = {"adam", "ignatius"};
      const StaticSet<const char *> bif(pointers, pointers + 2);
      expect(bif.contains(pointers[0]));
      expect(bif.contains(pointers[1]));
    });
  });
});
