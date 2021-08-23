#ifndef SPACKER_MULTIPLIER_HPP
#define SPACKER_MULTIPLIER_HPP

#include <limits>

namespace spacker {

template<int factor = 4>
struct Multiplier {
    static int width(int bits) {
        return factor * (bits + 1);
    }

    template<int bits>
    static constexpr int width() {
        return factor * (bits + 1);
    }
    
    static constexpr int max_bits_per_byte() {
        if constexpr(factor > 8) {
            return -1; // cannot fit inside a byte. 
        } else {
            if constexpr(factor > 4) {
                return 0;
            } else {
                if constexpr(factor > 2) {
                    return 1;
                } else {
                    if constexpr(factor > 1) {
                        return 4;
                    } else {
                        return 8;
                    }
                }
            }
        }
    }

    // Methods for unpacking.
    static void update_remaining(int& remaining, int has_bit) {
        remaining += factor * has_bit;
    }

    static constexpr int init_remaining = factor;
};

}

#endif
