add_library(secp256k1 STATIC
  secp256k1/src/secp256k1.c
)

target_include_directories(secp256k1
    PUBLIC
        secp256k1
        secp256k1/include
    PRIVATE
        secp256k1/src
        config
)

target_compile_definitions(secp256k1 PRIVATE HAVE_CONFIG_H=1)

install( TARGETS secp256k1 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
