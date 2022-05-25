add_executable( test_cypher_suites test_cypher_suites.cpp )
target_link_libraries( test_cypher_suites fc )

add_executable( test_webauthn test_webauthn.cpp )
target_link_libraries( test_webauthn fc )

add_executable( test_hash_functions test_hash_functions.cpp )
target_link_libraries( test_hash_functions fc )

add_executable( test_alt_bn128 test_alt_bn128.cpp )
target_link_libraries( test_alt_bn128 fc )

add_test(NAME test_cypher_suites COMMAND libraries/fc/test/crypto/test_cypher_suites WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME test_webauthn COMMAND libraries/fc/test/crypto/test_webauthn WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME test_hash_functions COMMAND libraries/fc/test/crypto/test_hash_functions WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME test_alt_bn128 COMMAND libraries/fc/test/crypto/test_alt_bn128 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})