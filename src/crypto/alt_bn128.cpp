#include <gmp.h>
#include <fc/crypto/alt_bn128.hpp>

#include <libff/algebra/curves/alt_bn128/alt_bn128_g1.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_g2.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pairing.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>

#include <libff/common/profiling.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>

namespace fc {

    using Scalar = libff::bigint<libff::alt_bn128_q_limbs>;

    void initLibSnark() noexcept {
        static bool s_initialized = []() noexcept {
            libff::inhibit_profiling_info = true;
            libff::inhibit_profiling_counters = true;
            libff::alt_bn128_pp::init_public_params();
            return true; 
        }();
        (void)s_initialized;
    }

    Scalar to_scalar(const bytes& be) noexcept {
        mpz_t m;
        mpz_init(m);
        mpz_import(m, be.size(), /*order=*/1, /*size=*/1, /*endian=*/0, /*nails=*/0, &be[0]);
        Scalar out{m};
        mpz_clear(m);
        return out;
    }

    // Notation warning: Yellow Paper's p is the same libff's q.
    // Returns x < p (YP notation).
    static bool valid_element_of_fp(const Scalar& x) noexcept {
        return mpn_cmp(x.data, libff::alt_bn128_modulus_q.data, libff::alt_bn128_q_limbs) < 0;
    }

    std::pair<alt_bn128_error, libff::alt_bn128_G1> decode_g1_element(const bytes& bytes64_be) noexcept {
        if(bytes64_be.size() != 64) {
            return std::make_pair(alt_bn128_error::input_len_error, libff::alt_bn128_G1::zero());
        }
    
        bytes sub1(bytes64_be.begin(), bytes64_be.begin()+32);
        bytes sub2(bytes64_be.begin()+32, bytes64_be.begin()+64);

        Scalar x{to_scalar(sub1)};
        if (!valid_element_of_fp(x)) {
            return std::make_pair(alt_bn128_error::operand_component_invalid, libff::alt_bn128_G1::zero());
        }

        Scalar y{to_scalar(sub2)};
        if (!valid_element_of_fp(y)) {
            return std::make_pair(alt_bn128_error::operand_component_invalid, libff::alt_bn128_G1::zero());
        }

        if (x.is_zero() && y.is_zero()) {
            return std::make_pair(alt_bn128_error::operand_at_origin, libff::alt_bn128_G1::zero());
        }

        libff::alt_bn128_G1 point{x, y, libff::alt_bn128_Fq::one()};
        if (!point.is_well_formed()) {
            return std::make_pair(alt_bn128_error::operand_not_in_curve, libff::alt_bn128_G1::zero());
        }
        return std::make_pair(alt_bn128_error::none, point);
    }

    std::pair<alt_bn128_error, libff::alt_bn128_Fq2> decode_fp2_element(const bytes& bytes64_be) noexcept {
        if(bytes64_be.size() != 64) {
            return std::make_pair(alt_bn128_error::input_len_error, libff::alt_bn128_Fq2{});
        }

        // big-endian encoding
        bytes sub1(bytes64_be.begin()+32, bytes64_be.begin()+64);
        bytes sub2(bytes64_be.begin(), bytes64_be.begin()+32);        

        Scalar c0{to_scalar(sub1)};
        Scalar c1{to_scalar(sub2)};

        if (!valid_element_of_fp(c0) || !valid_element_of_fp(c1)) {
            return std::make_pair(alt_bn128_error::operand_component_invalid, libff::alt_bn128_Fq2::one() );
        }

        return std::make_pair(alt_bn128_error::none, libff::alt_bn128_Fq2{c0, c1});
    }

