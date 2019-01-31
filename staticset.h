#ifndef LIBSTATICSET_STATICSET_H
#define LIBSTATICSET_STATICSET_H

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

template <class T, class Compare = std::less<T>, class Allocator = std::allocator<T>> class StaticSet {
  typedef std::vector<T> Vector;
  typedef typename Vector::iterator VectorIterator;

  Compare compare;
  Vector tree;

  void BuildTree(size_t index, const VectorIterator slice_begin, const VectorIterator slice_end) {
    const size_t count = slice_end - slice_begin;

    if (count <= 0) {
      return;
    }

    const auto slice_center = slice_begin + count / 2;
    tree[index] = *slice_center;

    BuildTree(2 * index + 1, slice_begin, slice_center);
    BuildTree(2 * index + 2, slice_center + 1, slice_end);
  }

  void Initialize(Vector scratch) {
    std::sort(scratch.begin(), scratch.end(), compare);
    tree.resize(scratch.size());
    BuildTree(0, scratch.begin(), scratch.end());
  }

public:
  typedef T value_type;
  typedef Allocator allocator_type;
  typedef typename Vector::size_type size_type;

  class OrderedIterator {
    const StaticSet<T, Compare, Allocator> &ss;
    size_t index;

  public:
    typedef size_t difference_type;
    typedef T value_type;
    typedef const T *pointer;
    typedef const T &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    reference operator*() const { return ss.tree(index); }

    pointer operator->() const { return &ss.tree(index); }

    bool operator==(const OrderedIterator &other) const { return (&ss == &other.ss && index == other.index); }

    bool operator!=(const OrderedIterator &other) const { return !(*this == other); }
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
};

#endif
