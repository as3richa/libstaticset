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

  static const size_t size_t_bits = sizeof(size_t) * CHAR_BIT;

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

  Compare compare;
  Vector tree;
  size_t leftmost;
  size_t rightmost;

  void buildTree(size_t index, const VectorIterator slice_begin, const VectorIterator slice_end) {
    const size_t count = slice_end - slice_begin;

    assert((count > 0) == (index < tree.size()));

    if (count == 0) {
      return;
    }

    if (count == 1) {
      tree[index] = *slice_begin;
      return;
    }

    /* Consider the shortest possible binary tree with n nodes, and with the property that all
     * levels except possibly the bottommost are complete, and the bottommost level is filled
     * from left to right. As it happens, this configuration yields the most compact representation
     * of our search tree as an array, because our node indices would range from 0 to n - 1 with
     * no gaps */

    /* The height of the shortest possible binary tree on count nodes */
    size_t height;
    for (height = 2; (1 << height) <= count; height++) {
      ;
    }

    /* The number of nodes required to yield complete left and right subtrees excluding the
     * bottommost row */
    const size_t subtree_size_excluding_bottom = (1 << (height - 2)) - 1;

    /* The number of nodes that could fit into the bottommost row of a tree of the given height*/
    const size_t bottom_row_size = (1 << (height - 1));

    /* The number of nodes that we will actually be packing into the bottommost row */
    const size_t bottom_row_count = count - 1 - 2 * subtree_size_excluding_bottom;

    /* If we can pack the entire bottom row into the left subtree (in which case the right subtree
     * is one unit shorter, but complete), put sufficiently many nodes into the left subtree to do
     * so. Otherwise, put sufficiently many nodes into the left subtree to make it complete,
     * leaving the rest for the right subtree */
    VectorIterator pivot;
    if (bottom_row_count <= bottom_row_size / 2) {
      pivot = slice_begin + subtree_size_excluding_bottom + bottom_row_count;
    } else {
      pivot = slice_begin + subtree_size_excluding_bottom + bottom_row_size / 2;
    }

    tree[index] = *pivot;

    buildTree(goLeft(index), slice_begin, pivot);
    buildTree(goRight(index), pivot + 1, slice_end);
  }

  void initialize(Vector scratch) {
    std::sort(scratch.begin(), scratch.end(), compare);

    size_t deduped_size = 0;

    for (size_t i = 0; i < scratch.size();) {
      const T &value = scratch[i];

      if (i != deduped_size) {
        scratch[deduped_size] = scratch[i];
      }

      while (++i < scratch.size() && !compare(value, scratch[i])) {
        assert(compare(value, scratch[i]));
      }

      deduped_size++;
    }

    tree.resize(deduped_size);
    buildTree(0, scratch.begin(), scratch.begin() + deduped_size);

    leftmost = digLeft(0, tree.size());
    rightmost = digRight(0, tree.size());
  }

public:
  typedef typename Vector::const_iterator UnorderedIterator;

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
        const size_t right = goRight(index);

        if (right < ss.size()) {
          /* Case (ii) */
          index = digLeft(right, ss.size());
        } else {
          /* Case (iii) */
          assert(index > 0);

          while (!isLeft(index)) {
            index = goUp(index);
            assert(index > 0);
          }

          index = goUp(index);
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

      const size_t left = goLeft(index);

      if (left < ss.size()) {
        index = digRight(left, ss.size());
      } else {
        assert(index > 0);

        while (!isRight(index)) {
          index = goUp(index);
          assert(index > 0);
        }

        index = goUp(index);
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
    initialize(Vector(first, last));
  }

  StaticSet(std::initializer_list<T> list, const Compare &comp = Compare(), const Allocator &alloc = Allocator())
      : compare(comp), tree(alloc) {
    initialize(Vector(list, alloc));
  }

  StaticSet(const StaticSet &other) = default;

  StaticSet(StaticSet &&other) = default;

  StaticSet<T, Compare, Allocator> &operator=(const StaticSet<T, Compare, Allocator> &other) = default;

  StaticSet<T, Compare, Allocator> &operator=(StaticSet<T, Compare, Allocator> &&other) = default;

  StaticSet<T, Compare, Allocator> &operator=(std::initializer_list<T> list) { initialize(Vector(list)); }

  size_t size() const { return tree.size(); }

  bool empty() const { return tree.empty(); }

  OrderedIterator begin() const { return OrderedIterator(*this, ((size() == 0) ? 1 : leftmost)); }

  OrderedIterator end() const { return OrderedIterator(*this, size() + 1); }

  UnorderedIterator ubegin() const { return tree.cbegin(); }

  UnorderedIterator uend() const { return tree.cend(); }

  bool contains(const T &needle) const { return (find(needle) != end()); }

  OrderedIterator find(const T &needle) const {
    const OrderedIterator iterator = lower_bound(needle);
    assert(!compare(*iterator, needle));

    return ((iterator == end() || compare(needle, *iterator)) ? end() : iterator);
  }

  OrderedIterator lower_bound(const T &needle) const {
    size_t index = 0;
    size_t best = size() + 1;

    while (index < size()) {
      if (compare(needle, tree[index])) {
        best = index;
        index = goLeft(index);
      } else if (compare(tree[index], needle)) {
        index = goRight(index);
      } else {
        best = index;
        break;
      }
    }

    assert(best == size() + 1 || !compare(tree[best], needle));

    return OrderedIterator(*this, best);
  }

  OrderedIterator lowerBound(const T &needle) const { return lower_bound(needle); }

  OrderedIterator upper_bound(const T &needle) const {
    size_t index = 0;
    size_t best = size() + 1;

    while (index < size()) {
      if (compare(needle, tree[index])) {
        best = index;
        index = goLeft(index);
      } else {
        index = goRight(index);
      }
    }

    assert(best == size() + 1 || compare(needle, tree[best]));

    return OrderedIterator(*this, best);
  }

  OrderedIterator upperBound(const T &needle) const { return upper_bound(needle); }
};

#endif
