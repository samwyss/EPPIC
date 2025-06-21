# CMake Setup ----------------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(EPPIC
        VERSION 0.0.1
        LANGUAGES CXX
        HOMEPAGE_URL https://github.com/samwyss/EPPIC
)

# compiler settings ----------------------------------------------------------------------------------------------------
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -Wall -g3)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -g0 -DNDEBUG -Wall -fstrict-aliasing -ftree-vectorize -march=native -mtune=native -mprefer-vector-width=512 -fno-trapping-math -fno-math-errno -ffast-math -flto)
    elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-O3 -g3 -Wall -fstrict-aliasing -ftree-vectorize -march=native -mtune=native -mprefer-vector-width=512 -fno-trapping-math -fno-math-errno -ffast-math)
    endif ()
else ()
    message(STATUS "Build flags are not explicitly set for this compiler.")
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
    message(FATAL_ERROR "Found HDF5 ${HDF5_VERSION} is not parallel. Please correct and reconfigure.")
endif ()

find_package(fmt QUIET)
if (NOT fmt_FOUND)
    message(STATUS "Library `{fmt}` not found. Installing locally now.")
    FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 11.1.4
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(fmt)
else ()
    message(STATUS "Library `{fmt}` found on system.")
endif ()

find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
    message(STATUS "Library `spdlog` not found. Installing locally now.")
    FetchContent_Declare(spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.15.3
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(spdlog)
else ()
    message(STATUS "Library `spdlog` found on system.")
endif ()

# testing --------------------------------------------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

# EPPIC setup ------------------------------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        src/fields/triplet.h
        src/fields/scalar.h
)

get_target_property(EPPIC_COMPILE_OPTIONS ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "C/C++ Compile Options: ${EPPIC_COMPILE_OPTIONS}")

add_library(Core
        ${PROJECT_SOURCE_DIR}/src/core/world.cpp
        ${PROJECT_SOURCE_DIR}/src/core/world.h
        ${PROJECT_SOURCE_DIR}/src/core/config.cpp
        ${PROJECT_SOURCE_DIR}/src/core/config.h
        src/core/emengine.cpp
        src/core/emengine.h
)

add_library(Constants
        ${PROJECT_SOURCE_DIR}/src/constants/physical.hpp
)
set_target_properties(Constants PROPERTIES LINKER_LANGUAGE CXX)

add_library(Fields
        src/fields/triplet.h
        src/fields/scalar.h
        src/fields/vector.h
)
set_target_properties(Fields PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME}
        PRIVATE ${MPI_CXX_INCLUDE_PATH}
        PRIVATE ${OPENMP_CXX_INCLUDE_PATH}
        PRIVATE ${HDF5_INCLUDE_DIRS}
        PRIVATE ${PROJECT_BINARY_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/src/core
        PRIVATE ${PROJECT_SOURCE_DIR}/src/fields
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE MPI::MPI_CXX
        PRIVATE OpenMP::OpenMP_CXX
        PRIVATE HDF5::HDF5
        PRIVATE fmt::fmt
        PRIVATE spdlog::spdlog
        PRIVATE Core
        PRIVATE Fields
)