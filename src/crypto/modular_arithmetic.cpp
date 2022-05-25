#include <gmp.h>
#include <fc/crypto/modular_arithmetic.hpp>
#include <algorithm>

namespace fc {

    std::pair<modular_arithmetic_error, bytes> modexp(const bytes& _base, const bytes& _exponent, const bytes& _modulus)
    {
        if (_modulus.size() == 0) {
            return std::make_pair(modular_arithmetic_error::modulus_len_zero, bytes{});
        }

        auto output = bytes(_modulus.size(), '\0');

        mpz_t base;
        mpz_init(base);
        if (_base.size()) {
            mpz_import(base, _base.size(), 1, 1, 0, 0, _base.data());
        }

        mpz_t exponent;
        mpz_init(exponent);
        if (_exponent.size()) {
            mpz_import(exponent, _exponent.size(), 1, 1, 0, 0, _exponent.data());
        }

        mpz_t modulus;
        mpz_init(modulus);
        mpz_import(modulus, _modulus.size(), 1, 1, 0, 0, _modulus.data());

        if (mpz_sgn(modulus) == 0) {
            mpz_clear(modulus);
            mpz_clear(exponent);
            mpz_clear(base);

            return std::make_pair(modular_arithmetic_error::none, output);
        }

        mpz_t result;
        mpz_init(result);

        mpz_powm(result, base, exponent, modulus);
        // export as little-endian
        mpz_export(output.data(), nullptr, -1, 1, 0, 0, result);
        // and convert to big-endian
        std::reverse(output.begin(), output.end());

        mpz_clear(result);
        mpz_clear(modulus);
        mpz_clear(exponent);
        mpz_clear(base);

        return std::make_pair(modular_arithmetic_error::none, output);
    }

}
