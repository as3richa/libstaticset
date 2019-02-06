#include "driver.h"
#include "staticset.h"

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

      for (size_t i = 0; i <= 256; i++) {
        sizes[i] = i;
      }
      sizes[257] = 1000;
      sizes[258] = 10000;
      sizes[259] = 100000;

      for (size_t i = 0; i < n_cases; i++) {
        const std::vector<int> data = generateRandomVector(sizes[i]);
        const StaticSet<int> ss(data.begin(), data.end());

        int prev = INT_MIN;

        for (const auto value : data) {
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
        for (int k = 0; k < 10; k++) {
          const int query = distribution(generator);
          expect(ss.lowerBound(query) == ss.end());
        }
      }
    });

    it("respects the given comparator", []() {
      const StaticSet<int, std::greater<int>> ss = {2, 4, 6, 8, 10};

      const std::vector<std::pair<int, int>> cases = {{99, 10}, {11, 10}, {10, 10}, {9, 8}, {8, 8}, {7, 6},
                                                      {6, 6},   {5, 4},   {4, 4},   {3, 2}, {2, 2}};

      for (const auto cse : cases) {
        expect(*ss.lowerBound(cse.first) == cse.second);
      }
      expect(ss.lowerBound(1) == ss.end());
    });
  });

  describe("upper bound", []() {
    it("yields an iterator pointing at the smallest elem. GT the query, or end() if no such elem. exists", []() {
      const size_t n_cases = 260;
      size_t sizes[n_cases];

      for (size_t i = 0; i <= 256; i++) {
        sizes[i] = i;
      }
      sizes[257] = 1000;
      sizes[258] = 10000;
      sizes[259] = 100000;

      for (size_t i = 0; i < n_cases; i++) {
        const std::vector<int> data = generateRandomVector(sizes[i]);
        const StaticSet<int> ss(data.begin(), data.end());

        int prev = INT_MIN;

        for (const auto value : data) {
          std::uniform_int_distribution<int> distribution(prev, value - 1);

          /* Try 10 random queries at least as large as the previous element, but strictly
           * less than the current */
          for (int k = 0; k < 10; k++) {
            const int query = distribution(generator);
            expect(*ss.upperBound(query) == value);
          }

          /* Try the previous element */
          expect(*ss.upperBound(prev) == value);

          prev = value;
        }

        /* Try the maximum element of the set */
        expect(ss.upperBound(prev) == ss.end());

        std::uniform_int_distribution<int> distribution(prev + 1, INT_MAX);

        /* Try 10 random queries strictly greater than the maximum element of the set */
        for (int k = 0; k < 10; k++) {
          const int query = distribution(generator);
          expect(ss.upperBound(query) == ss.end());
        }
      }
    });

    it("respects the given comparator", []() {
      const StaticSet<int, std::greater<int>> ss = {2, 4, 6, 8, 10};

      const std::vector<std::pair<int, int>> cases = {
          {99, 10}, {11, 10}, {10, 8}, {9, 8}, {8, 6}, {7, 6}, {6, 4}, {5, 4}, {4, 2}, {3, 2},
      };

      for (const auto cse : cases) {
        expect(*ss.upperBound(cse.first) == cse.second);
      }
      expect(ss.upperBound(2) == ss.end());
    });
  });

  describe("find", []() {
    it("returns an iterator pointing to an elem. equal to the query, or end() if no such elem. exists", []() {
      std::vector<int> data;

      for(int i = 0; i < 100000; i ++) {
        data.push_back(i * 10);
      }

      const StaticSet<int> ss(data.begin(), data.end());

      for(int i = 0; i < 10 * 100000; i ++) {
        if(i % 10 == 0) {
          expect(*ss.find(i) == i);
        } else {
          expect(ss.find(i) == ss.end());
        }
      }
    });

    it("defines equality in terms of the given comparator (and not operator==)", []() {
      std::vector<std::pair<int, int>> data;

      for(int i = 0; i < 100000; i ++) {
        data.push_back(std::make_pair(i, 0));
      }

      const auto compare = [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
        return (x.first < y.first);
      };

      const StaticSet<std::pair<int, int>, decltype(compare)> ss(data.begin(), data.end(), compare);

      for(int i = 0; i < 100000; i ++) {
        const auto expectation = std::make_pair(i, 0);

        for(int j = 0; j < 10; j ++) {
          const auto needle = std::make_pair(i, j);
          const auto result = *ss.find(needle);
          expect(result == expectation);
        }
      }
    });
  });

  describe("contains", []() {
    it("returns a boolean indicating the presence/absence of an element that compares equal to the query", []() {
      std::vector<int> data;

      for(int i = 0; i < 100000; i ++) {
        data.push_back(i * 10);
      }

      const StaticSet<int> ss(data.begin(), data.end());

      for(int i = 0; i < 10 * 100000; i ++) {
        expect(ss.contains(i) == (i % 10 == 0));
      }
    });

    it("defines equality in terms of the given comparator (and not operator==)", []() {
      std::vector<std::pair<int, int>> data;

      for(int i = 0; i < 100000; i ++) {
        data.push_back(std::make_pair(i, 0));
      }

      const auto compare = [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
        return (x.first < y.first);
      };

      const StaticSet<std::pair<int, int>, decltype(compare)> ss(data.begin(), data.end(), compare);

      for(int i = 0; i < 100000; i ++) {
        for(int j = 0; j < 10; j ++) {
          const auto needle = std::make_pair(i, j);
          expect(ss.contains(needle));
        }
      }
    });
  });
});
