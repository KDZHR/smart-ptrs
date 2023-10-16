#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : block_(nullptr), observer_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : block_(other.block_), observer_(other.observer_) {
        if (block_ != nullptr) {
            block_->IncWeakCounter();
        }
    }
    template <class Y>
    WeakPtr(const WeakPtr<Y>& other) : block_(other.block_), observer_(other.observer_) {
        if (block_ != nullptr) {
            block_->IncWeakCounter();
        }
    }
    WeakPtr(WeakPtr&& other) : block_(other.block_), observer_(other.observer_) {
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <class Y>
    WeakPtr(const SharedPtr<Y>& other) : block_(other.block_), observer_(other.observer_) {
        if (block_ != nullptr) {
            block_->IncWeakCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (block_ != nullptr) {
            block_->DecWeakCounter();
        }
        block_ = other.block_;
        observer_ = other.observer_;
        if (block_ != nullptr) {
            block_->IncWeakCounter();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (block_ != nullptr) {
            block_->DecWeakCounter();
        }
        block_ = other.block_;
        observer_ = other.observer_;
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ != nullptr) {
            block_->DecWeakCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DecWeakCounter();
            block_ = nullptr;
            observer_ = nullptr;
        }
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observer_, other.observer_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ != nullptr) {
            return block_->GetStrongCounter();
        }
        return 0;
    }
    bool Expired() const {
        return block_ == nullptr || block_->GetStrongCounter() == 0;
    }
    SharedPtr<T> Lock() const {
        if (!Expired()) {
            return SharedPtr<T>(*this);
        }
        return SharedPtr<T>();
    }

private:
    BaseBlock* block_;
    T* observer_;
};
