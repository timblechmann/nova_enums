#pragma once

// Copyright (c) 2023 Tim Blechmann
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// As a non-binding request, please use this code responsibly and ethically.

#include <boost/preprocessor.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>
#include <version>

//----------------------------------------------------------------------------------------------------------------------

namespace nova::enums {

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
constexpr auto to_underlying( EnumType e )
{
    return static_cast< std::underlying_type_t< EnumType > >( e );
}

//----------------------------------------------------------------------------------------------------------------------

template < typename StringType >
struct string_adapter
{
    // expected signature
    StringType operator()( std::string_view ) const;
};

//----------------------------------------------------------------------------------------------------------------------

namespace impl {

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
struct is_registered_enum : std::false_type
{};

} // namespace impl

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
constexpr inline bool is_registered_enum_v = impl::is_registered_enum< EnumType >::value;


//----------------------------------------------------------------------------------------------------------------------

namespace impl {

//----------------------------------------------------------------------------------------------------------------------

[[noreturn]] void unreachable()
{
#ifdef __cpp_lib_unreachable
    return std::unreachable();
#else
    abort();
#endif
}


//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType, size_t Size >
    requires( std::is_enum_v< EnumType > )
constexpr bool is_ordinal_table( std::array< EnumType, Size > list )
{
    return std::ranges::equal( list, std::ranges::views::iota( size_t( 0 ), list.size() ), {}, to_underlying< EnumType > );
}

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
constexpr bool determine_ordinality();

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
inline constexpr bool is_ordinal = determine_ordinality< EnumType >();

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
struct number_of_elements;

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
inline constexpr size_t number_of_elements_v = number_of_elements< EnumType >::value;

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType, typename StringType >
    requires( std::is_enum_v< EnumType > )
struct enum_lookup_table_common
{
    static constexpr size_t number_of_elements = number_of_elements_v< EnumType >;

    using association       = std::pair< EnumType, StringType >;
    using enum_lookup_table = std::array< std::pair< StringType, EnumType >, number_of_elements >;

    constexpr explicit enum_lookup_table_common( std::array< association, number_of_elements > associations )
    {
        std::ranges::sort( associations, []( const association& lhs, const association& rhs ) {
            return lhs.second < rhs.second;
        } );

        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            string_to_enum_table[ index ] = std::pair {
                associations[ index ].second,
                associations[ index ].first,
            };
    }

    explicit enum_lookup_table_common( const enum_lookup_table_common< EnumType, std::string_view >& other )
    {
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            string_to_enum_table[ index ] = std::pair {
                nova::enums::string_adapter< StringType >()( other.string_to_enum_table[ index ].first ),
                other.string_to_enum_table[ index ].second,
            };

        std::ranges::sort( string_to_enum_table, std::less<> {}, []( const std::pair< StringType, EnumType >& value ) {
            return value.first;
        } );
    }

    constexpr std::optional< EnumType > to_enum( const StringType& sv ) const
    {
        auto found = std::ranges::lower_bound( string_to_enum_table,
                                               sv,
                                               {},
                                               []( const std::pair< StringType, EnumType >& element ) {
            return element.first;
        } );

        if ( found == string_to_enum_table.end() )
            return std::nullopt;

        if ( found->first == sv )
            return found->second;
        else
            return std::nullopt;
    }

    constexpr std::array< StringType, number_of_elements > all_enum_strings() const
    {
        std::array< StringType, number_of_elements > ret;
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            ret[ index ] = string_to_enum_table[ index ].first;
        return ret;
    }

    enum_lookup_table string_to_enum_table {};
};

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType, typename StringType >
    requires( std::is_enum_v< EnumType > )
struct ordinal_enum_lookup_table : enum_lookup_table_common< EnumType, StringType >
{
    using common_table = enum_lookup_table_common< EnumType, StringType >;
    using association  = typename common_table::association;
    using common_table::number_of_elements;

    using string_lookup_table = std::array< StringType, number_of_elements >;

    constexpr explicit ordinal_enum_lookup_table( std::array< association, number_of_elements > associations ) :
        common_table {
            associations,
        }
    {
        assert( std::ranges::is_sorted( associations, []( const association& lhs, const association& rhs ) {
            return lhs.first < rhs.first;
        } ) );

        auto strings_only = std::ranges::views::transform( associations, []( const association& value ) {
            return value.second;
        } );

        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            string_table[ index ] = strings_only[ index ];
    }

