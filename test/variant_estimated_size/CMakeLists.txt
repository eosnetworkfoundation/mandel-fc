add_executable( variant_estimated_size test_variant_estimated_size.cpp )
target_link_libraries( variant_estimated_size fc )

add_test(NAME test_variant COMMAND libraries/fc/test/variant_estimated_size/test_variant_estimated_size WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
