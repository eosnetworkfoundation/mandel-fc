// Snark - Wrapper for alt_bn128 add mul pair and modexp

#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace fc {
    using bytes = std::vector<char>;

    enum class ecrecover_error : int32_t {
        init_error,
        input_error,
        invalid_signature,
        recover_error,
    };

    std::variant<ecrecover_error, bytes> ecrecover(const bytes& signature, const bytes& digest);
} // fc