    std::pair<alt_bn128_error, libff::alt_bn128_G2> decode_g2_element(const bytes& bytes128_be) noexcept {
        assert(bytes128_be.size() == 128);

        bytes sub1(bytes128_be.begin(), bytes128_be.begin()+64);        
        auto x = decode_fp2_element(sub1);
        if (x.first != alt_bn128_error::none) {
            return std::make_pair(x.first, libff::alt_bn128_G2::zero());
        }

        bytes sub2(bytes128_be.begin()+64, bytes128_be.begin()+128);        
        auto y = decode_fp2_element(sub2);
        
        if (y.first != alt_bn128_error::none) {
            return std::make_pair(y.first, libff::alt_bn128_G2::zero());
        }

        if (x.second.is_zero() && y.second.is_zero()) {
            return std::make_pair(alt_bn128_error::operand_at_origin, libff::alt_bn128_G2::zero());
        }

        libff::alt_bn128_G2 point{x.second, y.second, libff::alt_bn128_Fq2::one()};
        if (!point.is_well_formed()) {
            return std::make_pair(alt_bn128_error::operand_not_in_curve, libff::alt_bn128_G2::zero());;
        }

        if (!(libff::alt_bn128_G2::order() * point).is_zero()) {
            // wrong order, doesn't belong to the subgroup G2
            return std::make_pair(alt_bn128_error::operand_outside_g2, libff::alt_bn128_G2::zero());;
        }

        return std::make_pair(alt_bn128_error::none, point);
    }

    bytes encode_g1_element(libff::alt_bn128_G1 p) noexcept {
        bytes out(64, '\0');
        if (p.is_zero()) {
            return out;
        }

        p.to_affine_coordinates();

        auto x{p.X.as_bigint()};
        auto y{p.Y.as_bigint()};

        std::memcpy(&out[0], y.data, 32);
        std::memcpy(&out[32], x.data, 32);

        std::reverse(out.begin(), out.end());
        return out;
    }

    std::pair<alt_bn128_error, bytes> alt_bn128_add(const bytes& op1, const bytes& op2) {
        fc::initLibSnark();

        auto x = decode_g1_element(op1);

        if (x.first != alt_bn128_error::none) {
            return std::make_pair(x.first, bytes{});
        }

        auto y = decode_g1_element(op2);

        if (y.first != alt_bn128_error::none) {
            return std::make_pair(y.first, bytes{});
        }

        libff::alt_bn128_G1 g1Sum = x.second + y.second;
        auto retEncoded = encode_g1_element(g1Sum);
        return std::make_pair(alt_bn128_error::none, retEncoded);
    }

    std::pair<alt_bn128_error, bytes> alt_bn128_mul(const bytes& g1_point, const bytes& scalar) {
        initLibSnark();

        auto x = decode_g1_element(g1_point);

        if (x.first != alt_bn128_error::none) {
            return std::make_pair(x.first, bytes{});
        }

        if(scalar.size() != 32) {
            return std::make_pair(alt_bn128_error::invalid_scalar_size, bytes{});
        }

        Scalar n{to_scalar(scalar)};

        libff::alt_bn128_G1 g1Product = n * x.second;
        auto retEncoded = encode_g1_element(g1Product);
        return std::make_pair(alt_bn128_error::none, retEncoded);
    }
    
    static constexpr size_t kSnarkvStride{192};

    std::pair<alt_bn128_error, bool>  alt_bn128_pair(const bytes& g1_g2_pairs) {
        if (g1_g2_pairs.size() % kSnarkvStride != 0) {
            return std::make_pair(alt_bn128_error::pairing_list_size_error, false);
        }

        size_t k{g1_g2_pairs.size() / kSnarkvStride};

        initLibSnark();
        using namespace libff;

        static const auto one{alt_bn128_Fq12::one()};
        auto accumulator{one};

        for (size_t i{0}; i < k; ++i) {
            auto offset = i * kSnarkvStride;
            bytes sub1(g1_g2_pairs.begin()+offset, g1_g2_pairs.begin()+offset+64);        
            auto a = decode_g1_element(sub1);
            if (a.first != alt_bn128_error::none) {
                return std::make_pair(a.first, false);
            }
            bytes sub2(g1_g2_pairs.begin()+offset+64, g1_g2_pairs.begin()+offset+64+128);        
            auto b = decode_g2_element(sub2);
            if (b.first != alt_bn128_error::none) {
                return std::make_pair(b.first, false);
            }

            if (a.second.is_zero() || b.second.is_zero()) {
                continue;
            }

            accumulator = accumulator * alt_bn128_miller_loop(alt_bn128_precompute_G1(a.second), alt_bn128_precompute_G2(b.second));
        }

        bool pair_result = false;
        if (alt_bn128_final_exponentiation(accumulator) == one) {
            pair_result = true;
        }

        return std::make_pair(alt_bn128_error::none, pair_result);
    }
}
