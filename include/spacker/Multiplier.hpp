#ifndef SPACKER_MULTIPLIER_HPP
#define SPACKER_MULTIPLIER_HPP

#include <limits>

namespace spacker {

template<uint8_t factor = 4>
struct Multiplier {
    static uint8_t width(uint8_t bits) {
        return factor * (bits + 1);
    }

    template<uint8_t bits>
    static constexpr uint8_t width() {
        return factor * (bits + 1);
    }
    
    static constexpr uint8_t max_bits_per_byte() {
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
    static void update_remaining(uint8_t& remaining, uint8_t has_bit) {
        remaining += factor * has_bit;
    }

    static constexpr uint8_t init_remaining = factor;
};

}

#endif
