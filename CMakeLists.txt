CMAKE_MINIMUM_REQUIRED(VERSION 3.14 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RELEASE")
ENDIF()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

find_package(Git)

include(C:/vcpkg/scripts/buildsystems/vcpkg.cmake)

execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")

project(chiapos C CXX ASM)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

set(BUILD_BLS_PYTHON_BINDINGS 0 CACHE INTEGER "")
set(BUILD_BLS_TESTS 0 CACHE INTEGER "")
set(BUILD_BLS_BENCHMARKS 0 CACHE INTEGER "")

# Add path for custom modules
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules
)

# CMake 3.14+
include(FetchContent)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v2.2.1
)
FetchContent_MakeAvailable(cxxopts)

FetchContent_Declare(
  gulrak
  GIT_REPOSITORY https://github.com/gulrak/filesystem.git
  GIT_TAG        v1.5.6
)
FetchContent_MakeAvailable(gulrak)

set(FSE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/FiniteStateEntropy/lib)
set(FSE_FILES
    ${FSE_LIB}/fse_compress.c
    ${FSE_LIB}/fse_decompress.c
    ${FSE_LIB}/entropy_common.c
    ${FSE_LIB}/hist.c
)

include_directories(
  ${INCLUDE_DIRECTORIES}
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include
  ${cxxopts_SOURCE_DIR}/include
  ${gulrak_SOURCE_DIR}/include/ghc
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/FiniteStateEntropy/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/uint128_t
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bls/relic
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bls
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gmp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pthreads
  )

add_library(fse ${FSE_FILES})

add_compile_definitions(BLAKE3_NO_AVX512)

IF (MSVC)
message(STATUS "using MSVC")
IF (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /O3 /Oy /arch:AVX512 -Xclang -O3 -funroll-loops -fomit-frame-pointer -march=native -mavx2 -mavx512f -mavx512dq")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /O3 /Oy /arch:AVX512 -Xclang -O3 -funroll-loops -fomit-frame-pointer -march=native -mavx2 -mavx512f -mavx512dq")
ELSE()
message(STATUS "using MSVC debug")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ob1 /MTd")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ob1 /MTd")
ENDIF()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi -mavx2")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi -mavx2")

ELSE()
IF (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -funroll-loops -fomit-frame-pointer -mavx2")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -funroll-loops -fomit-frame-pointer -mavx2")
ELSE()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
ENDIF()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "ASAN")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "TSAN")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=thread")
ENDIF()

IF (APPLE)
# on macOS "uname -m" returns the architecture (x86_64 or arm64)
execute_process(
    COMMAND uname -m
    RESULT_VARIABLE result
    OUTPUT_VARIABLE OSX_NATIVE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

set(BLS_SRC
    src/bls/bls.cpp
	src/bls/elements.cpp
	src/bls/privatekey.cpp
	src/bls/schemes.cpp
)

set(INT128_SRC
    uint128_t/uint128_t.cpp
)

IF (WIN32)
set(BLAKE3_SRC
    src/b3/blake3.c
    src/b3/blake3_portable.c
    src/b3/blake3_dispatch.c
    src/b3/blake3_avx2.c
    #src/b3/blake3_avx512.c
    src/b3/blake3_sse41.c
)
ELSEIF(OSX_NATIVE_ARCHITECTURE STREQUAL "arm64")
set(BLAKE3_SRC
    src/b3/blake3.c
    src/b3/blake3_portable.c
    src/b3/blake3_dispatch.c
)
ELSE()
set(BLAKE3_SRC
    src/b3/blake3.c
    src/b3/blake3_portable.c
    src/b3/blake3_dispatch.c
    src/b3/blake3_avx2_x86-64_unix.S
    src/b3/blake3_avx512_x86-64_unix.S
    src/b3/blake3_sse41_x86-64_unix.S
)
ENDIF()

add_executable(ProofOfSpace
	${BLS_SRC}
	${INT128_SRC}
    src/cli.cpp
    src/chacha8.c
    ${BLAKE3_SRC}	
)

find_package(Threads REQUIRED)

#add_library(uint128 STATIC uint128_t/uint128_t.cpp)
#target_include_directories(uint128 PUBLIC uint128_t)

target_compile_features(ProofOfSpace PUBLIC cxx_std_17)
target_compile_options(ProofOfSpace PUBLIC /MT)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (MSVC)
  target_link_libraries(ProofOfSpace fse Threads::Threads)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/gmp/gmp.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/relic/relic_s.lib)
else()
  target_link_libraries(ProofOfSpace fse stdc++fs Threads::Threads)
endif()

