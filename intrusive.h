#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }
    size_t DecRef() {
        --count_;
        return count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }
    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        size_t ref_cnt = counter_.DecRef();
        if (ref_cnt == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }
    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }
    RefCounted& operator=(const RefCounted& other) {
        return *this;
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : object_(nullptr) {
    }
    IntrusivePtr(std::nullptr_t) : object_(nullptr) {
    }
    IntrusivePtr(T* ptr) : object_(ptr) {
        object_->IncRef();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : object_(other.object_) {
        if (object_ != nullptr) {
            object_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : object_(other.object_) {
        other.object_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : object_(other.object_) {
        if (object_ != nullptr) {
            object_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) : object_(other.object_) {
        other.object_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (object_ == other.object_) {
            return *this;
        }
        if (object_ != nullptr) {
            object_->DecRef();
        }
        object_ = other.object_;
        if (object_ != nullptr) {
            object_->IncRef();
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (object_ == other.object_) {
            return *this;
        }
        if (object_ != nullptr) {
            object_->DecRef();
        }
        object_ = other.object_;
        other.object_ = nullptr;
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (object_ != nullptr) {
            object_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        Reset(nullptr);
    }
    void Reset(T* ptr) {
        if (object_ != nullptr) {
            object_->DecRef();
        }
        object_ = ptr;
        if (object_ != nullptr) {
            object_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(object_, other.object_);
    }

    // Observers
    T* Get() const {
        return object_;
    }
    T& operator*() const {
        return *object_;
    }
    T* operator->() const {
        return object_;
    }
    size_t UseCount() const {
        if (object_ != nullptr) {
            return object_->RefCount();
        }
        return 0;
    }
    explicit operator bool() const {
        return object_ != nullptr;
    }

private:
    T* object_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
