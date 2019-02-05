#include "driver.h"
#include "staticset.h"

#include <iostream>
#include <random>

std::vector<int> generateRandomVector(size_t count) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(-(1 << 16), (1 << 16));

  std::vector<int> data;

  while (count--) {
    data.push_back(distribution(generator));
  }

  std::sort(data.begin(), data.end());
  data.resize(std::unique(data.begin(), data.end()) - data.begin());

  return data;
}

describe("search", []() {
  describe("lowerBound", []() {
    it("yields an iterator pointing at the smallest elem. GTE the query, or end() if no such element exists", []() {
      const size_t n_cases = 20;
      const size_t sizes[n_cases] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 100, 1000, 10000};

      for (size_t i = 0; i < n_cases; i++) {
        std::vector<int> data = generateRandomVector(sizes[i]);
        StaticSet<int> ss(data.begin(), data.end());

        int prev = INT_MIN;

        for (auto value : data) {
          std::default_random_engine generator;
          std::uniform_int_distribution<int> distribution(prev + 1, value);

          for (int k = 0; k < 10; k++) {
            const int query = distribution(generator);
            expect(*ss.lowerBound(query) == value);
          }
          expect(*ss.lowerBound(value) == value);

          prev = value;
        }
      }
    });
  });
});
