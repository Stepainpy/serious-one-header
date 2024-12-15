/*
Pseudo-random number generator based on
algorithm xorshift* with 64-bit result,
C++11 and later
*/

#ifndef XORSHIFT_PRNG_HPP
#define XORSHIFT_PRNG_HPP

#include <cstdint>
#include <ostream>
#include <istream>

class xorshift {
public:
    using result_type = uint_fast64_t;
    static constexpr result_type default_seed = 1;

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT_FAST64_MAX; }

    xorshift() : m_state(default_seed) {}
    xorshift(result_type s) : xorshift() { seed(s); }
    template <class SeedSeq>
    xorshift(SeedSeq& q) : xorshift() { seed(q); }

    void seed() { m_state = default_seed; }
    void seed(result_type s) {
        if (s) m_state = s;
    }
    template <class SeedSeq>
    void seed(SeedSeq& q) {
        uint_least32_t sa[2] {};
        q.generate(sa, sa + 2);
        (m_state <<= 32) |= sa[1] & UINT32_MAX;
        (m_state <<= 32) |= sa[0] & UINT32_MAX;
    }

    result_type operator()() {
        next_state();
        return m_state *
            UINT64_C(2685821657736338717);
    }

    void discard(unsigned long long count) {
        while (count--) next_state();
    }

    bool operator==(const xorshift& rhs) const {
        return m_state == rhs.m_state;
    }
    bool operator!=(const xorshift& rhs) const {
        return !(*this == rhs);
    }

    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const xorshift& xsg) {
        using ios_base = typename std::basic_ostream<CharT, Traits>::ios_base;

        const typename ios_base::fmtflags flags = os.flags();
        const CharT prevfill = os.fill(os.widen(' '));
        os.flags(ios_base::dec | ios_base::left);

        os << xsg.m_state;

        os.flags(flags);
        os.fill(prevfill);
        return os;
    }

    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>&
    operator>>(std::basic_istream<CharT, Traits>& is, xorshift& xsg) {
        using ios_base = typename std::basic_istream<CharT, Traits>::ios_base;
        
        const typename ios_base::fmtflags flags = is.flags();
        is.flags(ios_base::dec);

        is >> xsg.m_state;

        is.flags(flags);
        return is;
    }

private:
    void next_state() {
        m_state ^= m_state >> 12;
        m_state ^= m_state << 25;
        m_state ^= m_state >> 27;
    }

    result_type m_state;
};

#endif // XORSHIFT_PRNG_HPP