cmake_minimum_required(VERSION 3.25)

project(nova_enums VERSION 0.1.0 LANGUAGES CXX)

########################################################################################################################

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

if(NOT CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
endif()

########################################################################################################################

add_custom_target(nova_enums_project_files SOURCES
    .clang-tidy
    .pre-commit-config.yaml
    _clang-format
    Readme.md
    License.txt
    .github/workflows/ci.yml
)

########################################################################################################################
# Bootstrap CPM.cmake (download to build dir on first configure)

if(PROJECT_IS_TOP_LEVEL)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
        file(DOWNLOAD
            "https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake"
            "${CMAKE_BINARY_DIR}/cmake/CPM.cmake"
            TLS_VERIFY ON
        )
    endif()
    include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

    CPMAddPackage(
            NAME Boost
            VERSION 1.90.0
            URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
            URL_HASH SHA256=aca59f889f0f32028ad88ba6764582b63c916ce5f77b31289ad19421a96c555f
            OPTIONS "BOOST_ENABLE_CMAKE ON"
                    "BOOST_SKIP_INSTALL_RULES ON"
                    "BUILD_SHARED_LIBS OFF"
                    "BOOST_INCLUDE_LIBRARIES preprocessor"
    )

    block()
        set(CMAKE_UNITY_BUILD ON)
        CPMAddPackage(
            NAME fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 12.1.0
        )

        CPMAddPackage(
            NAME Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.14.0
        )
    endblock()
endif()

########################################################################################################################

set(Headers
    inc/nova/enums/enums.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Headers})

add_library(nova_enums INTERFACE)
add_library(nova::enums ALIAS nova_enums)

target_include_directories(nova_enums INTERFACE inc)
target_link_libraries(nova_enums
    INTERFACE Boost::preprocessor
)

target_sources(nova_enums PUBLIC FILE_SET HEADERS FILES ${Headers} )

########################################################################################################################

option(NOVA_ENUMS_BUILD_TEST "Build unit tests" ON)

if (NOVA_ENUMS_BUILD_TEST)
    enable_testing()

    add_executable(nova_enums_test test/enums_test.cpp)

    # register test executable (name matches executable)
    add_test(NAME nova_enums_test COMMAND nova_enums_test)

    target_link_libraries(nova_enums_test
        PUBLIC Catch2::Catch2WithMain nova::enums fmt::fmt)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES test/enums_test.cpp)
endif()
