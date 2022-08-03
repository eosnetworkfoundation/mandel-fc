#define BOOST_TEST_MODULE modular_arithmetic
#include <boost/test/included/unit_test.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/modular_arithmetic.hpp>
#include <fc/utility.hpp>

#include <chrono>
#include <random>
#include <limits>

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

        //test4
        {
            {
                "01",
                "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2e",
                "0000",
            },
            to_bytes("0000")
        },

        //test5
        {
            {
                "00",
                "00",
                "0F",
            },
            to_bytes("01"),
        },

        //test6
        {
            {
                "00",
                "01",
                "0F",
            },
            to_bytes("00"),
        },

        //test7
        {
            {
                "01",
                "00",
                "0F",
            },
            to_bytes("01"),
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

BOOST_AUTO_TEST_CASE(modexp_benchmarking) try {

    std::mt19937 r(0x11223344);

    auto generate_random_bytes = [](std::mt19937& rand_eng, unsigned int num_bytes) {
        std::vector<char> result(num_bytes);

        uint_fast32_t v = 0;
        for(int byte_pos = 0, end = result.size(); byte_pos < end; ++byte_pos) {
            if ((byte_pos & 0x03) == 0) { // if divisible by 4
                v = rand_eng();
            }
            result[byte_pos] = v & 0xFF;
            v >>= 8;
        }

        return result;
    };

    static constexpr unsigned int num_trials = 100; // 10000

    static_assert(num_trials > 0);

    static constexpr unsigned int start_num_bytes = 128; // 64
    static constexpr unsigned int end_num_bytes   = 256; // 512
    static constexpr unsigned int delta_num_bytes = 128; // 64

    static_assert(start_num_bytes <= end_num_bytes);
    static_assert(delta_num_bytes > 0);
    static_assert((end_num_bytes - start_num_bytes) % delta_num_bytes == 0);

    static constexpr unsigned num_slots = (end_num_bytes - start_num_bytes) / delta_num_bytes + 1;

    struct statistics {
        int64_t min_time_ns;
        int64_t max_time_ns;
        int64_t avg_time_ns;
    };

    std::vector<statistics> stats(num_slots);

    for (unsigned int n = start_num_bytes, slot = 0; n <= end_num_bytes; n += delta_num_bytes, ++slot) {
        int64_t min_duration_ns = std::numeric_limits<int64_t>::max();
        int64_t max_duration_ns = 0;
        int64_t total_duration_ns = 0;

        for (unsigned int trial = 0; trial < num_trials; ++trial) {
            auto base     = generate_random_bytes(r, n);
            auto exponent = generate_random_bytes(r, n);
            auto modulus  = generate_random_bytes(r, n);

            auto start_time = std::chrono::steady_clock::now();

            auto res = fc::modexp(base, exponent, modulus);

            auto end_time = std::chrono::steady_clock::now();

            int64_t duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

            //ilog("(${base})^(${exp}) % ${mod} = ${result}", 
            //     ("base", base)("exp", exponent)("mod", modulus)("result", std::get<bytes>(res))
            //    );

            //ilog("slot ${slot}: mod_exp took ${duration} ns", ("slot", slot)("duration", duration_ns));

            min_duration_ns = std::min(min_duration_ns, duration_ns);
            max_duration_ns = std::max(max_duration_ns, duration_ns);
            total_duration_ns += duration_ns;
        }

        stats[slot] = statistics{
            .min_time_ns = min_duration_ns,
            .max_time_ns = max_duration_ns,
            .avg_time_ns = (total_duration_ns / num_trials),
        };

        ilog("Completed random runs of mod_exp with ${bit_width}-bit width values. Min time: ${min} ns; Average time: ${avg} ns; Max time: ${max} ns.",
             ("bit_width", n*8)("min", stats[slot].min_time_ns)("avg", stats[slot].avg_time_ns)("max", stats[slot].max_time_ns)
            );
    }

    // Running the above benchmark (using commented values for num_trials and *_num_bytes) with a release build on an AMD 3.4 GHz CPU
    // provides average durations for executing mod_exp for increasing bit sizes for the value.

    // For example: with 512-bit values, the average duration is approximately 40 microseconds; with 1024-bit values, the average duration
    // is approximately 260 microseconds; with 2048-bit values, the average duration is approximately 2 milliseconds; and, with 4096-bit 
    // values, the average duration is approximately 14 milliseconds.

    // It appears that a model of the average time that scales quadratically with the bit size fits the empirically generated data well.
    // TODO: See if theoretical analysis of the modular exponentiation algorithm also justifies quadratic scaling.

} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()