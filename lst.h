/*
 * lst.h
 *
 *  Created on: 31 марта 2016 г.
 *      Author: user
 */

#ifndef LST_H_
#define LST_H_

#include <cstddef>
#include <algorithm>
#include <iterator>

struct ListNodeBase {
  ListNodeBase* next;
  ListNodeBase* prev;

  ListNodeBase()
      : next(this), prev(this) {
  }

  void swap(ListNodeBase& other) {
    std::swap(*this, other);
    if (next == &other) {
      next = this;
      prev = this;
    } else {
      next->prev = this;
      prev->next = this;
    }
    if (other.next == this) {
      other.next = &other;
      other.prev = &other;
    } else {
      other.next->prev = &other;
      other.prev->next = &other;
    }
  }
};

template<typename T>
struct ListNode : public ListNodeBase {
  T data;

  template<typename ... Args>
  ListNode(Args&&... args)
      : data(std::forward<Args>(args)...) {
  }
};

template<typename T>
struct ListIterator {
  typedef ListNode<T> Node;

  typedef ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;

  ListNodeBase* ptr;

  ListIterator(ListNodeBase* const ptr)
      : ptr(ptr) {
  }

  reference operator*() const {
    return static_cast<Node*>(ptr)->data;
  }

  pointer operator->() const {
    return std::__addressof(static_cast<Node*>(ptr)->data);
  }

  ListIterator& operator++() {
    ptr = ptr->next;
    return *this;
  }

  ListIterator operator++(int) {
    auto tmp = *this;
    ptr = ptr->next;
    return tmp;
  }

  ListIterator& operator--() {
    ptr = ptr->prev;
    return *this;
  }

  ListIterator operator--(int) {
    auto tmp = *this;
    ptr = ptr->prev;
    return tmp;
  }

  bool operator==(const ListIterator& other) const {
    return ptr == other.ptr;
  }

  bool operator!=(const ListIterator& other) const {
    return ptr != other.ptr;
  }
};

template<typename T>
struct ListConstIterator {
  typedef const ListNode<T> Node;
  typedef ListIterator<T> iterator;

  typedef ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef T value_type;
  typedef const T* pointer;
  typedef const T& reference;

  const ListNodeBase* ptr;

  ListConstIterator(const ListNodeBase* const ptr)
      : ptr(ptr) {
  }

  ListConstIterator(const iterator& other)
      : ptr(other.ptr) {
  }

  reference operator*() const {
    return static_cast<Node*>(ptr)->data;
  }

  pointer operator->() const {
    return std::__addressof(static_cast<Node*>(ptr)->data);
  }

  ListConstIterator& operator++() {
    ptr = ptr->next;
    return *this;
  }

  ListConstIterator operator++(int) {
    auto tmp = *this;
    ptr = ptr->next;
    return tmp;
  }
  ListConstIterator& operator--() {
    ptr = ptr->prev;
    return *this;
  }

  ListConstIterator operator--(int) {
    auto tmp = *this;
    ptr = ptr->prev;
    return tmp;
  }

  bool operator==(const ListConstIterator& other) const {
    return ptr == other.ptr;
  }

  bool operator!=(const ListConstIterator& other) const {
    return ptr != other.ptr;
  }
};

template<typename T, typename Allocator = std::allocator<T>>
class TList : private Allocator::template rebind<ListNode<T>>::other {
public:
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef ListIterator<T> iterator;
  typedef ListConstIterator<T> const_iterator;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef Allocator allocator_type;

private:
  typedef typename Allocator::template rebind<ListNode<value_type>>::other NodesAllocator;

public:
  // Strong guarantees (no changes in case of exception, i. e. all would be destroyed)
  TList(const allocator_type& alloc = allocator_type())
      : NodesAllocator(alloc) {
  }

  // Strong guarantees (no changes in case of exception, i. e. all would be destroyed)
  TList(const TList& x)
      : NodesAllocator(x.GetAllocator()) {
    try {
      for (auto iter = x.begin(); iter != x.end(); ++iter) {
        emplace_back(*iter);
      }
    } catch (...) {
      DestroyAll();
    }
  }

  TList(TList&& x)
      : NodesAllocator(std::move(x.GetAllocator())) {
    swap(x);
  }

  // Strong guarantees (no changes in case of exception, i. e. all would be destroyed)
  explicit TList(size_type n) {
    try {
      for (; size_ != n; ++size_) {
        emplace_back();
      }
    } catch (...) {
      DestroyAll();
    }
  }

