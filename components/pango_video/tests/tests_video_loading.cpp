#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/video.h>

#include <catch2/catch_test_macros.hpp>

using namespace pangolin;

TEST_CASE("Loading built in video driver")
{
  // If this throws, we've probably messed up the factory loading stuff again...
  auto video = OpenVideo("test:[size=123x345,n=1,fmt=RGB24]//");

  REQUIRE(video.get());
  REQUIRE(video->SizeBytes() == 123 * 345 * 3);
  REQUIRE(video->Streams().size() == 1);
  REQUIRE(ToString(video->Streams()[0].format()) == std::string("RGB24"));
  REQUIRE(video->Streams()[0].shape().width() == 123);
  REQUIRE(video->Streams()[0].shape().height() == 345);

  std::unique_ptr<unsigned char[]> image(new unsigned char[video->SizeBytes()]);
  const bool success = video->GrabNext(image.get());
  REQUIRE(success);
}

TEST_CASE("Error when providing the wrong arguments")
{
  REQUIRE_THROWS_AS(
      OpenVideo("test:[width=123,height=345,n=3,fmt=RGB24]//"),
      pangolin::FactoryRegistry::ParameterMismatchException);
}
