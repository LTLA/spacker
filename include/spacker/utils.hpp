#ifndef SPACKER_UTILS_HPP
#define SPACKER_UTILS_HPP

namespace spacker {

template<typename T, class Scheme, int bits>
inline constexpr bool supports() {
    static_assert(bits <= 7);
    return Scheme::template width<bits>() <= std::numeric_limits<T>::digits;
}

template<typename T, class Scheme, int bits>
inline constexpr T max() {
    static_assert(supports<T, Scheme, bits>());
    constexpr int available = Scheme::template width<bits>() - bits - 1; // the 1 comes from the extra zero at the end of the preamble.
    constexpr T space = (static_cast<T>(1) << available); 
    if constexpr(bits == 0) {
        return space;
    } else {
        constexpr T previous = max<T, Scheme, bits - 1>();
        return previous + space;
    }
}

template<typename T, class Scheme, int bits>
inline constexpr T min() {
    if constexpr(bits == 0) {
        return 1;
    } else {
        return max<T, Scheme, bits - 1>() + 1;
    }
}

}

#endif
