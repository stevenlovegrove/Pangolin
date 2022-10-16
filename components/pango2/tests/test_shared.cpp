#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <pangolin/experimental/shared.h>

using namespace farm_ng;

struct ErrorTag {};

struct Foo {
    Foo(int val) 
    : val_(val)
    {}
    Foo(ErrorTag)  { throw std::runtime_error("ConstructorThrows"); }
    int test_method() { return val_; }

    int val_;
};

ExpectShared<Foo> makeSomething(int val, bool succeed)
{
    if(succeed) {
        return tryMakeShared<Foo>(val);
    }else{
        return tryMakeShared<Foo>(ErrorTag());
    }
}

TEST_CASE("Simple checks")
{
    if( auto maybe_int = tryMakeShared<int>(5) ) {
        int t = **maybe_int + 10;
        CHECK(t == 15);
    }else{
        CHECK(false);
    }

    {
        // + is a checked dereference. Will throw.
        auto value = +tryMakeShared<int>(4);  
        int t = *value + 17;
        CHECK(t == 21);
    }
}

TEST_CASE("Rethrow")
{
    // User doesn't deal with error so we must throw
    if( auto t = makeSomething(42, true) ) {
        CHECK( (*t)->test_method() == 42);
    }else{
        CHECK(false);
    }

    if( auto t = makeSomething(73, false) ) {
        CHECK(false);
    }else{
        CHECK_FALSE(t.has_value());
    }

    REQUIRE_THROWS( [](){
        auto t = +makeSomething(73, false);
    }() );
}
