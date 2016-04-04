/*
 * fast_allocator.h
 *
 *  Created on: 4 апр. 2016 г.
 *      Author: user
 */

#ifndef FAST_ALLOCATOR_H_
#define FAST_ALLOCATOR_H_

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iostream>

class FixedAllocatorBase {
  template<typename T>
  friend class TFastAllocator;

protected:
  virtual ~FixedAllocatorBase() {
  }
  virtual void* GiveChunk() = 0;
  virtual void ReleaseChunk(void* chunk_to_release) = 0;
};

template<size_t ChunkSize = 20>
class FixedAllocator : FixedAllocatorBase {
  friend class FixedAllocatorInstancesOwner;

private:
  struct Chunk {
    Chunk* next;
    char data[ChunkSize - sizeof(Chunk*)];

    Chunk()
        : next(nullptr) {
    }

    Chunk(Chunk* const next)
        : next(next) {
    }
  };

  FixedAllocator(const size_t chunks_in_one_pool)
      : free_head(nullptr), chunks_in_one_pool(chunks_in_one_pool) {
  }

  FixedAllocator(const FixedAllocator& other) = delete;

  ~FixedAllocator() {
  }

  FixedAllocator& operator=(const FixedAllocator& other) = delete;

  void AddNewPool() {
    chunks_pools.emplace_back(chunks_in_one_pool);
    auto& new_chunks = chunks_pools.back();
    free_head = &new_chunks.front();
    for (size_t new_chunk_index = 0;
        new_chunk_index != new_chunks.size() - 1; ++new_chunk_index) {
      new_chunks[new_chunk_index].next = &new_chunks[new_chunk_index + 1];
    }
    new_chunks.back().next = nullptr;
  }

  virtual void* GiveChunk() {
    if (free_head == nullptr) {
      // Out of free chunks.
      AddNewPool();
    }

    auto tmp = free_head->next;
    free_head->~Chunk();
    void* new_memory_ptr = static_cast<void*>(free_head);
    free_head = tmp;

    return new_memory_ptr;
  }

  virtual void ReleaseChunk(void* chunk_to_release) {
    Chunk* next_of_new_free_head = free_head;
    free_head = static_cast<Chunk*>(chunk_to_release);
    new (free_head) Chunk();
    free_head->next = next_of_new_free_head;
  }

  std::vector<std::vector<Chunk>> chunks_pools;
  Chunk* free_head;
  const size_t chunks_in_one_pool;
};

class FixedAllocatorInstancesOwner {
  template<typename T>
  friend class TFastAllocator;

private:
  static FixedAllocatorBase* GetInstance(const size_t chunk_size) {
    static FixedAllocator<12> twelve(100000);
    static FixedAllocator<16> sixteen(100000);
    static FixedAllocator<20> twenty(100000);
    static FixedAllocator<24> twenty_four(100000);
    switch (chunk_size) {
    case 12:
      return &twelve;
    case 16:
      return &sixteen;
    case 20:
      return &twenty;
    case 24:
      return &twenty_four;
    default:
      return nullptr;
    }
  }
};

template<typename T>
class TFastAllocator {
public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  template<typename U>
  struct rebind {
    typedef TFastAllocator<U> other;
  };

  TFastAllocator() {
  }

  template<typename U>
  TFastAllocator(const TFastAllocator<U>& other) {
  }

  pointer address(reference r) const {
    std::cout << "address: " << &r << std::endl;
    return &r;
  }

  const_pointer address(const_reference r) const {
    std::cout << "const_address: " << &r << std::endl;
    return &r;
  }

  pointer allocate(size_type n) {
    const size_t bytes_to_allocate = n * sizeof(value_type);
//    std::cout << "allocate: " << n << "objects " << bytes_to_allocate
//        << " bytes" << std::endl;
    auto fixed_allocator = FixedAllocatorInstancesOwner::GetInstance(
        bytes_to_allocate);
    if (fixed_allocator == nullptr) {
      // There is no fixed allocator with ChunkSize equal to bytes_to_allocate
      return static_cast<pointer>(static_cast<void*>(new char[bytes_to_allocate]));
    }

    void* chunk_pointer = fixed_allocator->GiveChunk();
    return static_cast<pointer>(chunk_pointer);
  }

  void deallocate(pointer p, size_type n) {
    const size_t bytes_to_deallocate = n * sizeof(value_type);
//    std::cout << "deallocate: " << n << "objects " << bytes_to_deallocate
//        << " bytes" << std::endl;
    void* raw_pointer = static_cast<void*>(p);
    auto fixed_allocator = FixedAllocatorInstancesOwner::GetInstance(
        bytes_to_deallocate);
    if (fixed_allocator == nullptr) {
      delete static_cast<char*>(raw_pointer);
    } else {
      fixed_allocator->ReleaseChunk(raw_pointer);
    }
  }

  template<typename ... Args>
  void construct(pointer p, Args&& ... args) {
//    std::cout << "args-construct: " << p << std::endl;
    new (p) T(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
//    std::cout << "destroy: " << p << std::endl;
    p->~T();
  }

  size_type max_size() const noexcept {
    return size_t(-1) / sizeof(value_type);
  }
};

#endif /* FAST_ALLOCATOR_H_ */
