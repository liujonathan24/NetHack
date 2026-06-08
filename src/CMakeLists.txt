cmake_minimum_required(VERSION 3.15)
file(STRINGS "version.txt" NLE_VERSION)
project(nle VERSION ${NLE_VERSION})

# Default to RelWithDebInfo: at -O0 (Debug) the per-env macro accesses
# (#define name (current_nle_ctx->s_name)) re-load __thread current_nle_ctx
# on every reference inside inner loops, which costs ~20-25% SPS at N=1024.
# -O2 hoists the TLS load. Explicit CMAKE_BUILD_TYPE on the command line
# still wins.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Default build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  # Unclear if this is even necessary. `dsymutil rlmain -o rlmain.dSYM` seems to
  # have done the trick.
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

  if(0)
    # address sanitizer.
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_C_FLAGS_DEBUG
        "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG
        "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address"
    )
  endif()
  if(MSVC)
    add_compile_options(/W4)
  else()
    add_compile_options(-Wall)
  endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
else()
  message("Some other build type.")
endif()

message(STATUS "Building nle backend version: ${NLE_VERSION}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# We use this to decide where the root of the nle/ package is. Normally it
# shouldn't be needed, but sometimes (e.g. when using setuptools) we are
# generating some of the files outside of the original package path.
set(PYTHON_SRC_PARENT
    ${nle_SOURCE_DIR}
    CACHE STRING "Directory containing the nle package files")

set(HACKDIR
    "$ENV{HOME}/nethackdir.nle"
    CACHE STRING "Configuration files for nethack")

message(STATUS "HACKDIR set to: ${HACKDIR}")

# Playground vars
set(VARDIR ${HACKDIR})
set(INSTDIR ${HACKDIR})

add_compile_definitions(
  GCC_WARN
  NOCLIPPING
  NOMAIL
  NOTPARMDECL
  HACKDIR="${HACKDIR}"
  DEFAULT_WINDOW_SYS="rl"
  DLB
  NOCWD_ASSUMPTIONS
  NLE_USE_TILES)

option(USE_SEEDING "Use seeding support in NLE" ON)

if(USE_SEEDING)
  add_compile_definitions(NLE_ALLOW_SEEDING)
  message("Seeding enabled.")
else()
  message("Seeding disabled.")
endif()

set(NLE_SRC ${nle_SOURCE_DIR}/src)
set(NLE_INC ${nle_SOURCE_DIR}/include)
set(NLE_DAT ${nle_SOURCE_DIR}/dat)
set(NLE_UTIL ${nle_SOURCE_DIR}/util)
set(NLE_DOC ${nle_SOURCE_DIR}/doc)
set(NLE_SSYS ${nle_SOURCE_DIR}/sys/share)
set(NLE_WIN ${nle_SOURCE_DIR}/win)

set(NLE_SRC_GEN ${nle_BINARY_DIR}/src)
set(NLE_INC_GEN ${nle_BINARY_DIR}/include)
set(NLE_DAT_GEN ${nle_BINARY_DIR}/dat)
set(NLE_UTIL_GEN ${nle_BINARY_DIR}/util)

set(CMAKE_INSTALL_MESSAGE LAZY) # Don't tell us about up-to-date files.

# EXCLUDE_FROM_ALL: Don't install this static library into /usr/local.
add_subdirectory(third_party/deboost.context EXCLUDE_FROM_ALL)
add_subdirectory(util)
add_subdirectory(dat)

file(
  GLOB
  NETHACK_SRC
  "src/*.c"
  "sys/share/posixregex.c"
  "sys/share/ioctl.c"
  "sys/unix/unixunix.c"
  "sys/unix/unixmain.c"
  "sys/unix/unixres.c"
  "win/tty/*.c"
  "win/rl/winrl.cc")
# NLE fast-reset: C++ operator new/delete overrides routed to the arena.
list(APPEND NETHACK_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/nle_arena_cpp.cc")

# terminal emulator library
add_library(tmt STATIC "third_party/libtmt/tmt.c")
set_target_properties(tmt PROPERTIES C_STANDARD 11)

# libnethack library
add_library(nethack SHARED ${NETHACK_SRC})
add_dependencies(nethack util dat)
set_target_properties(nethack PROPERTIES CXX_STANDARD 14 SUFFIX ".so")
target_include_directories(
  nethack
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${NLE_INC_GEN} /usr/local/include
         ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libtmt)
target_link_directories(nethack PUBLIC /usr/local/lib)

# Careful with -DMONITOR_HEAP: Ironically, it fails to fclose FILE* heaplog.
# target_compile_definitions(nethack PUBLIC "$<$<CONFIG:DEBUG>:MONITOR_HEAP>")

# NLE fast-reset: arena-based allocator. Replace NetHack's free() with a
# wrapper that no-ops arena pointers, so snapshot/restore of the arena
# captures all NetHack heap state. Also link with -Bsymbolic-functions so
# libnethack's intra-DSO calls bind to our wrapper, not libc free.
target_compile_definitions(nethack PRIVATE NLE_USE_ARENA_FREE=1)
# Cluster AO: lock[], SAVEF[], bones[], fqn_prefix[] are per-env fields in
# nle_ctx_t (see include/nle.h). Decl.h redirects the legacy identifiers
# to current_nle_ctx->s_FOO when this define is set. Util binaries do
# NOT define this, so their local `char lock[N]` / `char SAVEF[N]` keep
# working unchanged.
target_compile_definitions(nethack PRIVATE NLE_PER_ENV_FILES=1)
# Cluster AW-full: `flags` global (struct flag) is now per-env via macro
# indirection through current_nle_ctx->flags_ptr (see include/flag.h). This
# only takes effect when NLE_PER_ENV_FLAGS is defined; util binaries that
# don't include nle.h continue to declare their own local `flags`.
target_compile_definitions(nethack PRIVATE NLE_PER_ENV_FLAGS=1)
# -Bsymbolic-functions: bind libnethack's intra-DSO function calls (notably
# free) to our own definitions, so NetHack's free() calls resolve to
# nle_arena_free instead of libc free even when the loader is also linked.
# -z muldefs: tolerate the tentative-vs-strong duplicate of current_nle_ctx
# that gcc 10+ otherwise rejects under -fno-common (this is preexisting in
# the codebase; previous builds dodged it differently).
target_link_options(nethack PRIVATE
    "LINKER:-z,muldefs"
    # -Bsymbolic-functions binds libnethack's intra-DSO function calls to its
    # own copies (operator new/delete overrides in nle_arena_cpp.cc).
    "LINKER:-Bsymbolic-functions"
    # GCC won't hide operator new/delete via visibility attributes
    # (compiler implicit external linkage overrides #pragma visibility).
    # Use a version script to mark them `local` so the dynamic loader can't
    # interpose them on torch / pybind11 / Python allocations. Without this
    # fix Python's free() at finalize-time aborts with
    # "free(): invalid pointer" because some libc allocations were routed
    # through libnethack's bump arena.  Cluster AS.
    "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/nethack.ver"
)

find_package(Threads REQUIRED)
target_link_libraries(nethack PUBLIC m fcontext bz2 tmt Threads::Threads)

# dlopen wrapper library
add_library(nethackdl STATIC "sys/unix/nledl.c")
target_include_directories(nethackdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nethackdl PUBLIC dl)

# rlmain C++ (test) binary
add_executable(rlmain "sys/unix/rlmain.cc")
set_target_properties(rlmain PROPERTIES CXX_STANDARD 11)
target_link_libraries(rlmain PUBLIC nethackdl)
target_include_directories(rlmain PUBLIC ${NLE_INC_GEN})
target_compile_definitions(rlmain PRIVATE NLE_OBJECTS_GLOBAL)
add_dependencies(rlmain util) # For pm.h.

# pybind11 core python library.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11)
pybind11_add_module(
  _pynethack
  win/rl/pynethack.cc
  src/monst.c
  src/decl.c
  src/drawing.c
  src/objects.c
  $<TARGET_OBJECTS:tile>)
target_link_libraries(_pynethack PUBLIC nethackdl)
set_target_properties(_pynethack PROPERTIES CXX_STANDARD 14)
target_include_directories(_pynethack PUBLIC ${NLE_INC_GEN}
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/deboost.context/include)
add_dependencies(_pynethack util) # For pm.h.

# ttyrec converter library
add_library(
  converter STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/converter/converter.c
                   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/converter/stripgfx.c)
target_include_directories(
  converter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libtmt
                   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/converter)
target_link_libraries(converter PUBLIC bz2 tmt)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_options(converter PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# ttyrec reader executable
add_executable(ttyrec_reader EXCLUDE_FROM_ALL "third_party/converter/reader.c")
target_link_libraries(ttyrec_reader PUBLIC converter)
target_include_directories(
  ttyrec_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/converter)

pybind11_add_module(_pyconverter third_party/converter/pyconverter.cc)
target_link_libraries(_pyconverter PUBLIC converter)
set_target_properties(_pyconverter PROPERTIES CXX_STANDARD 14)
target_include_directories(
  _pyconverter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/converter)
