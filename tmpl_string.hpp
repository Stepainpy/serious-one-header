/* C++14 and later, can use as parameter in template start at C++20 */
#ifndef TMPL_STRING_HPP
#define TMPL_STRING_HPP

#include <iosfwd>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#ifdef __cpp_lib_string_view
#include <string_view>
#endif

#if __cplusplus >= 202002L // C++20
#define CEX_CXX20 constexpr
#else
#define CEX_CXX20
#endif

#if __cplusplus >= 201703L // C++17
#define CEX_CXX17 constexpr
#else
#define CEX_CXX17
#endif

namespace detail {
#ifdef __cpp_lib_logical_traits
    using std::conjunction;
#else
    template <class... Bs> struct conjunction : std::true_type {};
    template <class B> struct conjunction<B> : B {};
    template <class B, class... Bs>
    struct conjunction<B, Bs...> : std::conditional<
        bool(B::value), conjunction<Bs...>, B>::type {};
#endif

    /* Get from GCC C++ standard library */

    template <typename CharT, typename Traits>
    void ostream_write(std::basic_ostream<CharT, Traits>& out, const CharT* data, std::streamsize n) {
        using os_t       = std::basic_ostream<CharT, Traits>;
        using ios_base_t = typename os_t::ios_base;

        const std::streamsize put = out.rdbuf()->sputn(data, n);
        if (put != n)
            out.setstate(ios_base_t::badbit);
    }

    template <typename CharT, typename Traits>
    void ostream_fill(std::basic_ostream<CharT, Traits>& out, std::streamsize n) {
        using os_t       = std::basic_ostream<CharT, Traits>;
        using ios_base_t = typename os_t::ios_base;

        const CharT c = out.fill();
        for (; n > 0; --n) {
            const typename Traits::int_type put = out.rdbuf()->sputc(c);
            if (Traits::eq_int_type(put, Traits::eof())) {
                out.setstate(ios_base_t::badbit);
                break;
            }
        }
    }

    template <typename CharT, typename Traits> std::basic_ostream<CharT, Traits>&
    ostream_insert(std::basic_ostream<CharT, Traits>& out, const CharT* data, std::streamsize n) {
        using os_t       = std::basic_ostream<CharT, Traits>;
        using ios_base_t = typename os_t::ios_base;

        typename os_t::sentry cerb(out);
        if (cerb) {
            try {
                const std::streamsize w = out.width();
                if (w > n) {
                    const bool left = ((out.flags() & ios_base_t::adjustfield) == ios_base_t::left);
                    if (!left)
                        ostream_fill(out, w - n);
                    if (out.good())
                        ostream_write(out, data, n);
                    if (left && out.good())
                        ostream_fill(out, w - n);
                } else ostream_write(out, data, n);
                out.width(0);
            } catch (...) {
                out.setstate(ios_base_t::badbit);
            }
        }

        return out;
    }
}

template <
    size_t N, typename CharT,
    typename Traits = std::char_traits<CharT>
