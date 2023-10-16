#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : block_(nullptr), observer_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : block_(nullptr), observer_(nullptr) {
    }
    template <class Y>
    explicit SharedPtr(Y* ptr) : block_(new BlockPointer<Y>(ptr)), observer_(ptr) {
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            InitWeakThis(ptr);
        }
    }

    SharedPtr(const SharedPtr& other) : block_(other.block_), observer_(other.observer_) {
        if (block_ != nullptr) {
            block_->IncStrongCounter();
        }
    }

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other) : block_(other.block_), observer_(other.observer_) {
        if (block_ != nullptr) {
            block_->IncStrongCounter();
        }
    }
    template <class Y>
    SharedPtr(SharedPtr<Y>&& other) : block_(other.block_), observer_(other.observer_) {
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }
    explicit SharedPtr(BlockObject<T>* ptr) : block_(ptr), observer_(ptr->GetObserver()) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            InitWeakThis(ptr->GetObserver());
        }
    }
    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.block_), observer_(ptr) {
        if (block_ != nullptr) {
            block_->IncStrongCounter();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        block_ = other.block_;
        observer_ = other.observer_;
        if (block_ != nullptr) {
            block_->IncStrongCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (block_ != nullptr) {
            block_->DecStrongCounter();
        }
        block_ = other.block_;
        observer_ = other.observer_;
        if (block_ != nullptr) {
            block_->IncStrongCounter();
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (block_ != nullptr) {
            block_->DecStrongCounter();
        }
        block_ = other.block_;
        observer_ = other.observer_;
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (block_ != nullptr) {
            block_->DecStrongCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DecStrongCounter();
            block_ = nullptr;
            observer_ = nullptr;
        }
    }
    template <class Y>
    void Reset(Y* ptr) {
        Reset();
        block_ = new BlockPointer<Y>(ptr);
        observer_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observer_, other.observer_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return observer_;
    }
    T& operator*() const {
        return *observer_;
    }
    T* operator->() const {
        return observer_;
    }
    size_t UseCount() const {
        if (block_ != nullptr) {
            return block_->GetStrongCounter();
        }
        return 0;
    }
    explicit operator bool() const {
        return block_ != nullptr;
    }
    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
    }

private:
    BaseBlock* block_;
    T* observer_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new BlockObject<T>(std::forward<Args>(args)...));
}

// Look for usage examples in tests

template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;

public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(weak_this_);
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(weak_this_);
    }

private:
    WeakPtr<T> weak_this_;
};