  // Strong guarantees (no changes in case of exception, i. e. all would be destroyed)
  TList(size_type n, const value_type& val, const allocator_type& alloc =
            allocator_type())
      : NodesAllocator(alloc) {
    try {
      while (size_ != n) {
        emplace_back(val);
      }
    } catch (...) {
      DestroyAll();
    }
  }

  // Strong guarantees (no changes in case of exception, i. e. all would be destroyed)
  template<class InputIterator, typename = std::_RequireInputIter<
      InputIterator>>
  TList(InputIterator first,
        InputIterator last,
        const allocator_type& alloc = allocator_type())
      : NodesAllocator(alloc) {
    try {
      for (auto iter = first; iter != last; ++iter) {
        emplace_back(*iter);
      }
    } catch (...) {
      DestroyAll();
    }
  }

  ~TList() {
    DestroyAll();
  }

  // Basic guarantees (no memory leak)
  TList& operator=(const TList& x) {
    if (this != &x) {
      iterator my_first = begin();
      iterator my_last = end();
      const_iterator x_first = x.begin();
      const_iterator x_last = x.end();
      for (; my_first != my_last && x_first != x_last;
          ++my_first, ++x_first) {
        *my_first = *x_first;
      }
      if (x_first == x_last) {
        erase(my_first, my_last);
      } else {
        insert(my_last, x_first, x_last);
      }
    }

    return *this;
  }

  TList& operator=(TList&& x) {
    clear();
    swap(x);
    return *this;
  }

  // Strong guarantees (no changes in case of exception)
  template<typename ... Args>
  void emplace_back(Args&&... args) {
    auto ptr = CreateNode(std::forward<Args>(args)...);
    PtrsWork(ptr, ptr, &base_);
  }

  // Strong guarantees (no changes in case of exception)
  template<typename ... Args>
  void emplace_front(Args&&... args) {
    auto ptr = CreateNode(std::forward<Args>(args)...);
    PtrsWork(ptr, ptr, base_.next);
  }

  // Strong guarantees (no changes in case of exception)
  void push_back(const value_type& val) {
    insert(end(), val);
  }
  // Strong guarantees (no changes in case of exception)
  void push_back(value_type&& val) {
    emplace_back(std::move(val));
  }

  // Strong guarantees (no changes in case of exception)
  void push_front(const value_type& val) {
    insert(begin(), val);
  }

  // Strong guarantees (no changes in case of exception)
  void push_front(value_type&& val) {
    emplace_front(std::move(val));
  }

  void pop_back() noexcept {
    erase(--end());
  }

  void pop_front() noexcept {
    erase(begin());
  }

  iterator erase(const_iterator position) noexcept {
    return erase(position, std::next(position));
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    ListNodeBase* ptr = const_cast<ListNodeBase*>(first.ptr);
    auto last_ptr = const_cast<ListNodeBase*>(last.ptr);
    auto my_left = ptr->prev;
    while (ptr != last_ptr) {
      auto tmp = ptr->next;
      this->destroy(static_cast<ListNode<value_type>*>(ptr));
      this->deallocate(static_cast<ListNode<value_type>*>(ptr), 1);
      --size_;
      ptr = tmp;
    }
    auto my_right = last_ptr;
    my_right->prev = my_left;
    if (!my_left) {
      base_.next = my_right;
    } else {
      my_left->next = my_right;
    }

    return iterator(my_right);
  }

  // Strong guarantees (no changes in case of exception)
  void splice(const_iterator position,
              TList&& x,
              const_iterator first,
              const_iterator last) noexcept {
    const size_t size_delta = std::distance(first, last);

    Splice(position, std::move(x), first, last);

    x.size_ -= size_delta;
    size_ += size_delta;
  }

  // Strong guarantees (no changes in case of exception)
  void splice(const_iterator position,
              TList& x,
              const_iterator first,
              const_iterator last) noexcept {
    splice(position, std::move(x), first, last);
  }

  // Strong guarantees (no changes in case of exception)
  void splice(const_iterator position, TList&& x) noexcept {
    if (!x.empty()) {
      Splice(position, std::move(x), x.begin(), x.end());
      size_ += x.size_;
      x.size_ = 0;
    }
  }

  // Strong guarantees (no changes in case of exception)
  void splice(const_iterator position, TList& x) noexcept {
    splice(position, std::move(x));
  }

