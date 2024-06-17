#define CATCH_CONFIG_MAIN
#if __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <string>
#include <limits>
#include <functional>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/video.h>
#include <pangolin/video/video_exception.h>
#include <pangolin/video/iostream_operators.h>

template<typename T>
inline void ExpectExceptionWithMessageFromAction(
    std::function<void()> action,
    const std::string& exceptionMessageBegin
) {
    try
    {
        action();

        FAIL("The action succeeded when it should have failed.");
    }
    catch(const T& ex)
    {
        std::string exceptionMessage = std::string(ex.what());
        std::string exceptionMessageRelevantPortion
            = exceptionMessage.substr(0, exceptionMessageBegin.size());
        REQUIRE(exceptionMessageRelevantPortion.compare(exceptionMessageBegin) == 0);
    }
    catch(...)
    {
        FAIL("Another kind of exception was thrown to the one expected");
    }
}

TEST_CASE("Empty Uri Test")
{
    REQUIRE_THROWS_AS(pangolin::OpenVideo(""), pangolin::VideoException);
}

TEST_CASE("Uri Dimension Legal Separator Character")
{

    using char_t = std::string::value_type;
    using char_nl = std::numeric_limits<char_t>;
    for(char_t c = char_nl::min(); c < char_nl::max(); ++c)
    {
        if(c == '\0' || c == '=' || c == ']' || c == ',' || (c >= '0' && c <= '9'))
        {
            continue;
        }

        const int dimWidth = 123, dimHeight = 456;
        const std::string encodedDim = std::to_string(dimWidth) + c
            + std::to_string(dimHeight);
        const std::string fullUri = "abc:[size=" + encodedDim + ",other=value]";
        const pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);

        REQUIRE(pangoUri.params.size() == 2);
        REQUIRE( pangoUri.params[0].first == std::string("size"));
        REQUIRE( pangoUri.params[1].first == std::string("other"));

        const pangolin::ImageDim dim
            = pangoUri.Get<pangolin::ImageDim>("size", pangolin::ImageDim(0,0));
        REQUIRE( dim.x == dimWidth);
        REQUIRE( dim.y == dimHeight);

        const std::string otherValue = pangoUri.Get<std::string>("other", "");
        REQUIRE( otherValue == std::string("value"));

    }
}
