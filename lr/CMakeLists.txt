cmake_minimum_required(VERSION 3.0)

project(idash_2019 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-std=gnu++11 -g3 -O0 -Wall -Werror")
set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Wall -Werror")

set(CMAKE_CXX_FLAGS_OPTIM "-std=gnu++11 -g3 -O9 -DNDEBUG -Wall -Werror")
set(CMAKE_C_FLAGS_OPTIM "-g3 -O9 -Wall -Werror -DNDEBUG")

add_subdirectory(./lib/tfhe EXCLUDE_FROM_ALL)
include_directories(./lib/tfhe/include)

set(LIB_SRC
        parse_vw.h
        parse_vw.cpp idash.cpp idash.h)

set(PROG_SRC
        keygen.cpp
        encrypt.cpp
        cloud.cpp
        decrypt.cpp
        validation.cpp
        validation2.cpp
        plaintext_algo.cpp)

if (APPLE)
    #set(LIBTFHE tfhe-spqlios-fma)
    set(LIBTFHE tfhe-nayuki-portable)
else ()
    #set(LIBTFHE tfhe-nayuki-portable)
    set(LIBTFHE tfhe-spqlios-fma)
endif (APPLE)


foreach (PROG_SOURCE ${PROG_SRC})
    get_filename_component(PROG_NAME ${PROG_SOURCE} NAME_WE)
    add_executable(${PROG_NAME} ${PROG_SOURCE} ${LIB_SRC})
    target_link_libraries(${PROG_NAME} ${LIBTFHE})
endforeach (PROG_SOURCE)