  // Strong guarantees (no changes in case of exception)
  iterator insert(const_iterator position, const value_type& val) {
    auto new_ptr = CreateNode(val);
    auto right_ptr = const_cast<ListNodeBase*>(position.ptr);
    PtrsWork(new_ptr, new_ptr, right_ptr);
    return iterator(new_ptr);
  }

  // Strong guarantees (no changes in case of exception)
  iterator insert(const_iterator position,
                  size_type n,
                  const value_type& val) {
    TList tmp(n, val);
    iterator iter = tmp.begin();
    splice(position, tmp);
    return iter;
  }

  // Strong guarantees (no changes in case of exception)
  template<class InputIterator>
  iterator insert(const_iterator position,
                  InputIterator first,
                  InputIterator last) {
    TList tmp(first, last);
    if (!tmp.empty()) {
      iterator it = tmp.begin();
      splice(position, tmp);
      return it;
    }

    return iterator(const_cast<ListNodeBase*>(position.ptr));
  }

  iterator begin() {
    return iterator(base_.next);
  }

  const_iterator begin() const {
    return const_iterator(base_.next);
  }

  const_iterator cbegin() const {
    return const_iterator(base_.next);
  }

  iterator end() {
    return iterator(&base_);
  }

  const_iterator end() const {
    return const_iterator(&base_);
  }

  const_iterator cend() const {
    return const_iterator(&base_);
  }

  bool empty() const {
    return base_.next == &base_;
  }

  size_t size() const {
    return size_;
  }

  reference front() {
    return *begin();
  }

  const_reference front() const {
    return *begin();
  }

  reference back() {
    iterator tmp = end();
    --tmp;
    return *tmp;
  }

  const_reference back() const {
    const_iterator tmp = end();
    --tmp;
    return *tmp;
  }

  // Basic guarantees (no memory leak)
  void assign(size_type n, const value_type& val) {
    iterator i = begin();
    for (; i != end() && n > 0; ++i, --n) {
      *i = val;
    }
    if (n > 0) {
      insert(end(), n, val);
    } else {
      erase(i, end());
    }
  }

  void swap(TList& x) {
    base_.swap(x.base_);
    std::swap(size_, x.size_);
    std::swap(GetAllocator(), x.GetAllocator());
  }

  // Strong guarantees (no changes in case of exception)
  void resize(size_type new_size) {
    while (size_ > new_size) {
      erase(--end());
    }
    if (size_ < new_size) {
      DefaultAppend(new_size - size_);
    }
  }

  // Strong guarantees (no changes in case of exception)
  void resize(size_type new_size, const value_type& val) {
    while (size_ > new_size) {
      erase(--end());
    }
    if (size_ < new_size) {
      insert(end(), new_size - size_, val);
    }
  }

  void clear() noexcept {
    erase(begin(), end());
  }

  void reverse() noexcept {
    if (!empty() && base_.prev != base_.next) {
      auto current_ptr = base_.next;
      while (current_ptr != &base_) {
        auto next = current_ptr->next;
        std::swap(current_ptr->prev, current_ptr->next);
        current_ptr = next;
      }
      base_.next->next = &base_;
      base_.prev->prev = nullptr;
      auto new_before_tail = base_.next;
      base_.next = base_.prev;
      base_.prev = new_before_tail;
    }
  }

  void merge(TList&& x) {
    if (this != &x) {
      iterator my_first = begin();
      iterator my_last = end();
      iterator x_first = x.begin();
      iterator x_last = x.end();
      while (my_first != my_last && x_first != x_last)
        if (*x_first < *my_first) {
          iterator next = x_first;
          splice(my_first, x, x_first, ++next);
          x_first = next;
        } else {
          ++my_first;
        }
      if (x_first != x_last) {
        splice(my_last, std::move(x), x_first, x_last);
      }
    }
  }

  void merge(TList& x) {
    merge(std::move(x));
  }

  void sort() {
    // Let's do nothing if the list has length 0 or 1.
    if (!empty() && base_.prev != base_.next) {
      {
        TList carry;
        TList tmp[64];
        TList* fill = &tmp[0];
        TList* counter;

        do {
          carry.splice(carry.begin(), *this, begin(), ++begin());
          for (counter = &tmp[0]; counter != fill && !counter->empty();
              ++counter) {
            counter->merge(carry);
            carry.swap(*counter);
          }
          carry.swap(*counter);
          if (counter == fill) {
            ++fill;
          }
        } while (!empty());

        for (counter = &tmp[1]; counter != fill; ++counter)
          counter->merge(*(counter - 1));
        swap(*(fill - 1));
      }
    }
  }

