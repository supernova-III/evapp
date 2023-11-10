#pragma once
#include <stdint.h>
#include <assert.h>

class ArenaAllocator {
  struct ListNode {
    void* buffer = nullptr;
    ListNode* prev = nullptr;
    uint16_t current_size;
  };

  ListNode* tail_;
  uint16_t buffer_size_;

  ListNode* allocateNode() {
    return new ListNode{.buffer = ::operator new(buffer_size_),
                        .prev = tail_,
                        .current_size = 0};
  }

 public:
  ArenaAllocator(uint16_t buffer_size) : buffer_size_(buffer_size) {
    tail_ = allocateNode();
  }

  void* Allocate(uint16_t size) noexcept {
    assert(tail_ != nullptr);
    assert(size <= buffer_size_);
    if (tail_->current_size + size >= buffer_size_) {
      tail_ = allocateNode();
    }

    void* result = (char*)(tail_->buffer) + tail_->current_size;
    tail_->current_size += size;
    return result;
  }
};

class BlockAllocator {
  ArenaAllocator arena_;
  // Block size in bytes
  uint16_t block_size_;

 public:
  BlockAllocator(uint16_t block_size, uint16_t n_blocks)
      : arena_(block_size * n_blocks), block_size_(block_size) {}

  void* AllocateBlock() noexcept { return arena_.Allocate(block_size_); }
};
