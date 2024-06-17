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

TEST_CASE("Missing Closing Bracket Causes Exception")
{
    const std::string rawUri = "abc:[...";
    auto testAction = [&](){pangolin::ParseUri(rawUri);};
    ExpectExceptionWithMessageFromAction<std::runtime_error>(testAction,
         "Unable to parse URI: '" + rawUri + "'");
}

TEST_CASE("Uri Equals Sign Character In Value Truncates The Rest")
{
    const std::string fullUri = "abc:[key=value=notfound,key2=value2]";
    const pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);

    // parsing option list should have precendence over parsing individual option
    REQUIRE(pangoUri.params.size() ==  2);

    REQUIRE( pangoUri.params[0].first == std::string("key"));
    REQUIRE( pangoUri.params[1].first == std::string("key2"));

    const std::string truncated = pangoUri.Get<std::string>("key", "");
    REQUIRE( truncated == std::string("value"));

    const std::string otherValue = pangoUri.Get<std::string>("key2", "");
    REQUIRE( otherValue == std::string("value2"));
}

TEST_CASE("Uri Multiple Occurrences Of Option Overrides Previous")
{
    const std::string fullUri = "abc:[key=value,key=value2]";
    const pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);

    REQUIRE(pangoUri.params.size() == 2);
    REQUIRE( pangoUri.params[0].first == std::string("key"));
    REQUIRE( pangoUri.params[1].first == std::string("key"));

    const std::string truncated = pangoUri.Get<std::string>("key", "");
    REQUIRE( truncated == std::string("value2"));
}


TEST_CASE("Uri Url Contains Everything After Separator")
{
    const std::string expectedUrl = "abc:123[],=";
    const std::string fullUri = "abc:[key=value,key=value2]//" + expectedUrl;
    const pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
    REQUIRE( pangoUri.url == expectedUrl);
}

TEST_CASE("Uri Invalid Characters After Closed Bracket Are Ignored")
{
    const std::string expectedUrl = "abc:123[],=";
    const std::string fullUri = "abc:[key=value,key=value2]xyz//" + expectedUrl;
    const pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
    REQUIRE( pangoUri.url == expectedUrl);
}
