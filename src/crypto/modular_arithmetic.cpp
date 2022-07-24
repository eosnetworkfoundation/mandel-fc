#include <fc/crypto/modular_arithmetic.hpp>
#include <tommath.h>
#include <algorithm>
#include <fc/scoped_exit.hpp>

namespace fc {

   std::variant<modular_arithmetic_error, bytes> modexp(const bytes& _base, const bytes& _exponent, const bytes& _modulus)
   {
      if (_modulus.size() == 0) {
         return modular_arithmetic_error::modulus_len_zero;
      }

      auto output = bytes(_modulus.size(), '\0');

      mp_int base, exponent, modulus, result;
      if (mp_init_multi(&base, &exponent, &modulus, &result, nullptr) != MP_OKAY) {
         return modular_arithmetic_error::init;
      }
      auto free_mp = fc::make_scoped_exit([&]() {
         mp_clear_multi(&base, &exponent, &modulus, &result, nullptr);
      });

      if (_base.size()) {
         if (mp_unpack(&base, _base.size(), MP_MSB_FIRST, 1, MP_LITTLE_ENDIAN, 0, _base.data()) != MP_OKAY) {
            return modular_arithmetic_error::unpack;
         }
      }

      if (_exponent.size()) {
         if (mp_unpack(&exponent, _exponent.size(), MP_MSB_FIRST, 1, MP_LITTLE_ENDIAN, 0, _exponent.data()) != MP_OKAY) {
            return modular_arithmetic_error::unpack;
         }
      }

      if (mp_unpack(&modulus, _modulus.size(), MP_MSB_FIRST, 1, MP_LITTLE_ENDIAN, 0, _modulus.data()) != MP_OKAY) {
         return modular_arithmetic_error::unpack;
      }

      if (mp_iszero(&modulus)) {
         return output;
      }

      if (mp_exptmod(&base, &exponent, &modulus, &result) != MP_OKAY) {
         return modular_arithmetic_error::modexp_run;
      }
      // export as little-endian
      size_t written;
      if (mp_pack(output.data(), output.size(), &written, MP_MSB_FIRST, 1, MP_LITTLE_ENDIAN, 0, &result) != MP_OKAY) {
         return modular_arithmetic_error::pack;
      }
      // and convert to big-endian
      std::reverse(output.begin(), output.end());

      return output;
   }

}
