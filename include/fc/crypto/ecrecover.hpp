// Snark - Wrapper for alt_bn128 add mul pair and modexp

#pragma once

#include <functional>
#include <cstdint>
#include <utility>
#include <vector>

namespace fc {
    using bytes = std::vector<char>;

    enum class ecrecover_error : int32_t {
        none = 0,
        init_error,
        input_error,
        invalid_signature,
        recover_error,
    };

    std::pair<ecrecover_error, bytes> ecrecover(const bytes& signature, const bytes& digest);
} // fc
