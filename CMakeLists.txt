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

# Add path for custom modules
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules
)

# CMake 3.14+
include(FetchContent)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/FiniteStateEntropy/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/uint128_t
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gmp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pthreads
  ${CMAKE_CURRENT_SOURCE_DIR}/src/freetype/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/freetype/include/freetype
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/Include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/linmath
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/mINI/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/nfd/src/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/stb
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/Tbl
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/backends
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Implot/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/glad/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/relic/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/relic/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/relic/include/low
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

add_library(fse ${FSE_FILES})

add_compile_definitions(BLAKE3_NO_AVX512)
add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

IF (MSVC)
message(STATUS "using MSVC")
IF (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /O3 /Oi /Oy /arch:AVX")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /O3 /Oi /Oy /arch:AVX")
ELSE()
message(STATUS "using MSVC debug")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od /MTd")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Od /MTd")
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

set(INT128_SRC
    uint128_t/uint128_t.cpp
)

file(GLOB_RECURSE RELIC_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/relic/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/relic/*.cpp
)

file(GLOB_RECURSE IMGUI_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/*.cpp
)

file(GLOB IMGUI_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/backends/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/backends/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/misc/cpp/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/misc/cpp/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/misc/freetype/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Imgui/misc/freetype/*.cpp
)

file(GLOB IMFRAME_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/ImGui/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/ImGui/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/WinPC/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/WinPC/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/nfd/src/nfd_win.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/stb/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImFrame/stb/*.cpp
)

add_compile_definitions(_GLFW_WIN32=1)
file(GLOB GLFW_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/osmesa_context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/wgl_context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/vulkan.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/egl*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/egl*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/win32*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/win32*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/window.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/init.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/input.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/src/monitor.c
)

file(GLOB GLAD_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/glad/src/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/glad/src/*.cpp
)

file(GLOB IMPLOT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/Implot/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/Implot/*.cpp
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

add_executable(ProofOfSpace WIN32 
	${INT128_SRC}
	${RELIC_SRC}
	${IMGUI_SRC}
	${IMFRAME_SRC}
	${GLFW_SRC}
	${GLAD_SRC}
	${IMPLOT_SRC}
	src/Keygen.cpp
    src/cli.cpp
	src/main.cpp
    src/chacha8.c
    ${BLAKE3_SRC}	
)

find_package(Threads REQUIRED)

#add_library(relic STATIC ${RELIC_SRC_FILES})
#target_include_directories(relic PUBLIC relic)

#add_library(uint128 STATIC uint128_t/uint128_t.cpp)
#target_include_directories(uint128 PUBLIC uint128_t)

target_compile_features(ProofOfSpace PUBLIC cxx_std_17)
target_compile_options(ProofOfSpace PUBLIC /MT /arch:AVX)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  target_link_libraries(ProofOfSpace fse Threads::Threads)
elseif (MSVC)
  #target_link_libraries(ProofOfSpace relic)
  target_link_libraries(ProofOfSpace fse Threads::Threads)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/gmp/gmp.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/freetype/freetype.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/bzip2/bz2.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/libpng/libpng16.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/zlib/zlib.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/brotli/brotlicommon-static.lib)
  target_link_libraries(ProofOfSpace ${CMAKE_SOURCE_DIR}/lib/brotli/brotlidec-static.lib)
else()
  target_link_libraries(ProofOfSpace fse stdc++fs Threads::Threads)
endif()