> struct basic_tmpl_string {
    using value_type      = CharT;
    using traits_type     = Traits;
    using pointer         = CharT*;
    using const_pointer   = const CharT*;
    using reference       = CharT&;
    using const_reference = const CharT&;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using const_iterator = const CharT*;
    using iterator       = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = const_reverse_iterator;

    value_type content[N] {}; // null-terminated string

public: // constructors
    constexpr basic_tmpl_string(const value_type (&str)[N]) noexcept {
        for (size_t i = 0; i < N; i++)
            traits_type::assign(content[i], str[i]);
    }
    template <typename... CharTs>
    constexpr basic_tmpl_string(CharTs&&... chars) noexcept : content{chars...} {
        static_assert(detail::conjunction<std::is_same<CharTs, value_type>...>::value, "All CharTs must be equal value_type");
        static_assert(sizeof...(chars) + 1 == N, "Count of chars not equal N - 1");
    }

    constexpr basic_tmpl_string() noexcept = delete;
    constexpr basic_tmpl_string(const basic_tmpl_string&) noexcept = default;
    constexpr basic_tmpl_string(basic_tmpl_string&&) noexcept = default;
    constexpr basic_tmpl_string& operator=(const basic_tmpl_string&) noexcept = default;
    constexpr basic_tmpl_string& operator=(basic_tmpl_string&&) noexcept = default;
    CEX_CXX20 ~basic_tmpl_string() noexcept = default;

public: // iterators
    constexpr const_iterator  begin() const noexcept { return data(); }
    constexpr const_iterator cbegin() const noexcept { return data(); }
    
    constexpr const_iterator  end() const noexcept { return data() + size(); }
    constexpr const_iterator cend() const noexcept { return data() + size(); }

    constexpr const_reverse_iterator  rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

    constexpr const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

public: // element access
    constexpr const_reference operator[](size_type pos) const noexcept { return *(data() + pos); }
    constexpr const_reference at(size_type pos) const {
        if (pos >= size())
            throw std::out_of_range("basic_tmpl_string::at: pos >= this->size()");
        return *(data() + pos);
    }
    constexpr const_reference front() const noexcept { return *begin(); }
    constexpr const_reference back() const noexcept { return *rbegin(); }
    constexpr const_pointer data() const noexcept { return content; }

public: // convert
    constexpr const_pointer c_str() const noexcept { return data(); }
#ifdef __cpp_lib_string_view
    constexpr operator std::basic_string_view<value_type, traits_type>() const noexcept { return {data(), size()}; }
#endif

public: // capacity
    constexpr size_type size() const noexcept { return N - 1; }
    constexpr size_type length() const noexcept { return size(); }
    constexpr bool empty() const noexcept { return begin() == end(); }

public: // comparation
    template <size_t M>
    CEX_CXX17 int compare(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept {
        const size_type s1 = size(), s2 = rhs.size();
        const size_type count = s1 < s2 ? s2 : s2;
        const int cmp = traits_type::compare(data(), rhs.data(), count);
        if (cmp != 0) return cmp;
        return s1 < s2 ? -1 : s1 > s2 ?  1 : 0;
    }

    template <size_t M>
    CEX_CXX17 bool operator==(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) == 0; }
#if __cplusplus < 202002L
    template <size_t M>
    CEX_CXX17 bool operator!=(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) != 0; }
    template <size_t M>
    CEX_CXX17 bool operator<(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) < 0; }
    template <size_t M>
    CEX_CXX17 bool operator>(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) > 0; }
    template <size_t M>
    CEX_CXX17 bool operator<=(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) <= 0; }
    template <size_t M>
    CEX_CXX17 bool operator>=(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept { return compare(rhs) >= 0; }
#else
    template <size_t M>
    constexpr auto operator<=>(const basic_tmpl_string<M, value_type, traits_type>& rhs) const noexcept {
        return static_cast<traits_type::comparison_category>(compare(rhs) <=> 0);
    }
#endif
};

template <typename CharT, typename Traits, size_t N>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const basic_tmpl_string<N, CharT, Traits>& ts) {
    return detail::ostream_insert(os, ts.data(), ts.size());
}

#if defined(__GNUG__) && !defined(__clang__)
namespace tmpl_string_literals {
    template <typename CharT, CharT... Chars>
    constexpr basic_tmpl_string<(sizeof...(Chars) + 1), CharT>
    operator""_ts() noexcept { return {Chars...}; }
}
#endif

template <size_t N> using tmpl_string  = basic_tmpl_string<N, char>;
template <size_t N> using tmpl_wstring = basic_tmpl_string<N, wchar_t>;
#ifdef __cpp_lib_char8_t
template <size_t N> using tmpl_u8string = basic_tmpl_string<N, char8_t>;
#endif
template <size_t N> using tmpl_u16string = basic_tmpl_string<N, char16_t>;
template <size_t N> using tmpl_u32string = basic_tmpl_string<N, char32_t>;

#endif // TMPL_STRING_HPP