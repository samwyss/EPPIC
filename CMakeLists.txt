# Copyright (C) 2025 Samuel Wyss
#
# This file is part of EPPIC.
#
# EPPIC is free software: you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# EPPIC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with EPPIC. If not, see
# <https://www.gnu.org/licenses/>.

# CMake Setup ----------------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(EPPIC
        VERSION 0.0.1
        LANGUAGES CXX C
        HOMEPAGE_URL https://github.com/samwyss/EPPIC
)

# build options --------------------------------------------------------------------------------------------------------
option(EPPIC_USE_FLTO "enable fat link time optimization" OFF)
if (EPPIC_USE_FLTO)
    message(STATUS "fat link time optimization: enabled")
    add_compile_options(-flto)
    add_link_options(-flto)
else ()
    message(STATUS "fat link time optimization: disabled")
endif ()

option(EPPIC_USE_FAST_MATH "enable fast math" OFF)
if (EPPIC_USE_FAST_MATH)
    message(STATUS "fast math: enabled")
    add_compile_options(-ffast-math)
else ()
    message(STATUS "fast math: disabled")
endif ()

option(EPPIC_USE_ASAN, "enable address sanitizer" OFF)
if (EPPIC_USE_ASAN)
    message(STATUS "address sanitizer: enabled")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
else ()
    message(STATUS "address sanitizer: disabled")
endif ()

option(EPPIC_USE_FLOAT, "uses `float` instead of `double` as floating point type for EPPIC" OFF)
if (EPPIC_USE_FLOAT)
    message(STATUS "floating point type: float")
    add_compile_definitions(EPPIC_USE_FLOAT=1)
else ()
    message(STATUS "floating point type: double")
    add_compile_definitions(EPPIC_USE_FLOAT=0)
endif ()

option(EPPIC_USE_UINT32_T, "uses `uint32_t` instead of `uint64_t` as unsigned integer type for EPPIC" OFF)
if (EPPIC_USE_UINT32_T)
    message(STATUS "unsigned integer type: uint32_t")
    add_compile_definitions(EPPIC_USE_UINT32_T=1)
else ()
    message(STATUS "unsigned integer type: uint64_t")
    add_compile_definitions(EPPIC_USE_UINT32_T=0)
endif ()

# general compiler settings --------------------------------------------------------------------------------------------
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -Wall -g3)
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG -Wall -march=native -mtune=native -fno-trapping-math -fno-math-errno)
    add_link_options(-flto)
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-O2 -g3 -DNDEBUG -Wall -march=native -mtune=native -fno-omit-frame-pointer -fno-trapping-math -fno-math-errno)
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)
endif ()

# dependencies ---------------------------------------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

find_package(MPI REQUIRED COMPONENTS CXX)

find_package(OpenMP REQUIRED COMPONENTS CXX)

set(HDF5_PREFER_PARALLEL TRUE)
find_package(HDF5 REQUIRED)
if (NOT HDF5_IS_PARALLEL)
    message(FATAL_ERROR "found HDF5 ${HDF5_VERSION} is not parallel ... please correct and reconfigure")
endif ()

find_package(fmt QUIET)
if (NOT fmt_FOUND)
    message(STATUS "library `{fmt}` not found ... installing locally now")
    FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 11.2.0
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(fmt)
else ()
    message(STATUS "library `{fmt}` found on system")
endif ()

find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
    message(STATUS "library `spdlog` not found ... installing locally now")
    FetchContent_Declare(spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.15.3
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(spdlog)
else ()
    message(STATUS "library `spdlog` found on system")
endif ()

set(MDSPAN_ENABLE_TESTS OFF)
set(MDSPAN_ENABLE_BENCHMARKS OFF)
find_package(mdspan QUIET)
if (NOT mdspan_FOUND)
    message(STATUS "library `mdspan` not found ... installing locally now")
    FetchContent_Declare(mdspan
            GIT_REPOSITORY https://github.com/kokkos/mdspan.git
            GIT_TAG stable
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(mdspan)
else ()
    message(STATUS "library `mdspan` found on system")
endif ()

find_package(toml11 QUIET)
if (NOT toml11_FOUND)
    message(STATUS "library `toml11` not found ... installing locally now")
    FetchContent_Declare(toml11
            GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
            GIT_TAG v4.4.0
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(toml11)
else ()
    message(STATUS "library `toml11` found on system")
endif ()

# EPPIC setup ------------------------------------------------------------------------------------------------
# main executable
add_executable(${PROJECT_NAME}
        src/main.cpp
)

# diagnostics
message(STATUS "C Compiler: ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
get_target_property(EPPIC_COMPILE_OPTIONS ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "C/C++ Compile Options: ${EPPIC_COMPILE_OPTIONS}")

# Core library
add_library(Core
        src/core/config.cpp
        src/core/config.h
        src/core/io.h
        src/core/physical.h
        src/core/numeric.h
        src/core/type.h
        src/core/coordinate.h
        src/core/scalar.h
        src/core/vector.h
        src/core/world.cpp
        src/core/world.h
)

target_include_directories(Core
        PUBLIC ${MPI_CXX_INCLUDE_PATH}
        PUBLIC ${OPENMP_CXX_INCLUDE_PATH}
        PUBLIC ${HDF5_INCLUDE_DIRS}
        PUBLIC ${PROJECT_BINARY_DIR}
)

target_link_libraries(Core
        PUBLIC mdspan::mdspan
        PUBLIC spdlog::spdlog
        PUBLIC toml11::toml11
        PUBLIC MPI::MPI_CXX
        PUBLIC OpenMP::OpenMP_CXX
        PUBLIC HDF5::HDF5
        PUBLIC fmt::fmt
        PUBLIC spdlog::spdlog
)

# main executable
target_include_directories(${PROJECT_NAME}
        PRIVATE src/core
        PRIVATE ${PROJECT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Core
        PRIVATE fmt::fmt
        PRIVATE spdlog::spdlog
)