#define CATCH_CONFIG_MAIN
#if __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <pangolin/video/video.h>
#include <pangolin/factory/factory_registry.h>

TEST_CASE( "Loading built in video driver" ) {
    // If this throws, we've probably messed up the factory loading stuff again...
    auto video = pangolin::OpenVideo("test:[size=123x345,n=1,fmt=RGB24]//");

    REQUIRE(video.get());
    REQUIRE(video->SizeBytes() == 123*345*3);
    REQUIRE(video->Streams().size() == 1);
    REQUIRE(video->Streams()[0].PixFormat().format == "RGB24");
    REQUIRE(video->Streams()[0].Width() == 123);
    REQUIRE(video->Streams()[0].Height() == 345);

    std::unique_ptr<unsigned char[]> image(new unsigned char[video->SizeBytes()]);
    const bool success = video->GrabNext(image.get());
    REQUIRE(success);
}

TEST_CASE( "Error when providing the wrong arguments" )
{
    REQUIRE_THROWS_AS(pangolin::OpenVideo("test:[width=123,height=345,n=3,fmt=RGB24]//"), pangolin::FactoryRegistry::ParameterMismatchException);
}
