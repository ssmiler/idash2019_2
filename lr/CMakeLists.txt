cmake_minimum_required(VERSION 3.0)

project(idash_2019 VERSION 0.1.0 LANGUAGES CXX)

add_executable(encrypt encrypt.cpp)
# target_link_libraries(encrypt)

add_executable(decrypt decrypt.cpp)
# target_link_libraries(decrypt)

add_executable(compute compute.cpp parse_vw.cpp)
# target_link_libraries(compute)