    explicit ordinal_enum_lookup_table( const ordinal_enum_lookup_table< EnumType, std::string_view >& other ) :
        common_table {
            other,
        }
    {
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            string_table[ index ] = nova::enums::string_adapter< StringType >()( other.string_table[ index ] );
    }

    template < typename IntType >
        requires( std::is_integral_v< IntType > )
    constexpr bool is_valid( IntType i ) const
    {
        return i >= 0 && i < number_of_elements;
    }

    constexpr const StringType& to_string( EnumType e ) const
    {
        assert( is_valid( to_underlying( e ) ) );
        return string_table[ to_underlying( e ) ];
    }

    constexpr std::array< EnumType, number_of_elements > all_enums() const
    {
        std::array< EnumType, number_of_elements > ret;
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            ret[ index ] = EnumType( index );
        return ret;
    }

    string_lookup_table string_table {};
};

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType, typename StringType >
    requires( std::is_enum_v< EnumType > )
struct enum_lookup_table : enum_lookup_table_common< EnumType, StringType >
{
    using common_table = enum_lookup_table_common< EnumType, StringType >;
    using association  = typename common_table::association;
    using common_table::number_of_elements;

    using string_lookup_table = std::array< std::pair< EnumType, StringType >, number_of_elements >;

    constexpr enum_lookup_table( const std::array< association, number_of_elements >& associations ) :
        common_table {
            associations,
        }
    {
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) ) {
            string_table[ index ] = associations[ index ];
        }

        std::ranges::sort( string_table, []( const association& lhs, const association& rhs ) {
            return lhs.first < rhs.first;
        } );
    }

    constexpr enum_lookup_table( const enum_lookup_table< EnumType, std::string_view >& other ) :
        common_table {
            other,
        }
    {
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            string_table[ index ] = std::pair {
                other.string_table[ index ].first,
                nova::enums::string_adapter< StringType >()( other.string_table[ index ].second ),
            };
    }

    template < typename IntType >
        requires( std::is_integral_v< IntType > )
    constexpr bool is_valid( IntType i ) const
    {
        auto found = std::ranges::lower_bound( string_table,
                                               i,
                                               std::less<>(),
                                               []( const std::pair< EnumType, StringType >& element ) {
            return IntType( element.first );
        } );
        if ( found == string_table.end() )
            return false;

        return IntType( found->first ) == i;
    }

    constexpr const StringType& to_string( EnumType e ) const
    {
        auto found = std::ranges::lower_bound( string_table,
                                               e,
                                               std::less<>(),
                                               []( const std::pair< EnumType, StringType >& element ) {
            return element.first;
        } );

        if ( found != string_table.end() && found->first == e )
            return found->second;

        assert( false && "enum not found" );
        unreachable();
        return string_table.front().second;
    }

    constexpr std::array< EnumType, number_of_elements > all_enums() const
    {
        std::array< EnumType, number_of_elements > ret;
        for ( size_t index : std::ranges::views::iota( size_t( 0 ), number_of_elements ) )
            ret[ index ] = string_table[ index ].first;
        return ret;
    }


    string_lookup_table string_table {};
};

//----------------------------------------------------------------------------------------------------------------------

template < typename Enum >
constexpr auto make_enum_table();

template < typename Enum, typename StringType >
constexpr auto enum_table_for_string_type()
{
    if constexpr ( std::is_same_v< StringType, std::string_view > ) {
        return make_enum_table< Enum >();
    } else {
        if constexpr ( is_ordinal< Enum > )
            return ordinal_enum_lookup_table< Enum, StringType >( make_enum_table< Enum >() );
        else
            return enum_lookup_table< Enum, StringType >( make_enum_table< Enum >() );
    }
}

template < typename Enum, typename StringType = std::string_view >
inline constexpr auto enum_table = enum_table_for_string_type< Enum, StringType >();

template < typename Enum, typename StringType >
const auto& static_enum_table()
{
    static const auto table = enum_table_for_string_type< Enum, StringType >();
    return table;
}

//----------------------------------------------------------------------------------------------------------------------

template < typename Type >
constexpr inline bool string_view_or_int_v
    = std::is_convertible_v< Type, std::string_view > || std::is_integral_v< Type >;

} // namespace impl

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
    requires( is_registered_enum_v< EnumType > )
