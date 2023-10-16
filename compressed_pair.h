#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>

template <typename T, size_t ind, bool inh = !std::is_final_v<T> && std::is_empty_v<T>>
class CPElem;

template <typename T, size_t ind>
class CPElem<T, ind, false> {
public:
    CPElem() : val_(T()) {
    }
    template <typename InitType>
    CPElem(InitType&& init) : val_(std::forward<InitType>(init)) {
    }
    T& Get() {
        return val_;
    }
    const T& Get() const {
        return val_;
    }

private:
    T val_;
};

template <typename T, size_t ind>
class CPElem<T, ind, true> : T {
public:
    CPElem() : T() {
    }
    template <typename InitType>
    CPElem(InitType&& init) : T(std::forward<InitType>(init)) {
    }
    T& Get() {
        return static_cast<T&>(*this);
    }

    const T& Get() const {
        return static_cast<const T&>(*this);
    }
};

template <typename F, typename S>
class CompressedPair : CPElem<F, 0>, CPElem<S, 1> {
    using FElem = CPElem<F, 0>;
    using SElem = CPElem<S, 1>;

public:
    CompressedPair() = default;
    template <typename FInitType, typename SInitType>
    CompressedPair(FInitType&& first, SInitType&& second)
        : FElem(std::forward<FInitType>(first)), SElem(std::forward<SInitType>(second)) {
    }

    F& GetFirst() {
        return static_cast<FElem*>(this)->Get();
    }
    const F& GetFirst() const {
        return static_cast<const FElem*>(this)->Get();
    }

    S& GetSecond() {
        return static_cast<SElem*>(this)->Get();
    }
    const S& GetSecond() const {
        return static_cast<const SElem*>(this)->Get();
    };

private:
};