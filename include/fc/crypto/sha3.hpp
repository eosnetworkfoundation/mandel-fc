#pragma once
#include <fc/fwd.hpp>
#include <fc/string.hpp>
#include <fc/platform_independence.hpp>
#include <fc/io/raw_fwd.hpp>

namespace fc
{


class sha3
{
  public:
    sha3();
    explicit sha3( const string& hex_str );
    explicit sha3( const char *data, size_t size );

    string str()const;
    operator string()const;

    const char* data()const;
    char*       data();
    size_t      data_size() const { return 256 / 8; }

    static sha3 hash( const char* d, uint32_t dlen, bool keccak );
    static sha3 hash( const string&, bool keccak );
    static sha3 hash( const sha3&, bool keccak );

    template<typename T>
    static sha3 hash( const T& t, bool keccak )
    {
      sha3::encoder e{keccak};
      fc::raw::pack(e,t);
      return e.result();
    }

    class encoder
    {
      public:
        encoder(bool keccak);
        ~encoder();

        void write( const char* d, uint32_t dlen );
        void put( char c ) { write( &c, 1 ); }
        void reset(bool keccak);
        sha3 result();

      private:
        struct      impl;
        fc::fwd<impl,224> my;
    };

    template<typename T>
    inline friend T& operator<<( T& ds, const sha3& ep ) {
      ds.write( ep.data(), sizeof(ep) );
      return ds;
    }

    template<typename T>
    inline friend T& operator>>( T& ds, sha3& ep ) {
      ds.read( ep.data(), sizeof(ep) );
      return ds;
    }
    friend sha3   operator << ( const sha3& h1, uint32_t i     );
    friend sha3   operator >> ( const sha3& h1, uint32_t i     );
    friend bool   operator == ( const sha3& h1, const sha3& h2 );
    friend bool   operator != ( const sha3& h1, const sha3& h2 );
    friend sha3   operator ^  ( const sha3& h1, const sha3& h2 );
    friend bool   operator >= ( const sha3& h1, const sha3& h2 );
    friend bool   operator >  ( const sha3& h1, const sha3& h2 );
    friend bool   operator <  ( const sha3& h1, const sha3& h2 );

    uint64_t _hash[4];
};

  class variant;

  void to_variant( const sha3& bi, variant& v );

  void from_variant( const variant& v, sha3& bi );

  uint64_t hash64(const char* buf, size_t len);

} // fc

namespace std
{
    template<>
    struct hash<fc::sha3>
    {
       size_t operator()( const fc::sha3& s )const
       {
           return  *((size_t*)&s);
       }
    };

}

namespace boost
{
    template<>
    struct hash<fc::sha3>
    {
       size_t operator()( const fc::sha3& s )const
       {
           return  s._hash[3];//*((size_t*)&s);
       }
    };
}
#include <fc/reflect/reflect.hpp>
FC_REFLECT_TYPENAME( fc::sha3 )
