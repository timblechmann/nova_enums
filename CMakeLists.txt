cmake_minimum_required(VERSION 3.25)
cmake_policy(SET CMP0167 NEW)

project(nova_enums LANGUAGES CXX)

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

function(nova_enums_install_cpm)
    if (NOT COMMAND CPMAddPackage)
        set(CPM_DOWNLOAD_VERSION 0.42.3)
        if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
            message(STATUS "Downloading CPM.cmake ${CPM_DOWNLOAD_VERSION}...")
            file(DOWNLOAD
                "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
                "${CMAKE_BINARY_DIR}/cmake/CPM.cmake"
                TLS_VERIFY ON
                EXPECTED_HASH SHA256=a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844
            )
        endif()
        include("${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    endif()
endfunction()

########################################################################################################################

if(NOT (TARGET Boost::preprocessor OR TARGET Boost::headers))
    nova_enums_install_cpm()
    CPMAddPackage(
        NAME Boost
        VERSION 1.91.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/boost-1.91.0-1-cmake.tar.xz
        URL_HASH SHA256=cc5dc5006ecbdf0051f90979be31b4eee5987d9ae14ae9fb9c03cfa43fa3cdad
        OPTIONS "BOOST_SKIP_INSTALL_RULES ON"
                "BUILD_SHARED_LIBS OFF"
                "BOOST_INCLUDE_LIBRARIES preprocessor"
    )
endif()

########################################################################################################################

set(Headers
    inc/nova/enums/enums.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Headers})

add_library(nova_enums INTERFACE)
add_library(nova::enums ALIAS nova_enums)

target_include_directories(nova_enums INTERFACE inc)
if (TARGET Boost::preprocessor)
    target_link_libraries(nova_enums
        INTERFACE Boost::preprocessor
    )
else()
    target_link_libraries(nova_enums
        INTERFACE Boost::headers
    )
endif()

target_sources(nova_enums PUBLIC FILE_SET HEADERS FILES ${Headers} )

########################################################################################################################

option(NOVA_ENUMS_BUILD_TEST "Build unit tests" ${PROJECT_IS_TOP_LEVEL})

if (NOVA_ENUMS_BUILD_TEST)
    set(CMAKE_FOLDER "test")
    enable_testing()

    block()
        nova_enums_install_cpm()
        set(CMAKE_UNITY_BUILD ON)
        CPMAddPackage(
            NAME fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 12.1.0
        )

        CPMAddPackage(
            NAME Catch2
            GITHUB_REPOSITORY catchorg/Catch2
            VERSION 3.15.0
        )
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
        include(Catch)
    endblock()

    add_executable(nova_enums_test test/enums_test.cpp)
    target_link_libraries(nova_enums_test PRIVATE nova::enums fmt Catch2::Catch2 Catch2::Catch2WithMain)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES test/enums_test.cpp)

    catch_discover_tests(nova_enums_test)
endif()
