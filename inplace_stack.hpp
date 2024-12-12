/* Inplace stack implementation in C++14 and later with constexpr */
#ifndef INPLACE_STACK_HPP
#define INPLACE_STACK_HPP

#include <initializer_list>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstring>
#include <new>

namespace impl {

namespace detail {

#if __cplusplus >= 201703L // C++17
template <typename T>
constexpr bool nothrow_swap = std::is_nothrow_swappable<T>::value;
#else
template <typename T>
constexpr bool nothrow_swap = noexcept(std::swap(std::declval<T&>(), std::declval<T&>()));
#endif

} // namespace detail

template <typename T, size_t N>
class Inplace_stack_is_cexpr {
public:
    using value_type = T;
    using pointer    = T*;
    using reference  = T&;
    using const_reference = const T&;

public:
    constexpr Inplace_stack_is_cexpr() = default;
    constexpr Inplace_stack_is_cexpr(std::initializer_list<T> il) {
        const size_t count = il.size() < N ? il.size() : N;
        auto begin = il.begin();
        for (size_t i = 0; i < count; i++)
            m_data[i] = *begin++;
        m_size = count;
    }

    constexpr void push(const T& new_value) {
        if (size() == capacity())
            throw std::bad_alloc();
        m_data[m_size++] = new_value;
    }
    constexpr void push(T&& new_value) {
        if (size() == capacity())
            throw std::bad_alloc();
        m_data[m_size++] = std::move(new_value);
    }

    constexpr pointer try_push(const T& new_value) {
        if (size() == capacity())
            return nullptr;
        m_data[m_size++] = new_value;
        return &top();
    }
    constexpr pointer try_push(T&& new_value) {
        if (size() == capacity())
            return nullptr;
        m_data[m_size++] = std::move(new_value);
        return &top();
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        if (size() == capacity())
            throw std::bad_alloc();
        m_data[m_size++] = T(std::forward<Args>(args)...);
        return top();
    }
    template <typename... Args>
    constexpr pointer try_emplace(Args&&... args) {
        if (size() == capacity())
            return nullptr;
        m_data[m_size++] = T(std::forward<Args>(args)...);
        return &top();
    }

    constexpr void pop() {
        if (empty()) return;
        --m_size;
    }

    constexpr reference top() noexcept {
        return m_data[m_size - 1];
    }
    constexpr const_reference top() const noexcept {
        return m_data[m_size - 1];
    }

    constexpr bool empty() const noexcept { return m_size == 0; }
    constexpr bool full()  const noexcept { return m_size == N; }
    constexpr size_t size() const noexcept { return m_size; }
    static constexpr size_t capacity() noexcept { return N; }

    constexpr void swap(Inplace_stack_is_cexpr& other)
    noexcept(detail::nothrow_swap<T>) {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
    }

private:
    T m_data[N] {};
    size_t m_size = 0;
}; // class Inplace_stack_is_cexpr

template <typename T, size_t N>
class Inplace_stack_no_cexpr {
public:
    using value_type = T;
    using pointer    = T*;
    using reference  = T&;
    using const_reference = const T&;

public:
    Inplace_stack_no_cexpr() = default;
    Inplace_stack_no_cexpr(std::initializer_list<T> il) {
        const size_t count = il.size() < N ? il.size() : N;
        auto begin = il.begin();
        for (size_t i = 0; i < count; i++)
            push(*begin++);
        m_size = count;
    }
    ~Inplace_stack_no_cexpr() {
        while (!empty()) pop();
    }

    Inplace_stack_no_cexpr(const Inplace_stack_no_cexpr& other) {
        for (size_t i = 0; i < other.size(); i++)
            push(*reinterpret_cast<const T*>(
                other.m_data + i * sizeof(T)));
    }
    Inplace_stack_no_cexpr(Inplace_stack_no_cexpr&& other) noexcept {
        std::memcpy(m_data, other.m_data, N * sizeof(T));
        std::memset(other.m_data, 0, N * sizeof(T));
        m_size = std::move(other.m_size);
        other.m_size = 0;
    }

    Inplace_stack_no_cexpr& operator=(const Inplace_stack_no_cexpr& other) {
        Inplace_stack_no_cexpr tmp(other);
        tmp.swap(*this);
        return *this;
    }
    Inplace_stack_no_cexpr& operator=(Inplace_stack_no_cexpr&& other) noexcept {
        other.swap(*this);
        return *this;
    }

    void push(const T& new_value) {
        if (size() == capacity())
            throw std::bad_alloc();
        new (get_next_cell()) T(new_value);
    }
    void push(T&& new_value) {
        if (size() == capacity())
            throw std::bad_alloc();
        new (get_next_cell()) T(std::move(new_value));
    }

    pointer try_push(const T& new_value) {
        if (size() == capacity())
            return nullptr;
        new (get_next_cell()) T(new_value);
        return get_curr_cell();
    }
    pointer try_push(T&& new_value) {
        if (size() == capacity())
            return nullptr;
        new (get_next_cell()) T(std::move(new_value));
        return get_curr_cell();
    }

    template <typename... Args>
    reference emplace(Args&&... args) {
        if (size() == capacity())
            throw std::bad_alloc();
        new (get_next_cell()) T(std::forward<Args>(args)...);
        return top();
    }
    template <typename... Args>
    pointer try_emplace(Args&&... args) {
        if (size() == capacity())
            return nullptr;
        new (get_next_cell()) T(std::forward<Args>(args)...);
        return get_curr_cell();
    }

    void pop() {
        if (empty()) return;
        get_last_cell()->~T();
    }

    reference top() { return *get_curr_cell(); }
    const_reference top() const { return *get_curr_cell(); }

    bool empty() const noexcept { return m_size == 0; }
    bool full()  const noexcept { return m_size == N; }
    size_t size() const noexcept { return m_size; }
    static constexpr size_t capacity() noexcept { return N; }

    void swap(Inplace_stack_no_cexpr& other)
    noexcept(detail::nothrow_swap<T>) {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
    }

private:
    const pointer get_curr_cell() const {
        return reinterpret_cast<const pointer>(
            m_data + (m_size - 1) * sizeof(T));
    }
    pointer get_curr_cell() {
        return reinterpret_cast<pointer>(
            m_data + (m_size - 1) * sizeof(T));
    }
    pointer get_next_cell() {
        return reinterpret_cast<pointer>(
            m_data + m_size++ * sizeof(T));
    }
    pointer get_last_cell() {
        return reinterpret_cast<pointer>(
            m_data + --m_size * sizeof(T));
    }

private:
    alignas(T) char m_data[N * sizeof(T)] {};
    size_t m_size = 0;
}; // class Inplace_stack_no_cexpr

template <typename T> class Inplace_stack_is_cexpr<T, 0> {};
template <typename T> class Inplace_stack_no_cexpr<T, 0> {};

} // namespace impl

template <typename T, size_t N>
using inplace_stack = typename std::conditional<
    std::is_trivial<T>::value,
    impl::Inplace_stack_is_cexpr<T, N>,
    impl::Inplace_stack_no_cexpr<T, N>
>::type;

namespace std {
    template <typename T, size_t N>
    constexpr void swap(inplace_stack<T, N>& a, inplace_stack<T, N>& b)
    noexcept(noexcept(a.swap(b))) { a.swap(b); }
}

#endif // INPLACE_STACK_HPP