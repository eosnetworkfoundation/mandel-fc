#define BOOST_TEST_MODULE modular_arithmetic
#include <boost/test/included/unit_test.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/modular_arithmetic.hpp>
#include <fc/utility.hpp>

using namespace fc;
#include "test_utils.hpp"

namespace std {
std::ostream& operator<<(std::ostream& st, const std::variant<fc::modular_arithmetic_error, bytes>& err)
{
    if(std::holds_alternative<fc::modular_arithmetic_error>(err))
        st << static_cast<int32_t>(std::get<fc::modular_arithmetic_error>(err));
    else
        st << fc::to_hex(std::get<bytes>(err));
    return st;
}
}


BOOST_AUTO_TEST_SUITE(modular_arithmetic)
BOOST_AUTO_TEST_CASE(modexp) try {


    using modexp_test = std::tuple<std::vector<string>, std::variant<fc::modular_arithmetic_error, bytes>>;

    const std::vector<modexp_test> tests {
        //test1
        {
            {
                "03",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",
            },
            to_bytes("0000000000000000000000000000000000000000000000000000000000000001"),
        },

        //test2
        {
            {
                "",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",
            },
            to_bytes("0000000000000000000000000000000000000000000000000000000000000000")
        },

        //test3
        {
            {
                "01",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "",
            },
            modular_arithmetic_error::modulus_len_zero
        },


    };

    for(const auto& test : tests) {
        const auto& parts           = std::get<0>(test);
        const auto& expected_result = std::get<1>(test);

        auto base = to_bytes(parts[0]);
        auto exponent = to_bytes(parts[1]);
        auto modulus = to_bytes(parts[2]);

        auto res = fc::modexp(base, exponent, modulus);
        BOOST_CHECK_EQUAL(res, expected_result);
    }

} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()