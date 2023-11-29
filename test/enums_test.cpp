#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <nova/enums/enums.hpp>

#include <fmt/format.h>

//----------------------------------------------------------------------------------------------------------------------

enum class foo
{
    a,
    b,
    c,
};

NOVA_ENUMS_REGISTER( foo, a, b, c );

// clang-format off
// NOVA_ENUMS_REGISTER_SEQ( foo, (a)(b)(c) );
// NOVA_ENUMS_REGISTER_LIST( foo, (a, (b, (c, BOOST_PP_NIL))));
// clang-format on

//----------------------------------------------------------------------------------------------------------------------

enum class bar
{
    a = 2,
    b = 22,
    c = -222,
};

NOVA_ENUMS_REGISTER( bar, a, b, c );

//----------------------------------------------------------------------------------------------------------------------

namespace nova::enums {

//----------------------------------------------------------------------------------------------------------------------

static_assert( is_ordinal_v< foo > );
static_assert( !is_ordinal_v< bar > );
static_assert( is_registered_enum_v< foo > );

static_assert( number_of_elements< foo > == 3 );
static_assert( number_of_elements< bar > == 3 );

static_assert( is_valid< foo >( 2 ) );
static_assert( !is_valid< foo >( 3 ) );

static_assert( is_valid< bar >( 2 ) );
static_assert( is_valid< bar >( 22 ) );
static_assert( is_valid< bar >( -222 ) );
static_assert( !is_valid< bar >( 23 ) );

//----------------------------------------------------------------------------------------------------------------------

TEST_CASE( "enums" )
{
    CHECK( to_string( foo::a ) == "a" );
    CHECK( to_string( bar::a ) == "a" );

    CHECK( to_enum< foo >( "a" ) == foo::a );
    CHECK( to_enum< bar >( "a" ) == bar::a );

    CHECK( to_enum< foo >( 0 ) == foo::a );
    CHECK( to_enum< foo >( 3 ) == std::nullopt );

    CHECK( to_enum< bar >( 2 ) == bar::a );
    CHECK( to_enum< bar >( 3 ) == std::nullopt );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_CASE( "values" )
{
    using namespace std::string_view_literals;

    auto values_in_foo = { foo::a, foo::b, foo::c };
    auto values_in_bar = { bar::a, bar::b, bar::c };
    auto string_values = { "a"sv, "b"sv, "c"sv };

    CHECK_THAT( all_enum_values< foo >, Catch::Matchers::RangeEquals( values_in_foo ) );
    CHECK_THAT( all_enum_values< bar >, Catch::Matchers::UnorderedRangeEquals( values_in_bar ) );

    CHECK_THAT( all_enum_strings< foo >, Catch::Matchers::RangeEquals( string_values ) );
    CHECK_THAT( all_enum_strings< bar >, Catch::Matchers::UnorderedRangeEquals( string_values ) );
}

//----------------------------------------------------------------------------------------------------------------------

TEST_CASE( "format" )
{
    CHECK( fmt::format( "{}", foo::a ) == "a" );
}

//----------------------------------------------------------------------------------------------------------------------

struct my_string_view
{
    std::string_view sv;

    auto operator<=>( const my_string_view& rhs ) const = default;
};

template <>
struct string_adapter< my_string_view >
{
    my_string_view operator()( std::string_view sv ) const
    {
        return my_string_view {
            .sv = sv,
        };
    }
};

TEST_CASE( "enums string adaptor" )
{
    auto asv = my_string_view {
        .sv = "a",
    };

    auto bsv = my_string_view {
        .sv = "b",
    };

    auto csv = my_string_view {
        .sv = "c",
    };

    CHECK( to_string< my_string_view >( foo::a ) == asv );
    CHECK( to_enum< foo >( asv ) == foo::a );

    auto string_values = { asv, bsv, csv };
    auto enum_strings  = all_enum_strings< foo, my_string_view >;
    CHECK_THAT( enum_strings, Catch::Matchers::RangeEquals( string_values ) );
}

//----------------------------------------------------------------------------------------------------------------------


} // namespace nova::enums

//----------------------------------------------------------------------------------------------------------------------

#ifdef __cpp_lib_format

TEST_CASE( "format" )
{
    CHECK( std::format( "{}", foo::a ) == "a" );
}

#endif
