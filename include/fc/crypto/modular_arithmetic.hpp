#pragma once

#include <cstdint>
#include <vector>

namespace fc {
    using bytes = std::vector<char>;

    enum class modular_arithmetic_error : int32_t {
        none = 0,
        modulus_len_zero,
    };

    std::pair<modular_arithmetic_error, bytes> modexp(const bytes& _base, const bytes& _exponent, const bytes& _modulus);
}