  void unique() {
    iterator first = begin();
    iterator last = end();
    if (first == last) {
      return;
    }
    iterator next = first;
    while (++next != last) {
      if (*first == *next) {
        erase(next);
      } else {
        first = next;
      }
      next = first;
    }
  }

private:
  void DestroyAll() {
    erase(begin(), end());
  }

  NodesAllocator& GetAllocator() {
    return *static_cast<NodesAllocator*>(this);
  }

  const NodesAllocator& GetAllocator() const {
    return *static_cast<const NodesAllocator*>(this);
  }

  // Strong guarantees (no changes in case of exception)
  void DefaultAppend(size_type n) {
    size_type i = 0;
    try {
      for (; i < n; ++i) {
        emplace_back();
      }
    } catch (...) {
      for (; i; --i) {
        pop_back();
      }
      throw;
    }
  }

  // Strong guarantees
  template<typename ... Args>
  ListNode<value_type>* CreateNode(Args&&... args) {
    ListNode<value_type>* ptr(this->allocate(1));
    try {
      this->construct(ptr, std::forward<Args>(args)...);
    } catch (...) {
      this->deallocate(ptr, 1);
      throw;
    }

    ++size_;
    return ptr;
  }

  // Some rountine work with ptrs. new_left and new_right are first and last
  // nodes of inserted diapason. new_left and new_right might be equal.
  // my_right_ptr - is old node which already existed. new nodes are inserted right before it.
  void PtrsWork(ListNodeBase* new_left,
                ListNodeBase* new_right,
                ListNodeBase* my_right_ptr) {
    auto my_left = my_right_ptr->prev;
    my_right_ptr->prev = new_right;
    new_right->next = my_right_ptr;
    new_left->prev = my_left;
    if (!my_left) {
      base_.next = new_left;
    } else {
      my_left->next = new_left;
    }
  }

  void Splice(const_iterator position,
              TList&& x,
              const_iterator first,
              const_iterator last) noexcept {
    auto x_first_to_transfer = const_cast<ListNodeBase*>(first.ptr);
    auto x_last_to_transfer = const_cast<ListNodeBase*>((--last).ptr);
    auto x_first_after_transferred_ptrs = x_last_to_transfer->next;
    auto x_last_before_transferred_ptrs = x_first_to_transfer->prev;
    auto my_first_after_new_transeferred_ptrs =
        const_cast<ListNodeBase*>(position.ptr);
    PtrsWork(x_first_to_transfer, x_last_to_transfer,
        my_first_after_new_transeferred_ptrs);

    x_first_after_transferred_ptrs->prev = x_last_before_transferred_ptrs;
    if (!x_last_before_transferred_ptrs) {
      x.base_.next = x_first_after_transferred_ptrs;
    } else {
      x_last_before_transferred_ptrs->next =
          x_first_after_transferred_ptrs;
    }
  }

  ListNodeBase base_;
  size_t size_ = 0;
};



template<typename T>
inline bool operator==(const ListIterator<T>& x,
                       const ListConstIterator<T>& y) noexcept
                       {
  return x.ptr == y.ptr;
}

template<typename T>
inline bool operator!=(const ListIterator<T>& x,
                       const ListConstIterator<T>& y) noexcept
                       {
  return x.ptr != y.ptr;
}

template<typename T, typename Alloc>
inline bool operator==(const TList<T, Alloc>& x,
                       const TList<T, Alloc>& y) {
  typedef typename TList<T, Alloc>::const_iterator const_iterator;
  const_iterator end1 = x.end();
  const_iterator end2 = y.end();

  const_iterator i1 = x.begin();
  const_iterator i2 = y.begin();
  while (i1 != end1 && i2 != end2 && *i1 == *i2) {
    ++i1;
    ++i2;
  }
  return i1 == end1 && i2 == end2;
}



template<typename T, typename Alloc>
inline bool operator==(const TList<T, Alloc>& x,
                       const std::list<T, Alloc>& y) {
  typedef typename TList<T, Alloc>::const_iterator const_iterator;
  typedef typename std::list<T, Alloc>::const_iterator std_const_iterator;
  const_iterator end1 = x.end();
  std_const_iterator end2 = y.end();

  const_iterator i1 = x.begin();
  std_const_iterator i2 = y.begin();
  while (i1 != end1 && i2 != end2 && *i1 == *i2) {
    ++i1;
    ++i2;
  }
  return i1 == end1 && i2 == end2 && x.size() == y.size();
}

#endif /* LST_H_ */
