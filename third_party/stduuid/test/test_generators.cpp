#include "uuid.h"
#include "catch.hpp"

#include <set>
#include <unordered_set>
#include <random>
#include <vector>

using namespace uuids;

TEST_CASE("Test default generator", "[gen][rand]")
{
   uuid const guid = uuids::uuid_random_generator{}();
   REQUIRE(!guid.is_nil());
   REQUIRE(guid.size() == 16);
   REQUIRE(guid.version() == uuids::uuid_version::random_number_based);
   REQUIRE(guid.variant() == uuids::uuid_variant::rfc);
}

TEST_CASE("Test random generator (default ctor)", "[gen][rand]")
{
   uuids::uuid_random_generator dgen;
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test random generator (conversion ctor w/ smart ptr)", "[gen][rand]")
{
   std::random_device rd;
   auto seed_data = std::array<int, std::mt19937::state_size> {};
   std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
   std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
   std::mt19937 generator(seq);

   uuids::uuid_random_generator dgen(&generator);
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test random generator (conversion ctor w/ ptr)", "[gen][rand]")
{
   std::random_device rd;
   auto seed_data = std::array<int, std::mt19937::state_size> {};
   std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
   std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
   auto generator = std::make_unique<std::mt19937>(seq);

   uuids::uuid_random_generator dgen(generator.get());
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test random generator (conversion ctor w/ ref)", "[gen][rand]")
{
   std::random_device rd;
   auto seed_data = std::array<int, std::mt19937::state_size> {};
   std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
   std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
   std::mt19937 generator(seq);

   uuids::uuid_random_generator dgen(generator);
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test basic random generator (default ctor) w/ ranlux48_base", "[gen][rand]")
{
   uuids::basic_uuid_random_generator<std::ranlux48_base> dgen;
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test basic random generator (conversion ctor w/ ptr) w/ ranlux48_base", "[gen][rand]")
{
   std::random_device rd;
   std::ranlux48_base generator(rd());

   uuids::basic_uuid_random_generator<std::ranlux48_base> dgen(&generator);
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test basic random generator (conversion ctor w/ smart ptr) w/ ranlux48_base", "[gen][rand]")
{
   std::random_device rd;
   auto generator = std::make_unique<std::ranlux48_base>(rd());

   uuids::basic_uuid_random_generator<std::ranlux48_base> dgen(generator.get());
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test basic random generator (conversion ctor w/ ref) w/ ranlux48_base", "[gen][rand]")
{
   std::random_device rd;
   std::ranlux48_base generator(rd());

   uuids::basic_uuid_random_generator<std::ranlux48_base> dgen(generator);
   auto id1 = dgen();
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen();
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::random_number_based);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
}

TEST_CASE("Test name generator", "[gen][name]")
{
   uuids::uuid_name_generator dgen(uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43"));
   auto id1 = dgen("john");
   REQUIRE(!id1.is_nil());
   REQUIRE(id1.size() == 16);
   REQUIRE(id1.version() == uuids::uuid_version::name_based_sha1);
   REQUIRE(id1.variant() == uuids::uuid_variant::rfc);

   auto id2 = dgen("jane");
   REQUIRE(!id2.is_nil());
   REQUIRE(id2.size() == 16);
   REQUIRE(id2.version() == uuids::uuid_version::name_based_sha1);
   REQUIRE(id2.variant() == uuids::uuid_variant::rfc);

   auto id3 = dgen("jane");
   REQUIRE(!id3.is_nil());
   REQUIRE(id3.size() == 16);
   REQUIRE(id3.version() == uuids::uuid_version::name_based_sha1);
   REQUIRE(id3.variant() == uuids::uuid_variant::rfc);

   auto id4 = dgen(L"jane");
   REQUIRE(!id4.is_nil());
   REQUIRE(id4.size() == 16);
   REQUIRE(id4.version() == uuids::uuid_version::name_based_sha1);
   REQUIRE(id4.variant() == uuids::uuid_variant::rfc);

   REQUIRE(id1 != id2);
   REQUIRE(id2 == id3);
   REQUIRE(id3 != id4);
}
