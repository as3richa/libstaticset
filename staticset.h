#ifndef LIBSTATICSET_STATICSET_H
#define LIBSTATICSET_STATICSET_H

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

template <class T, class Compare = std::less<T>, class Allocator = std::allocator<T>> class StaticSet {
public:
  typedef std::vector<T> Vector;
  typedef typename Vector::iterator VectorIterator;

  struct util {
    static size_t goUp(size_t index) { return (index - 1) / 2; }
    static size_t goLeft(size_t index) { return 2 * index + 1; }
    static size_t goRight(size_t index) { return 2 * index + 2; }
    static bool isLeft(size_t index) { return (index % 2 == 1); }
    static bool isRight(size_t index) { return !isLeft(index); }

    static size_t digLeft(size_t index, size_t n) {
      for (;;) {
        size_t next = goLeft(index);
        if (next >= n) {
          return index;
        }
        index = next;
      }
    }

    static size_t digRight(size_t index, size_t n) {
      for (;;) {
        size_t next = goRight(index);
        if (next >= n) {
          return index;
        }
        index = next;
      }
    }
  };

  Compare compare;
  Vector tree;
  size_t leftmost;
  size_t rightmost;

  void buildTree(size_t index, const VectorIterator slice_begin, const VectorIterator slice_end) {
    const size_t count = slice_end - slice_begin;

    if (count == 0) {
      return;
    }

    const auto slice_center = slice_begin + count / 2;
    tree[index] = *slice_center;

    buildTree(util::goLeft(index), slice_begin, slice_center);
    buildTree(util::goRight(index), slice_center + 1, slice_end);
  }

  void Initialize(Vector scratch) {
    std::sort(scratch.begin(), scratch.end(), compare);

    const auto areEqual = [this](const T &x, const T &y) { return !(compare(x, y) || compare(y, x)); };
    const auto end = std::unique(scratch.begin(), scratch.end(), areEqual);
    scratch.resize(end - scratch.begin());

    tree.resize(scratch.size());
    buildTree(0, scratch.begin(), scratch.end());

    leftmost = util::digLeft(0, tree.size());
    rightmost = util::digRight(0, tree.size());
  }

public:
  typedef T value_type;
  typedef Allocator allocator_type;
  typedef typename Vector::size_type size_type;

  class OrderedIterator {
  public:
    friend class StaticSet;

    const StaticSet<T, Compare, Allocator> &ss;
    size_t index;

    OrderedIterator(const StaticSet<T, Compare, Allocator> &ss, size_t index) : ss(ss), index(index) { ; }

  public:
    typedef size_t difference_type;
    typedef T value_type;
    typedef const T *pointer;
    typedef const T &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    reference operator*() const { return ss.tree[index]; }

    pointer operator->() const { return &ss.tree[index]; }

    bool operator==(const OrderedIterator &other) const { return (&ss == &other.ss && index == other.index); }

    bool operator!=(const OrderedIterator &other) const { return !(*this == other); }

    OrderedIterator &operator++() {
      assert(index < ss.size());

      /* Three cases: (i) We're at the last element of the ordered sequence; indicate this by
       * setting index = ss.size() + 1; (ii) We're at a node with a right subtree; the next element
       * of the ordered sequence is the leftmost descendant of the right subtree; (iii) We're at a
       * node with no right subtree, but (because we're not at the end of the ordered sequence) we
       * must be in the left subtree of some ancestor; the lowest such ancestor is the next node in
       * the ordered sequence */

      if (index == ss.rightmost) {
        /* Case (i) */
        index = ss.size() + 1;
      } else {
        const size_t right = util::goRight(index);

        if (right < ss.size()) {
          /* Case (ii) */
          index = util::digLeft(right, ss.size());
        } else {
          /* Case (iii) */
          assert(index > 0);

          while (!util::isLeft(index)) {
            index = util::goUp(index);
            assert(index > 0);
          }

          index = util::goUp(index);
        }
      }

      return *this;
    }

    OrderedIterator operator++(int) {
      OrderedIterator prev = *this;
      ++(*this);
      return prev;
    }

    OrderedIterator &operator--() {
      assert(index != ss.leftmost && index <= 1 + ss.size());

      const size_t left = util::goLeft(index);

      if (left < ss.size()) {
        index = util::digRight(left, ss.size());
      } else {
        assert(index > 0);

        while (!util::isRight(index)) {
          index = util::goUp(index);
          assert(index > 0);
        }

        index = util::goUp(index);
      }

      return *this;
    }

    OrderedIterator operator--(int) {
      OrderedIterator prev = *this;
      --(*this);
      return prev;
    }
  };

  StaticSet() { ; }

  explicit StaticSet(const Compare &comp, const Allocator &alloc = Allocator()) : compare(comp), tree(alloc) { ; }

  explicit StaticSet(const Allocator &alloc) : tree(alloc) { ; }

  template <class Iter>
  StaticSet(Iter first, Iter last, const Compare &comp = Compare(), const Allocator &alloc = Allocator())
      : compare(comp), tree(alloc) {
    Initialize(Vector(first, last));
  }

  StaticSet(std::initializer_list<T> list, const Compare &comp = Compare(), const Allocator &alloc = Allocator())
      : compare(comp), tree(alloc) {
    Initialize(Vector(list, alloc));
  }

  StaticSet(const StaticSet &other) : compare(other.compare), tree(other.tree) { ; }

  StaticSet(const StaticSet &other, const Allocator &alloc) : compare(other.compare), tree(other.tree, alloc) { ; }

  StaticSet(StaticSet &&other) : compare(std::move(other.compare)), tree(std::move(other.tree)) { ; }

  StaticSet(StaticSet &&other, const Allocator &alloc)
      : compare(std::move(other.compare)), tree(std::move(other.tree), alloc) {
    ;
  }

  size_t size() const { return tree.size(); }

  bool empty() const { return tree.empty(); }

  OrderedIterator begin() const { return OrderedIterator(*this, leftmost); }

  OrderedIterator end() const { return OrderedIterator(*this, size() + 1); }
};

#endif