inline constexpr bool is_ordinal_v = impl::is_ordinal< EnumType >;

template < typename EnumType >
    requires( is_registered_enum_v< EnumType > )
inline constexpr size_t number_of_elements = impl::number_of_elements_v< EnumType >;

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType, typename IntType >
    requires( is_registered_enum_v< EnumType >, std::is_integral_v< IntType > )
constexpr bool is_valid( IntType arg )
{
    return impl::enum_table< EnumType, std::string_view >.is_valid( arg );
}

//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
    requires( is_registered_enum_v< EnumType > )
constexpr std::string_view to_string( EnumType value )
{
    return impl::enum_table< EnumType, std::string_view >.to_string( value );
}

template < typename StringType, typename EnumType >
    requires( is_registered_enum_v< EnumType > )
auto to_string( EnumType value )
{
    return impl::static_enum_table< EnumType, StringType >().to_string( value );
}


//----------------------------------------------------------------------------------------------------------------------


template < typename EnumType, typename StringOrInt = std::string_view >
    requires( is_registered_enum_v< EnumType >, impl::string_view_or_int_v< StringOrInt > )
constexpr std::optional< EnumType > to_enum( const StringOrInt& string_or_int )
{
    if constexpr ( std::is_convertible_v< StringOrInt, std::string_view > )
        return impl::enum_table< EnumType >.to_enum( std::string_view { string_or_int } );
    else if constexpr ( std::is_integral_v< StringOrInt > ) {
        if ( is_valid< EnumType >( string_or_int ) )
            return EnumType( string_or_int );
        else
            return std::nullopt;
    }
}

template < typename EnumType, typename StringOrInt >
    requires( is_registered_enum_v< EnumType >, !impl::string_view_or_int_v< StringOrInt > )
std::optional< EnumType > to_enum( const StringOrInt& string_or_int )
{
    return impl::static_enum_table< EnumType, StringOrInt >().to_enum( string_or_int );
}


//----------------------------------------------------------------------------------------------------------------------

template < typename EnumType >
    requires( is_registered_enum_v< EnumType > )
inline constexpr auto all_enum_values = impl::enum_table< EnumType >.all_enums();

template < typename EnumType, typename StringType = std::string_view >
    requires( is_registered_enum_v< EnumType > )
inline auto all_enum_strings = impl::static_enum_table< EnumType, StringType >().all_enum_strings();

template < typename EnumType >
    requires( is_registered_enum_v< EnumType > )
inline constexpr auto all_enum_strings< EnumType, std::string_view >
    = impl::enum_table< EnumType, std::string_view >.all_enum_strings();


//----------------------------------------------------------------------------------------------------------------------

} // namespace nova::enums

//----------------------------------------------------------------------------------------------------------------------

#define NOVA_ENUMS_IMPL_MAKE_TABLE_ENTRY_MACRO( r, data, element ) \
    std::pair {                                                    \
        data::element,                                             \
        std::string_view { BOOST_PP_STRINGIZE( element ) },        \
        },

#define NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE_LIST( TYPE, LIST )                                                     \
                                                                                                               \
    template <>                                                                                                \
    constexpr auto nova::enums::impl::make_enum_table< TYPE >()                                                \
    {                                                                                                          \
        constexpr auto table_definition = std::to_array< std::pair< TYPE, std::string_view > >(                \
            { BOOST_PP_LIST_FOR_EACH( NOVA_ENUMS_IMPL_MAKE_TABLE_ENTRY_MACRO, TYPE, LIST ) } );                \
                                                                                                               \
        if constexpr ( nova::enums::impl::is_ordinal< TYPE > )                                                 \
            return nova::enums::impl::ordinal_enum_lookup_table< TYPE, std::string_view >( table_definition ); \
        else                                                                                                   \
            return nova::enums::impl::enum_lookup_table< TYPE, std::string_view >( table_definition );         \
    };

#define NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE( TYPE, ... ) \
    NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE_LIST( TYPE, BOOST_PP_VARIADIC_TO_LIST( __VA_ARGS__ ) )

//----------------------------------------------------------------------------------------------------------------------

#define NOVA_ENUMS_IMPL_ENUM_LIST_MACRO( r, data, element ) data::element,

