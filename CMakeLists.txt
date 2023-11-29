cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

project(nova_enums VERSION 0.1.0 LANGUAGES CXX)

########################################################################################################################

add_custom_target(nova_enums_project_files SOURCES
    .clang-tidy
    .pre-commit-config.yaml
    _clang-format
    conanfile.txt
    Readme.md
    License.txt
    .github/workflows/ci.yml
    cmake/conan_provider.cmake
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS conanfile.txt)

########################################################################################################################

find_package(Boost)
find_package(Catch2 CONFIG)

########################################################################################################################

set(Headers
    inc/nova/enums/enums.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Headers})

add_library(nova_enums INTERFACE)
add_library(nova::enums ALIAS nova_enums)

target_include_directories(nova_enums INTERFACE inc)
target_link_libraries(nova_enums
    INTERFACE  Boost::headers
)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
    target_sources(nova_enums PUBLIC FILE_SET HEADERS FILES ${Headers} )
endif()

########################################################################################################################

option(NOVA_ENUMS_BUILD_TEST "Build unit tests" ON)

if (NOVA_ENUMS_BUILD_TEST)
    enable_testing()

    find_package(Fmt)

    add_executable(nova_enums_test test/enums_test.cpp)

    add_test(NAME nova_enum_test COMMAND nova_enum_test)

    target_link_libraries(nova_enums_test
        PUBLIC Catch2::Catch2WithMain nova::enums fmt::fmt)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES test/symbol_test.cpp)
endif()
