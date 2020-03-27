#include "gtest/gtest.h"
#include <pangolin/video/video.h>
#include <string>
#include <pangolin/video/video_exception.h>
#include <pangolin/factory/factory_registry.h>
#include <functional>
#include <limits>
#include <pangolin/video/iostream_operators.h>

template<typename T>
inline void ExpectExceptionWithMessageFromAction(std::function<void()> action, 
    const std::string& exceptionMessageBegin)
{
    try
    {
        action();
        FAIL() << "The action succeeded when it should have failed.";
    }
    catch(const T& ex)
    {
        std::string exceptionMessage = std::string(ex.what());
        std::string exceptionMessageRelevantPortion 
            = exceptionMessage.substr(0, exceptionMessageBegin.size());
        bool beginsWithExpectedMessage 
            = exceptionMessageRelevantPortion.compare(exceptionMessageBegin) == 0;
        EXPECT_TRUE(beginsWithExpectedMessage);
    }
    catch(...)
    {
        FAIL() << "Another kind of exception was thrown to the one expected";
    }
}

TEST(UriTest, EmptyUriTest) {

    EXPECT_THROW(pangolin::OpenVideo(""), pangolin::VideoException);
}

TEST(UriTest, UriBeginningWithDotInterpretedAsImage) {
    auto testAction = [](){pangolin::OpenVideo(".");};
    ExpectExceptionWithMessageFromAction<std::runtime_error>(testAction,
         "Unsupported image file type");
}

TEST(UriTest, MissingClosingBracketCausesException) {
    const std::string rawUri = "abc:[...";
    auto testAction = [&](){pangolin::ParseUri(rawUri);};
    ExpectExceptionWithMessageFromAction<std::runtime_error>(testAction,
         "Unable to parse URI: '" + rawUri + "'");
}

TEST(UriTest, UriDimensionLegalSeparatorCharacter) {

    using char_t = std::string::value_type;
    using char_nl = std::numeric_limits<char_t>;
    for(char_t c = char_nl::min(); c < char_nl::max(); ++c)
    {
        if(c == '\0' || c == '=' || c == ']' || c == ',' || (c >= '0' && c <= '9'))
        {
            continue;
        }

        const int dimWidth = 123, dimHeight = 456;
        std::string encodedDim = std::to_string(dimWidth) + c 
            + std::to_string(dimHeight);
        std::string fullUri = "abc:[size=" + encodedDim + ",other=value]";
        pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
        
        EXPECT_EQ(pangoUri.params.size(), 2) << "Could not parse: " 
            << encodedDim << ", c was: " << std::to_string(c);
        EXPECT_EQ( pangoUri.params[0].first, "size");
        EXPECT_EQ( pangoUri.params[1].first, "other");

        pangolin::ImageDim dim 
            = pangoUri.Get<pangolin::ImageDim>("size", pangolin::ImageDim(0,0));
        EXPECT_EQ( dim.x, dimWidth);
        EXPECT_EQ( dim.y, dimHeight);
        
        std::string otherValue = pangoUri.Get<std::string>("other", "");
        EXPECT_EQ( otherValue, "value");

    }
}

TEST(UriTest, UriEqualsSignCharacterInValueTruncatesTheRest) {
        std::string fullUri = "abc:[key=value=notfound,key2=value2]";
        pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
        
        EXPECT_EQ(pangoUri.params.size(), 2) 
            << "parsing option list should have precendence"
            << " over parsing individual option";
        EXPECT_EQ( pangoUri.params[0].first, "key");
        EXPECT_EQ( pangoUri.params[1].first, "key2");

        std::string truncated = pangoUri.Get<std::string>("key", "");
        EXPECT_EQ( truncated, "value");
        
        std::string otherValue = pangoUri.Get<std::string>("key2", "");
        EXPECT_EQ( otherValue, "value2");
}

TEST(UriTest, UriMultipleOccurrencesOfOptionOverridesPrevious) {
        std::string fullUri = "abc:[key=value,key=value2]";
        pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
        
        EXPECT_EQ(pangoUri.params.size(), 2) << "there are two";
        EXPECT_EQ( pangoUri.params[0].first, "key");
        EXPECT_EQ( pangoUri.params[1].first, "key");

        std::string truncated = pangoUri.Get<std::string>("key", "");
        EXPECT_EQ( truncated, "value2");
}


TEST(UriTest, UriUrlContainsEverythingAfterSeparator) {
    std::string expectedUrl = "abc:123[],=";
    std::string fullUri = "abc:[key=value,key=value2]//" + expectedUrl;
    pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
    EXPECT_EQ( pangoUri.url, expectedUrl);
}

TEST(UriTest, UriInvalidCharactersAfterClosedBracketAreIgnored) {
    std::string expectedUrl = "abc:123[],=";
    std::string fullUri = "abc:[key=value,key=value2]xyz//" + expectedUrl;
    pangolin::Uri pangoUri = pangolin::ParseUri(fullUri);
    EXPECT_EQ( pangoUri.url, expectedUrl);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
