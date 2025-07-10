# CMake Setup ----------------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(EPPIC
        VERSION 0.0.1
        LANGUAGES CXX
        HOMEPAGE_URL https://github.com/samwyss/EPPIC
)

# compiler settings ----------------------------------------------------------------------------------------------------
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -Wall -g3)
        add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -g0 -DNDEBUG -Wall -fstrict-aliasing -ftree-vectorize -march=native -mtune=native -mprefer-vector-width=512 -fno-trapping-math -fno-math-errno -ffast-math -flto)
        add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)
    elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-O3 -g3 -Wall -fstrict-aliasing -ftree-vectorize -march=native -mtune=native -mprefer-vector-width=512 -fno-trapping-math -fno-math-errno -ffast-math)
        add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
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

set(MDSPAN_ENABLE_TESTS OFF)
set(MDSPAN_ENABLE_BENCHMARKS OFF)
find_package(mdspan QUIET)
if (NOT mdspan_FOUND)
    message(STATUS "Library `mdspan` not found. Installing locally now.")
    FetchContent_Declare(mdspan
            GIT_REPOSITORY https://github.com/kokkos/mdspan.git
            GIT_TAG stable
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(mdspan)
else ()
    message(STATUS "Library `mdspan` found on system.")
endif ()

find_package(SimpleXdmf QUIET)
if (NOT SimpleXdmf_FOUND)
    message(STATUS "Library `SimpleXdmf` not found. Installing locally now.")
    FetchContent_Declare(SimpleXdmf
            GIT_REPOSITORY https://github.com/hsimyu/SimpleXdmf.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            GIT_SUBMODULES "" # prevents cloning submodules
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(SimpleXdmf)
else ()
    message(STATUS "Library `SimpleXdmf` found on system.")
endif ()

# testing --------------------------------------------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

# EPPIC setup ------------------------------------------------------------------------------------------------
# main executable
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/fields/coordinate.h
        src/fields/scalar.h
)

# diagnostics
get_target_property(EPPIC_COMPILE_OPTIONS ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "C/C++ Compile Options: ${EPPIC_COMPILE_OPTIONS}")

# Constants library
add_library(Constants
        src/constants/physical.h
        src/constants/numeric.h
        src/constants/type.h
)

set_target_properties(Constants PROPERTIES LINKER_LANGUAGE CXX)

# Fields library
add_library(Fields
        src/fields/coordinate.h
        src/fields/scalar.h
        src/fields/vector.h
)

set_target_properties(Fields PROPERTIES LINKER_LANGUAGE CXX)

# Core library
add_library(Core
        src/core/world.cpp
        src/core/world.h
        src/core/config.cpp
        src/core/config.h
        src/core/fdtd_engine.cpp
        src/core/fdtd_engine.h
)

target_include_directories(Core
        PUBLIC src/fields
        PUBLIC src/constants
        PUBLIC ${simplexdmf_SOURCE_DIR}/include
)

target_link_libraries(Core
        PUBLIC Fields
        PUBLIC Constants
        PUBLIC mdspan::mdspan
        PUBLIC spdlog::spdlog
)

# main executable
target_include_directories(${PROJECT_NAME}
        PRIVATE ${MPI_CXX_INCLUDE_PATH}
        PRIVATE ${OPENMP_CXX_INCLUDE_PATH}
        PRIVATE ${HDF5_INCLUDE_DIRS}
        PRIVATE ${PROJECT_BINARY_DIR}
        PRIVATE ${simplexdmf_SOURCE_DIR}/include
        PRIVATE src/core
        PRIVATE src/fields
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE MPI::MPI_CXX
        PRIVATE OpenMP::OpenMP_CXX
        PRIVATE HDF5::HDF5
        PRIVATE fmt::fmt
        PRIVATE spdlog::spdlog
        PRIVATE Core
        PRIVATE Fields
        PRIVATE mdspan::mdspan
)