#include "Rcpp.h"
#include "spacker/pack_psip.hpp"
#include "spacker/unpack_psip.hpp"
#include "spacker/Doubling.hpp"
#include "spacker/Multiplier.hpp"

#include <cstdint>
#include <algorithm>

// [[Rcpp::export(rng=false)]]
Rcpp::RawVector doubling_packer(Rcpp::IntegerVector x) {
    std::vector<uint32_t> input(x.size());
    std::copy(x.begin(), x.end(), input.data());
    std::vector<uint8_t> output = spacker::pack_psip(input.size(), input.data());
    return Rcpp::RawVector(output.begin(), output.end());
}

// [[Rcpp::export(rng=false)]]
Rcpp::IntegerVector doubling_unpacker(Rcpp::RawVector x, int n) {
    std::vector<uint32_t> output(n);
    spacker::unpack_psip(x.size(), (const uint8_t*)x.begin(), n, output.data());
    return Rcpp::IntegerVector(output.begin(), output.end());
}

// [[Rcpp::export(rng=false)]]
Rcpp::RawVector multiplier_packer(Rcpp::IntegerVector x) {
    std::vector<uint32_t> input(x.size());
    std::copy(x.begin(), x.end(), input.data());
    std::vector<uint8_t> output = spacker::pack_psip<true, spacker::Multiplier<>>(input.size(), input.data());
    return Rcpp::RawVector(output.begin(), output.end());
}

// [[Rcpp::export(rng=false)]]
Rcpp::IntegerVector multiplier_unpacker(Rcpp::RawVector x, int n) {
    std::vector<uint32_t> output(n);
    spacker::unpack_psip<spacker::Multiplier<>>(x.size(), (const uint8_t*)x.begin(), n, output.data());
    return Rcpp::IntegerVector(output.begin(), output.end());
}

// [[Rcpp::export(rng=false)]]
Rcpp::RawVector encode_short(Rcpp::IntegerVector x) {
    Rcpp::RawVector output(x.size() * 2);
    auto oIt = output.begin();
    for (auto i : x) {
        uint16_t current = i;
        *oIt = (current >> 8);
        ++oIt;
        *oIt = (current & 0b11111111);
        ++oIt;
    }
    return output;
}
