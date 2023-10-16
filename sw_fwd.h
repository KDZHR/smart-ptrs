#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class EnableSharedFromThisBase {};
template <typename T>
class EnableSharedFromThis;

class BaseBlock {
public:
    virtual void IncStrongCounter() = 0;
    virtual void IncWeakCounter() = 0;
    virtual void DecStrongCounter() = 0;
    virtual void DecWeakCounter() = 0;
    virtual size_t GetStrongCounter() const = 0;
    virtual size_t GetWeakCounter() const = 0;
    virtual ~BaseBlock() {
    }
};

template <class T>
class BlockPointer : public BaseBlock {
public:
    BlockPointer(T* obj) : strong_counter_(1), weak_counter_(0), object_(obj) {
    }
    void IncStrongCounter() {
        ++strong_counter_;
    }
    void IncWeakCounter() {
        ++weak_counter_;
    }
    void DecStrongCounter() {
        --strong_counter_;
        if (strong_counter_ == 0) {
            ++weak_counter_;
            delete object_;
            --weak_counter_;
            if (weak_counter_ == 0) {
                delete this;
            }
        }
    }
    void DecWeakCounter() {
        --weak_counter_;
        if (weak_counter_ == 0 && strong_counter_ == 0) {
            delete this;
        }
    }
    size_t GetStrongCounter() const {
        return strong_counter_;
    }
    size_t GetWeakCounter() const {
        return weak_counter_;
    }

private:
    size_t strong_counter_;
    size_t weak_counter_;
    T* object_;
};

template <class T>
class BlockObject : public BaseBlock {
public:
    template <typename... Args>
    BlockObject(Args&&... args) : strong_counter_(1), weak_counter_(0) {
        new (&object_) T(std::forward<Args>(args)...);
    }
    void IncStrongCounter() {
        ++strong_counter_;
    }
    void IncWeakCounter() {
        ++weak_counter_;
    }
    void DecStrongCounter() {
        --strong_counter_;
        if (strong_counter_ == 0) {
            ++weak_counter_;
            GetObserver()->~T();
            --weak_counter_;
            if (weak_counter_ == 0) {
                delete this;
            }
        }
    }
    void DecWeakCounter() {
        --weak_counter_;
        if (weak_counter_ == 0 && strong_counter_ == 0) {
            delete this;
        }
    }
    size_t GetStrongCounter() const {
        return strong_counter_;
    }
    size_t GetWeakCounter() const {
        return weak_counter_;
    }
    T* GetObserver() {
        return reinterpret_cast<T*>(&object_);
    }

private:
    size_t strong_counter_;
    size_t weak_counter_;
    std::aligned_storage_t<sizeof(T), alignof(T)> object_;
};