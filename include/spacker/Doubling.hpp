#ifndef SPACKER_DOUBLING_HPP
#define SPACKER_DOUBLING_HPP

#include <limits>

namespace spacker {

template<int base = 1>
struct Doubling {
    static int width(int bits) {
        return (1 << bits) * base;
    }

    template<int bits>
    static constexpr int width() {
        return (1 << bits) * base;
    }
    
    static constexpr int max_bits_per_byte() {
        if constexpr(base > 8) {
            return -1; // a.k.a. cannot fit inside a byte.
        } else {
            if constexpr(base > 4) {
                return 0;
            } else {
                if constexpr(base > 2) {
                    return 1;
                } else {
                    if constexpr(base > 1) {
                        return 2;
                    } else {
                        return 3;
                    }
                }
            }
        }
    }

    // Methods for unpacking.
    static void update_remaining(int& remaining, int has_bit) {
        remaining <<= has_bit;
    }

    static constexpr int init_remaining = base;
};

}

#endif