#define NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR_LIST( TYPE, LIST )                                                  \
                                                                                                                  \
    template <>                                                                                                   \
    constexpr bool nova::enums::impl::determine_ordinality< TYPE >()                                              \
    {                                                                                                             \
        constexpr auto table_definition                                                                           \
            = std::to_array< TYPE >( { BOOST_PP_LIST_FOR_EACH( NOVA_ENUMS_IMPL_ENUM_LIST_MACRO, TYPE, LIST ) } ); \
                                                                                                                  \
        return nova::enums::impl::is_ordinal_table( table_definition );                                           \
    }

#define NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR( TYPE, ... ) \
    NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR_LIST( TYPE, BOOST_PP_VARIADIC_TO_LIST( __VA_ARGS__ ) )

//----------------------------------------------------------------------------------------------------------------------

#define NOVA_ENUMS_IMPL_MAKE_NUMBER_OF_ELEMENTS_LIST( TYPE, LIST )                                                      \
    template <>                                                                                                         \
    struct nova::enums::impl::number_of_elements< TYPE > : std::integral_constant< size_t, BOOST_PP_LIST_SIZE( LIST ) > \
    {};

#define NOVA_ENUMS_IMPL_MAKE_NUMBER_OF_ELEMENTS( TYPE, ... )                    \
    template <>                                                                 \
    struct nova::enums::impl::number_of_elements< TYPE > :                      \
        std::integral_constant< size_t, BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ) > \
    {};

//----------------------------------------------------------------------------------------------------------------------

#define NOVA_ENUMS_IMPL_MAKE_REGISTRATION_TRAIT( TYPE )                   \
    template <>                                                           \
    struct nova::enums::impl::is_registered_enum< TYPE > : std::true_type \
    {};

//----------------------------------------------------------------------------------------------------------------------

#define NOVA_ENUMS_REGISTER( TYPE, ... )                         \
    NOVA_ENUMS_IMPL_MAKE_REGISTRATION_TRAIT( TYPE )              \
    NOVA_ENUMS_IMPL_MAKE_NUMBER_OF_ELEMENTS( TYPE, __VA_ARGS__ ) \
    NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR( TYPE, __VA_ARGS__ )   \
    NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE( TYPE, __VA_ARGS__ )         \
    static_assert( true, "force semicolon" )

#define NOVA_ENUMS_REGISTER_LIST( TYPE, LIST )                 \
    NOVA_ENUMS_IMPL_MAKE_REGISTRATION_TRAIT( TYPE )            \
    NOVA_ENUMS_IMPL_MAKE_NUMBER_OF_ELEMENTS_LIST( TYPE, LIST ) \
    NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR_LIST( TYPE, LIST )   \
    NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE_LIST( TYPE, LIST )         \
    static_assert( true, "force semicolon" )

#define NOVA_ENUMS_REGISTER_SEQ( TYPE, SEQ )                                          \
    NOVA_ENUMS_IMPL_MAKE_REGISTRATION_TRAIT( TYPE )                                   \
    NOVA_ENUMS_IMPL_MAKE_NUMBER_OF_ELEMENTS_LIST( TYPE, BOOST_PP_SEQ_TO_LIST( SEQ ) ) \
    NOVA_ENUMS_IMPL_MAKE_ORDINAL_DETECTOR_LIST( TYPE, BOOST_PP_SEQ_TO_LIST( SEQ ) )   \
    NOVA_ENUMS_IMPL_MAKE_ENUM_TABLE_LIST( TYPE, BOOST_PP_SEQ_TO_LIST( SEQ ) )         \
    static_assert( true, "force semicolon" )


//----------------------------------------------------------------------------------------------------------------------

#if __has_include( <fmt/format.h> )
#    include <fmt/format.h>

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
struct fmt::formatter< EnumType, char > : fmt::formatter< std::string_view >
{
    template < typename FormatContext >
    auto format( EnumType value, FormatContext& ctx ) const -> decltype( ctx.out() )
    {
        return fmt::format_to( ctx.out(), "{}", nova::enums::to_string( value ) );
    }
};

#endif


#if __cpp_lib_format
#    include <format>

template < typename EnumType >
    requires( std::is_enum_v< EnumType > )
struct st::formatter< EnumType, char > : st::formatter< std::string_view >
{
    template < typename FormatContext >
    auto format( EnumType value, FormatContext& ctx ) const -> decltype( ctx.out() )
    {
        return std::format_to( ctx.out(), "{}", nova::enums::to_string( value ) );
    }
};

#endif
