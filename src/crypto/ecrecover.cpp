#include <fc/crypto/ecrecover.hpp>

#include <secp256k1.h>
#include <secp256k1_recovery.h>

namespace fc {

    const secp256k1_context* ecrecover_context() {
        static secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
        return ctx;
    }

    std::pair<ecrecover_error, bytes> ecrecover(const bytes& signature, const bytes& digest) {
        const secp256k1_context* context{ecrecover_context()};
        if (!context) {
            return std::make_pair(ecrecover_error::init_error, bytes{});
        }

        if (signature.size() != 65 || digest.size() != 32) {
            return std::make_pair(ecrecover_error::input_error, bytes{});
        }

        int recid = signature[0];
        if (recid<27 || recid>=35) return std::make_pair(ecrecover_error::invalid_signature, bytes{});
        recid = (recid - 27) & 3;

        secp256k1_ecdsa_recoverable_signature sig;
        if (!secp256k1_ecdsa_recoverable_signature_parse_compact(context, &sig, (const unsigned char*)&signature[1], recid)) {
            return std::make_pair(ecrecover_error::invalid_signature, bytes{});
        }

        secp256k1_pubkey pub_key;
        if (!secp256k1_ecdsa_recover(context, &pub_key, &sig, (const unsigned char*)&digest[0])) {
            return std::make_pair(ecrecover_error::recover_error, bytes{});
        }

        size_t kOutLen{65};
        bytes out(kOutLen, '\0');
        secp256k1_ec_pubkey_serialize(context, (unsigned char*)&out[0], &kOutLen, &pub_key, SECP256K1_EC_UNCOMPRESSED);
        return std::make_pair(ecrecover_error::none, out);
    }
}
