#define BOOST_TEST_MODULE modular_arithmetic
#include <boost/test/included/unit_test.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/modular_arithmetic.hpp>
#include <fc/utility.hpp>

using namespace fc;
#include "test_utils.hpp"

namespace std {
std::ostream& operator<<(std::ostream& st, modular_arithmetic_error err)
{
    st << static_cast<int32_t>(err);
    return st;
}
}


BOOST_AUTO_TEST_SUITE(modular_arithmetic)
BOOST_AUTO_TEST_CASE(modexp) try {


    using modexp_test = std::tuple<std::vector<string>, fc::modular_arithmetic_error, std::string>;

    const std::vector<modexp_test> tests {
        //test1
        {
            {
                "03",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",
            },
            modular_arithmetic_error::none,
            "0000000000000000000000000000000000000000000000000000000000000001",
        },

        //test2
        {
            {
                "",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",
            },
            modular_arithmetic_error::none,
            "0000000000000000000000000000000000000000000000000000000000000000",
        },

        //test3
        {
            {
                "01",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "",
            },
            modular_arithmetic_error::modulus_len_zero,
            "",
        },


    };

    for(const auto& test : tests) {
        const auto& parts           = std::get<0>(test);
        const auto& expected_error  = std::get<1>(test);
        const auto& expected_result = std::get<2>(test);

        auto base = to_bytes(parts[0]);
        auto exponent = to_bytes(parts[1]);
        auto modulus = to_bytes(parts[2]);

        auto res = fc::modexp(base, exponent, modulus);
        BOOST_CHECK_EQUAL(res.first, expected_error);
        BOOST_CHECK_EQUAL(fc::to_hex(res.second.data(), res.second.size()), expected_result);
    }

} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()