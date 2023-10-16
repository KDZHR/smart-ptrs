#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <class T>
class Slug {
public:
    Slug() = default;
    template <class D>
    requires std::is_base_of_v<T, D> Slug(const Slug<D>& other) {
    }
    void operator()(T* p) const {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete p;
    }
};

template <class T>
class Slug<T[]> {
public:
    Slug() = default;
    template <class D>
    requires std::is_base_of_v<std::remove_extent<T>, std::remove_extent<D>> Slug(
        const Slug<D>& other) {
    }
    void operator()(T* p) const {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete[] p;
    }
};

// Primary template
template <class T, class Deleter = Slug<T>>
class UniquePtr {
    template <class DerT, class Del>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : pair_(ptr, std::move(deleter)) {
    }
    UniquePtr(const UniquePtr& other) = delete;
    template <class D, class Del>
    requires std::is_base_of_v<T, D> || std::is_same_v<T, D> UniquePtr(UniquePtr<D, Del>&& other)
    noexcept : pair_(std::move(other.Get()), std::move(other.pair_.GetSecond())) {
        other.pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    template <class D, class Del>
    requires std::is_base_of_v<T, D> || std::is_same_v<T, D> UniquePtr& operator=(
        UniquePtr<D, Del>&& other) noexcept {
        auto cur_ptr = pair_.GetFirst();
        if (cur_ptr == other.pair_.GetFirst()) {
            return *this;
        }
        if (cur_ptr != nullptr) {
            GetDeleter()(cur_ptr);
        }
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        auto& cur_ptr = pair_.GetFirst();
        if (cur_ptr != nullptr) {
            GetDeleter()(cur_ptr);
        }
        cur_ptr = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        auto& ptr = pair_.GetFirst();
        if (ptr != nullptr) {
            GetDeleter()(ptr);
            ptr = nullptr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
    template <class DerT, class Del>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : pair_(ptr, std::move(deleter)) {
    }
    UniquePtr(const UniquePtr& other) = delete;
    template <class D, class Del>
    requires std::is_base_of_v<T, D> || std::is_same_v<T, D> UniquePtr(UniquePtr<D, Del>&& other)
    noexcept : pair_(std::move(other.Get()), std::move(other.pair_.GetSecond())) {
        other.pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    template <class D, class Del>
    requires std::is_base_of_v<T, D> || std::is_same_v<T, D> UniquePtr& operator=(
        UniquePtr<D, Del>&& other) noexcept {
        auto cur_ptr = pair_.GetFirst();
        if (cur_ptr == other.pair_.GetFirst()) {
            return *this;
        }
        if (cur_ptr != nullptr) {
            GetDeleter()(cur_ptr);
        }
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        auto& cur_ptr = pair_.GetFirst();
        if (cur_ptr != nullptr) {
            GetDeleter()(cur_ptr);
        }
        cur_ptr = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        auto& ptr = pair_.GetFirst();
        if (ptr != nullptr) {
            GetDeleter()(ptr);
            ptr = nullptr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Operator[]
    T& operator[](size_t index) const {
        return pair_.GetFirst()[index];
    };

private:
    CompressedPair<T*, Deleter> pair_;
};