cmake_minimum_required(VERSION 3.0)

project(idash_2019 VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(./lib/tfhe)
include_directories(./lib/tfhe/include)

set(LIB_SRC
        parse_vw.h
        parse_vw.cpp idash.cpp idash.h)

add_executable(encrypt ${LIB_SRC} encrypt.cpp)
target_link_libraries(encrypt tfhe-spqlios-fma)
# target_link_libraries(encrypt)

add_executable(decrypt ${LIB_SRC} decrypt.cpp)
target_link_libraries(decrypt tfhe-spqlios-fma)
# target_link_libraries(decrypt)

add_executable(validation ${LIB_SRC} validation.cpp)
target_link_libraries(validation tfhe-spqlios-fma)
# target_link_libraries(validation)

