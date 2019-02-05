#include "driver.h"
#include "staticset.h"

#include <iostream>
#include <random>

std::default_random_engine generator;

std::vector<int> generateRandomVector(size_t count) {
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
    it("yields an iterator pointing at the smallest elem. GTE the query, or end() if no such elem. exists", []() {
      const size_t n_cases = 260;
      size_t sizes[n_cases];

      for(size_t i = 0; i <= 256; i ++) {
        sizes[i] = i;
      }
      sizes[257] = 1000;
      sizes[258] = 10000;
      sizes[259] = 100000;

      for (size_t i = 0; i < n_cases; i++) {
        std::vector<int> data = generateRandomVector(sizes[i]);
        StaticSet<int> ss(data.begin(), data.end());

        int prev = INT_MIN;

        for (auto value : data) {
          std::uniform_int_distribution<int> distribution(prev + 1, value);

          /* Try 10 random queries strictly greater than the next smallest value */
          for (int k = 0; k < 10; k++) {
            const int query = distribution(generator);
            expect(*ss.lowerBound(query) == value);
          }

          /* Try the value itself */
          expect(*ss.lowerBound(value) == value);

          prev = value;
        }

        std::uniform_int_distribution<int> distribution(prev + 1, INT_MAX);

        /* Try 10 random queries strictly greater than the maximum element of the set */
        for(int k = 0; k < 10; k ++) {
          const int query = distribution(generator);
          expect(ss.lowerBound(query) == ss.end());
        }
      }
    });
  });
});