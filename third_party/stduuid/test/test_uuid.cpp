#include "uuid.h"
#include "catch.hpp"

#include <cstring>
#include <set>
#include <unordered_set>
#include <vector>
#include <iostream>

using namespace uuids;

TEST_CASE("Test default constructor", "[ctors]") 
{
   uuid empty;
   REQUIRE(empty.is_nil());
   REQUIRE(empty.size() == 16);
}

TEST_CASE("Test from_string(string_view)", "[parse]")
{
   using namespace std::string_literals;

   {
      auto str = "47183823-2574-4bfd-b411-99ed177d3e43"s;
      auto guid = uuids::uuid::from_string(str);
      REQUIRE(uuids::to_string(guid) == str);
   }

   {
      auto str = "{47183823-2574-4bfd-b411-99ed177d3e43}"s;
      auto guid = uuids::uuid::from_string(str);
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43");
   }

   {
      auto guid = uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43");
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43");
      REQUIRE(uuids::to_wstring(guid) == L"47183823-2574-4bfd-b411-99ed177d3e43");
   }

   {
      auto str = "4718382325744bfdb41199ed177d3e43"s;
      REQUIRE_NOTHROW(uuids::uuid::from_string(str));
   }
}

TEST_CASE("Test from_string(wstring_view)", "[parse]")
{
   using namespace std::string_literals;

   auto str = L"47183823-2574-4bfd-b411-99ed177d3e43"s;
   auto guid = uuids::uuid::from_string(str);
   REQUIRE(uuids::to_wstring(guid) == str);
}

TEST_CASE("Test from_string invalid format", "[parse]")
{
   using namespace std::string_literals;

   {
      auto str = ""s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }

   {
      auto str = "{}"s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }

   {
      auto str = "47183823-2574-4bfd-b411-99ed177d3e4"s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }

   {
      auto str = "47183823-2574-4bfd-b411-99ed177d3e430"s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }

   {
      auto str = "{47183823-2574-4bfd-b411-99ed177d3e43"s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }

   {
      auto str = "47183823-2574-4bfd-b411-99ed177d3e43}"s;
      REQUIRE_THROWS_AS(uuids::uuid::from_string(str), uuids::uuid_error);
   }
}

TEST_CASE("Test iterators constructor", "[ctors]")
{
   using namespace std::string_literals;

   {
      std::array<uuids::uuid::value_type, 16> arr{ {
            0x47, 0x18, 0x38, 0x23,
            0x25, 0x74,
            0x4b, 0xfd,
            0xb4, 0x11,
            0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43 } };

      uuid guid(std::begin(arr), std::end(arr));
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43"s);
   }

   {
      uuids::uuid::value_type arr[16] = {
         0x47, 0x18, 0x38, 0x23,
         0x25, 0x74,
         0x4b, 0xfd,
         0xb4, 0x11,
         0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43 };

      uuid guid(std::begin(arr), std::end(arr));
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43"s);
   }
}

TEST_CASE("Test equality", "[operators]")
{
   uuid empty;
   uuid guid = uuids::uuid_random_generator{}();

   REQUIRE(empty == empty);
   REQUIRE(guid == guid);
   REQUIRE(empty != guid);
}

TEST_CASE("Test comparison", "[operators]")
{
   auto empty = uuid{};
   uuids::uuid_random_generator gen;
   auto id = gen();

   REQUIRE(empty < id);

   std::set<uuids::uuid> ids{
      uuid{},
      gen(),
      gen(),
      gen(),
      gen()
   };

   REQUIRE(ids.size() == 5);
   REQUIRE(ids.find(uuid{}) != ids.end());
}

TEST_CASE("Test hashing", "[ops]")
{
   using namespace std::string_literals;
   auto str = "47183823-2574-4bfd-b411-99ed177d3e43"s;
   auto guid = uuids::uuid::from_string(str);

   auto h1 = std::hash<std::string>{};
   auto h2 = std::hash<uuid>{};
   REQUIRE(h1(str) == h2(guid));

   uuids::uuid_random_generator gen;

   std::unordered_set<uuids::uuid> ids{
      uuid{},
      gen(),
      gen(),
      gen(),
      gen()
   };

   REQUIRE(ids.size() == 5);
   REQUIRE(ids.find(uuid{}) != ids.end());
}

TEST_CASE("Test swap", "[ops]")
{
   uuid empty;
   uuid guid = uuids::uuid_random_generator{}();

   REQUIRE(empty.is_nil());
   REQUIRE(!guid.is_nil());

   std::swap(empty, guid);

   REQUIRE(!empty.is_nil());
   REQUIRE(guid.is_nil());

   empty.swap(guid);

   REQUIRE(empty.is_nil());
   REQUIRE(!guid.is_nil());
}

TEST_CASE("Test string conversion", "[ops]")
{
   uuid empty;
   REQUIRE(uuids::to_string(empty) == "00000000-0000-0000-0000-000000000000");
   REQUIRE(uuids::to_wstring(empty) == L"00000000-0000-0000-0000-000000000000");
}

TEST_CASE("Test iterators", "[iter]")
{
   std::array<uuids::uuid::value_type, 16> arr{ {
         0x47, 0x18, 0x38, 0x23,
         0x25, 0x74,
         0x4b, 0xfd,
         0xb4, 0x11,
         0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43
      } };

   {
      uuid guid(arr);
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43");

      size_t i = 0;
      for (auto const & b : guid)
      {
         REQUIRE(arr[i++] == b);
      }
   }

   {
      const uuid guid = uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43");
      REQUIRE(!guid.is_nil());
      REQUIRE(uuids::to_string(guid) == "47183823-2574-4bfd-b411-99ed177d3e43");

      size_t i = 0;
      for (auto const & b : guid)
      {
         REQUIRE(arr[i++] == b);
      }
   }
}

TEST_CASE("Test constexpr", "[const]")
{
   constexpr uuid empty;
   [[maybe_unused]] constexpr bool isnil = empty.is_nil();
   [[maybe_unused]] constexpr size_t size = empty.size();
   [[maybe_unused]] constexpr uuids::uuid_variant variant = empty.variant();
   [[maybe_unused]] constexpr uuid_version version = empty.version();
}

TEST_CASE("Test size", "[operators]")
{
   REQUIRE(sizeof(uuid) == 16);
}

TEST_CASE("Test assignment", "[ops]")
{
   auto id1 = uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43");
   auto id2 = id1;
   REQUIRE(id1 == id2);

   id1 = uuids::uuid::from_string("{fea43102-064f-4444-adc2-02cec42623f8}");
   REQUIRE(id1 != id2);

   auto id3 = std::move(id2);
   REQUIRE(uuids::to_string(id3) == "47183823-2574-4bfd-b411-99ed177d3e43");
}

TEST_CASE("Test trivial", "[trivial]")
{
   REQUIRE(std::is_trivially_copyable_v<uuids::uuid>);
}

TEST_CASE("Test as_bytes", "[ops]")
{
   std::array<uuids::uuid::value_type, 16> arr{ {
         0x47, 0x18, 0x38, 0x23,
         0x25, 0x74,
         0x4b, 0xfd,
         0xb4, 0x11,
         0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43
      } };

   {
      uuids::uuid id{ arr };
      REQUIRE(!id.is_nil());

      auto view = id.as_bytes();
      REQUIRE(memcmp(view.data(), arr.data(), arr.size()) == 0);
   }

   {
      const uuids::uuid id{ arr };
      REQUIRE(!id.is_nil());

      auto view = id.as_bytes();
      REQUIRE(memcmp(view.data(), arr.data(), arr.size()) == 0);
   }

}
