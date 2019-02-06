#include "driver.h"
#include "staticset.h"

#include <random>

static std::default_random_engine generator;

static std::vector<int> generateRandomVector(size_t count) {
  std::uniform_int_distribution<int> distribution(-(1 << 16), (1 << 16));

  std::vector<int> data;

  while (count--) {
    data.push_back(distribution(generator));
  }

  std::sort(data.begin(), data.end());
  data.resize(std::unique(data.begin(), data.end()) - data.begin());

  return data;
}

describe("unordered iteration", []() {
  it("exposes iterators for the elements of the set, in no particular order, as ubegin/uend", []() {
    for(const size_t size: { 0, 1, 5, 100, 100000 }) {
      std::vector<int> data = generateRandomVector(size);
      const StaticSet<int> ss(data.begin(), data.end());

      std::vector<int> unordered(ss.ubegin(), ss.uend());
      std::sort(unordered.begin(), unordered.end());
      expect(data == unordered);
    }
  });
});
