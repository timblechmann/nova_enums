# Enums

Adding introspection to c++ enums:

* string/enum conversions
* checking integral values
* listing all possible values

Features:
* (mostly) constexpr
* efficent implementation via sorted arrays as lookup tables

## Example

```c++

// example enum
enum class foo
{
    a,
    b,
    c,
};

// register enum with library
NOVA_ENUMS_REGISTER( foo, a, b, c ); // order matters!

using namespac nova::enums;

// compile-time introspection
constexpr bool is_ordinal    = is_ordinal_v< foo >;         // enum contains ordinal numbers (no gaps)
constexpr bool is_registered = is_registered_enum_v< foo >; // is registered with nova.enum


// enum/string conversions
constexpr std::string_view a_string   = to_string( foo::a );
constexpr std::optional< foo > foo_a  = to_enum< foo >( "a" );
constexpr std::optional< foo > foo_0  = to_enum< foo >( 0 );
std::string formatted_fmt             = fmt::format( "{}", foo::a );
std::string formatted_std             = std::format( "{}", foo::a );


// checks
constexpr bool is_42_valid                              = is_valid< foo >( 42 );   // 42 is not in foo
constexpr std::array< foo, 3 > all_values               = all_enum_values< foo >;  // {foo::a, foo::b, foo::c, }
constexpr std::array< std::string_view, 3 > all_strings = all_enum_strings< foo >; // {"a"sv, "b"sv, "c"sv, }

```

# Dependencies
* C++20 (with ranges and concepts)
* Boost (preprocessor)
* Catch2 (for unit tests)

# Building

Integrate Conan via
```
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/conan_provider.cmake [...]
```


# Caveats

`NOVA_ENUMS_REGISTER` depends on variadic macros that are limited in length to 128 elements. If you run into this length
limitation one can use `NOVA_ENUMS_REGISTER_LIST` or `NOVA_ENUMS_REGISTER_SEQ`:
```
NOVA_ENUMS_REGISTER_SEQ( foo, (a)(b)(c) )
// or
NOVA_ENUMS_REGISTER_LIST( foo, (a, (b, (c, BOOST_PP_NIL))));
```
