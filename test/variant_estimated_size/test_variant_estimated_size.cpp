#define BOOST_TEST_MODULE variant
#include <boost/test/included/unit_test.hpp>

#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/base64.hpp>
#include <string>

using namespace fc;

BOOST_AUTO_TEST_SUITE(variant_estimated_size_suite)
BOOST_AUTO_TEST_CASE(null_variant_estimated_size_test)
{
   nullptr_t np;

   variant v;
   variant v_nullptr(np);

   BOOST_CHECK_EQUAL(v.estimated_size(), 0);
   BOOST_CHECK_EQUAL(v_nullptr.estimated_size(), 0);
}

BOOST_AUTO_TEST_CASE(int64_variant_estimated_size_test)
{
   int64_t i = 1;
   int32_t j = 2;
   int16_t k = 3;
   int8_t l = 4;

   variant v_int_64(i);
   variant v_int_32(j);
   variant v_int_16(k);
   variant v_int_8(l);

   BOOST_CHECK_EQUAL(v_int_64.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_int_32.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_int_16.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_int_8.estimated_size(), 8);
}

BOOST_AUTO_TEST_CASE(uint64_variant_estimated_size_test)
{
   uint64_t i = 1;
   uint32_t j = 2;
   uint16_t k = 3;
   uint8_t l = 4;

   variant v_uint_64(i);
   variant v_uint_32(j);
   variant v_uint_16(k);
   variant v_uint_8(l);

   BOOST_CHECK_EQUAL(v_uint_64.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_uint_32.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_uint_16.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_uint_8.estimated_size(), 8);
}

BOOST_AUTO_TEST_CASE(double_variant_estimated_size_test)
{
   float f = 3.14;
   double d = 12.345;

   variant v_float(f);
   variant v_double(d);

   BOOST_CHECK_EQUAL(v_float.estimated_size(), 8);
   BOOST_CHECK_EQUAL(v_double.estimated_size(), 8);
}

BOOST_AUTO_TEST_CASE(string_variant_estimated_size_test)
{
   char c[] = "Hello World";
   const char* cc = "Goodbye";
   wchar_t wc[] = L"0123456789";
   const wchar_t* cwc = L"foo";
   string s = "abcdefghijklmnopqrstuvwxyz";

   variant v_char(c);
   variant v_const_char(cc);
   variant v_wchar(wc);
   variant v_const_wchar(cwc);
   variant v_string(s);

   BOOST_CHECK_EQUAL(v_char.estimated_size(), 11);
   BOOST_CHECK_EQUAL(v_const_char.estimated_size(), 7);
   BOOST_CHECK_EQUAL(v_wchar.estimated_size(), 10);
   BOOST_CHECK_EQUAL(v_const_wchar.estimated_size(), 3);
   BOOST_CHECK_EQUAL(v_string.estimated_size(), 26);
}

BOOST_AUTO_TEST_CASE(blob_variant_estimated_size_test)
{
   blob bl;
   bl.data.push_back('f');
   bl.data.push_back('o');
   bl.data.push_back('o');

   variant v_blob(bl);

   BOOST_CHECK_EQUAL(v_blob.estimated_size(), 3 + sizeof(size_t));
}

BOOST_AUTO_TEST_CASE(variant_object_variant_estimated_size_test)
{
   string k1 = "key_bool";
   string k2 = "key_string";
   string k3 = "key_int16";
   string k4 = "key_blob";

   bool b = false;
   string s = "HelloWorld";
   int16_t i = 123;
   blob bl;
   bl.data.push_back('b');
   bl.data.push_back('a');
   bl.data.push_back('r');

   variant v_bool(b);
   variant v_string(s);
   variant v_int16(i);
   variant v_blob(bl);

   mutable_variant_object mu;
   mu(k1, b);          // 0  (sum) + 8  (key) + 1    (bool)   = 9
   mu(k2, v_string);   // 9  (sum) + 10 (key) + 10   (string) = 29
   mu(k3, v_int16);    // 29 (sum) + 9  (key) + 8    (int64)  = 46
   mu(k4, bl);         // 46 (sum) + 8  (key) + 7/11 (blob)   = 61/65

   variant_object vo(mu);

   BOOST_CHECK_EQUAL(vo.estimated_size(), 57 + sizeof(size_t));
}

BOOST_AUTO_TEST_CASE(array_variant_estimated_size_test)
{
   bool b = true;
   wchar_t wc[] = L"Goodbye";
   uint32_t i = 54321;

   variant v_bool(b);
   variant v_wchar(wc);
   variant v_uint32(i);

   variants vs;
   vs.push_back(v_bool);
   vs.push_back(v_wchar);
   vs.push_back(v_uint32);

   variant v_variants(vs);
   BOOST_CHECK_EQUAL(v_variants.estimated_size(), 16 + sizeof(size_t));
}

BOOST_AUTO_TEST_SUITE_END()
