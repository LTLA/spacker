#ifndef SPACKER_UNPACK_HPP
#define SPACKER_UNPACK_HPP

#include <cstdint>
#include <vector>
#include <limits>
#include <array>
#include <algorithm>

namespace spacker {

template<typename T>
void unpack(size_t ni, const uint8_t* input, size_t no, T* output) {
    std::array<T, 8> buffer;
    std::fill_n(buffer.data(), buffer.size(), 0);
    std::array<int, 8> bits;
    std::fill_n(bits.data(), bits.size(), 0);

    // A boolean flag indicating whether we're still in the preamble.
    // We use int for easier multiplications below. We start at 1
    // to support the situation where the first entry is a preamble;
    // if it isn't, then it'll get flipped to zero anyway.
    int preamble = 1; 

    // Remaining bits to process in the current integer. 
    int remaining = 0; 

    // Where are we with respect to the buffers?
    int at = 0; 

    for (size_t i = 0; i < ni; ++i, ++input) {
        auto val = *input;

        // Behold the following stack of manually unrolled, no if/else
        // operations.  The only thing that changes in each step is the
        // definition of 'keep', so I'll only annotate the first copy.
        {
            auto keep = (val & 0b10000000) != 0;

            // If preamble = true AND keep = true, we have an extra bit in the
            // preamble, we increase the remaining count. Otherwise neither
            // bits nor remaining will change.
            bits[at] += preamble * keep;
            remaining <<= preamble * keep;

            // If preamble = false, we left-shift the existing buffer element.
            // If additionally keep = true, we set the least significant bit.
            buffer[at] <<= (1 - preamble);
            buffer[at] |= (1 - preamble) * keep;

            // If preamble = false, we subtract a bit. If we're at the end of
            // the preamble (i.e., preamble = true and keep = false), we
            // compute the actual number of remaining bits, by subtracting
            // the bits used in defining the preamble itself.
            remaining -= (1 - preamble) + preamble * (1 - keep) * (bits[at] + 1);

            // If there are no more remaining bits, we must be in the preamble
            // of the next element. In that case, we set remaining = 1 so that
            // later left-shifts have an effect.
            at += (remaining == 0); 
            preamble = (remaining == 0);
            remaining += (remaining == 0);
        }

#define SPACKER_UNROLLED_UNPACKER { \
            bits[at] += preamble * keep; \
            remaining <<= preamble * keep; \
            buffer[at] <<= (1 - preamble); \
            buffer[at] |= (1 - preamble) * keep; \
            remaining -= (1 - preamble) + preamble * (1 - keep) * (bits[at] + 1); \
            at += (remaining == 0); \
            preamble = (remaining == 0); \
            remaining += (remaining == 0); \
}
        {
            auto keep = (val & 0b01000000) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00100000) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00010000) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00001000) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00000100) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00000010) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        {
            auto keep = (val & 0b00000001) != 0;
            SPACKER_UNROLLED_UNPACKER
        }

        // Moving results to the output buffer.
        for (int i = 0; i < at; ++i, ++output) {
            *output = buffer[i] + maxed[bit[i]];
        }

        // Resetting for the next byte.
        std::fill_n(buffer.data(), at, 0);
        std::fill_n(bits.data(), at, 0);
        if (at && at < buffer.size()) {
            std::swap(buffer[0], buffer[at]);
            std::swap(bits[0], bits[at]);
            at = 0;
        }
    }

    return;
}

}

#endif
