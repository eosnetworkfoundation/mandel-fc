#include <fc/io/raw.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/fwd_impl.hpp>
#include <string.h>
#include <cmath>
#include <fc/crypto/sha3.hpp>
#include <fc/variant.hpp>
#include <fc/exception/exception.hpp>
#include "_digest_common.hpp"

extern "C" {
#include "sha3.h"
}

namespace fc {

    sha3::sha3() { memset( _hash, 0, sizeof(_hash) ); }
    sha3::sha3( const char *data, size_t size ) {
       if (size != sizeof(_hash))
         FC_THROW_EXCEPTION( exception, "sha3: size mismatch" );
       memcpy(_hash, data, size );
    }
    sha3::sha3( const string& hex_str ) {
      auto bytes_written = fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );
      if( bytes_written < sizeof(_hash) )
         memset( (char*)_hash + bytes_written, 0, (sizeof(_hash) - bytes_written) );
    }

    string sha3::str()const {
      return fc::to_hex( (char*)_hash, sizeof(_hash) );
    }
    sha3::operator string()const { return  str(); }

    const char* sha3::data()const { return (const char*)&_hash[0]; }
    char* sha3::data() { return (char*)&_hash[0]; }

    struct sha3::encoder::impl {
       sha3_context ctx;
    };

    sha3::encoder::~encoder() {}
    sha3::encoder::encoder(bool keccak) {
      reset(keccak);
    }

    sha3 sha3::hash( const char* d, uint32_t dlen, bool keccak) {
      encoder e{keccak};
      e.write(d,dlen);
      return e.result();
    }

    sha3 sha3::hash( const string& s, bool keccak) {
      return sha3::hash( s.c_str(), s.size(), keccak);
    }

    sha3 sha3::hash( const sha3& s, bool keccak )
    {
        return hash( s.data(), sizeof( s._hash ), keccak );
    }

    void sha3::encoder::write( const char* d, uint32_t dlen ) {
      sha3_Update( &my->ctx, d, dlen);
    }
    sha3 sha3::encoder::result() {
      sha3 h;
      const void* hash = sha3_Finalize(&my->ctx);
      memcpy(h._hash, hash, sizeof(h._hash));
      return h;
    }
    void sha3::encoder::reset(bool keccak=false) {
      sha3_Init256( &my->ctx);
      if(keccak) sha3_SetFlags(&my->ctx, SHA3_FLAGS_KECCAK);
    }

    sha3 operator << ( const sha3& h1, uint32_t i ) {
      sha3 result;
      fc::detail::shift_l( h1.data(), result.data(), result.data_size(), i );
      return result;
    }
    sha3 operator >> ( const sha3& h1, uint32_t i ) {
      sha3 result;
      fc::detail::shift_r( h1.data(), result.data(), result.data_size(), i );
      return result;
    }
    sha3 operator ^ ( const sha3& h1, const sha3& h2 ) {
      sha3 result;
      result._hash[0] = h1._hash[0] ^ h2._hash[0];
      result._hash[1] = h1._hash[1] ^ h2._hash[1];
      result._hash[2] = h1._hash[2] ^ h2._hash[2];
      result._hash[3] = h1._hash[3] ^ h2._hash[3];
      return result;
    }
    bool operator >= ( const sha3& h1, const sha3& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
    }
    bool operator > ( const sha3& h1, const sha3& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
    }
    bool operator < ( const sha3& h1, const sha3& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
    }
    bool operator != ( const sha3& h1, const sha3& h2 ) {
       return !(h1 == h2);
    }
    bool operator == ( const sha3& h1, const sha3& h2 ) {
       // idea to not use memcmp, from:
       //   https://lemire.me/blog/2018/08/22/avoid-lexicographical-comparisons-when-testing-for-string-equality/
       return
             h1._hash[0] == h2._hash[0] &&
             h1._hash[1] == h2._hash[1] &&
             h1._hash[2] == h2._hash[2] &&
             h1._hash[3] == h2._hash[3];
    }

  void to_variant( const sha3& bi, variant& v )
  {
     v = std::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) );
  }

  void from_variant( const variant& v, sha3& bi )
  {
    std::vector<char> ve = v.as< std::vector<char> >();
    if( ve.size() )
    {
        memcpy(&bi, ve.data(), fc::min<size_t>(ve.size(),sizeof(bi)) );
    }
    else
        memset( &bi, char(0), sizeof(bi) );
  }

} //end namespace fc
