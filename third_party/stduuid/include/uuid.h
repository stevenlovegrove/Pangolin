#pragma once

#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <array>
#include <string_view>
#include <iterator>
#include <random>
#include <memory>
#include <functional>
#include <type_traits>
#include <assert.h>

#ifdef STDUUID_USE_GSL
#  include <gsl/span>
#endif

#ifdef _WIN32
#  include <objbase.h>
#elif defined(__linux__) || defined(__unix__)
#  include <uuid/uuid.h>
#elif defined(__APPLE__)
#  include <CoreFoundation/CFUUID.h>
#endif

namespace uuids
{
   namespace detail
   {
      template <typename TChar>
      constexpr inline unsigned char hex2char(TChar const ch)
      {
         if (ch >= static_cast<TChar>('0') && ch <= static_cast<TChar>('9'))
            return ch - static_cast<TChar>('0');
         if (ch >= static_cast<TChar>('a') && ch <= static_cast<TChar>('f'))
            return 10 + ch - static_cast<TChar>('a');
         if (ch >= static_cast<TChar>('A') && ch <= static_cast<TChar>('F'))
            return 10 + ch - static_cast<TChar>('A');
         return 0;
      }

      template <typename TChar>
      constexpr inline bool is_hex(TChar const ch)
      {
         return
            (ch >= static_cast<TChar>('0') && ch <= static_cast<TChar>('9')) ||
            (ch >= static_cast<TChar>('a') && ch <= static_cast<TChar>('f')) ||
            (ch >= static_cast<TChar>('A') && ch <= static_cast<TChar>('F'));
      }

      template <typename TChar>
      constexpr inline unsigned char hexpair2char(TChar const a, TChar const b)
      {
         return (hex2char(a) << 4) | hex2char(b);
      }

      class sha1
      {
      public:
         using digest32_t = uint32_t[5];
         using digest8_t = uint8_t[20];

         static constexpr unsigned int block_bytes = 64;

         inline static uint32_t left_rotate(uint32_t value, size_t const count) 
         {
            return (value << count) ^ (value >> (32 - count));
         }

         sha1() { reset(); }

         void reset() 
         {
            m_digest[0] = 0x67452301;
            m_digest[1] = 0xEFCDAB89;
            m_digest[2] = 0x98BADCFE;
            m_digest[3] = 0x10325476;
            m_digest[4] = 0xC3D2E1F0;
            m_blockByteIndex = 0;
            m_byteCount = 0;
         }

         void process_byte(uint8_t octet) 
         {
            this->m_block[this->m_blockByteIndex++] = octet;
            ++this->m_byteCount;
            if (m_blockByteIndex == block_bytes)
            {
               this->m_blockByteIndex = 0;
               process_block();
            }
         }

         void process_block(void const * const start, void const * const end) 
         {
            const uint8_t* begin = static_cast<const uint8_t*>(start);
            const uint8_t* finish = static_cast<const uint8_t*>(end);
            while (begin != finish) 
            {
               process_byte(*begin);
               begin++;
            }
         }

         void process_bytes(void const * const data, size_t const len)
         {
            const uint8_t* block = static_cast<const uint8_t*>(data);
            process_block(block, block + len);
         }

         uint32_t const * get_digest(digest32_t digest) 
         {
            size_t const bitCount = this->m_byteCount * 8;
            process_byte(0x80);
            if (this->m_blockByteIndex > 56) {
               while (m_blockByteIndex != 0) {
                  process_byte(0);
               }
               while (m_blockByteIndex < 56) {
                  process_byte(0);
               }
            }
            else {
               while (m_blockByteIndex < 56) {
                  process_byte(0);
               }
            }
            process_byte(0);
            process_byte(0);
            process_byte(0);
            process_byte(0);
            process_byte(static_cast<unsigned char>((bitCount >> 24) & 0xFF));
            process_byte(static_cast<unsigned char>((bitCount >> 16) & 0xFF));
            process_byte(static_cast<unsigned char>((bitCount >> 8) & 0xFF));
            process_byte(static_cast<unsigned char>((bitCount) & 0xFF));

            memcpy(digest, m_digest, 5 * sizeof(uint32_t));
            return digest;
         }

         uint8_t const * get_digest_bytes(digest8_t digest) 
         {
            digest32_t d32;
            get_digest(d32);
            size_t di = 0;
            digest[di++] = ((d32[0] >> 24) & 0xFF);
            digest[di++] = ((d32[0] >> 16) & 0xFF);
            digest[di++] = ((d32[0] >> 8) & 0xFF);
            digest[di++] = ((d32[0]) & 0xFF);

            digest[di++] = ((d32[1] >> 24) & 0xFF);
            digest[di++] = ((d32[1] >> 16) & 0xFF);
            digest[di++] = ((d32[1] >> 8) & 0xFF);
            digest[di++] = ((d32[1]) & 0xFF);

            digest[di++] = ((d32[2] >> 24) & 0xFF);
            digest[di++] = ((d32[2] >> 16) & 0xFF);
            digest[di++] = ((d32[2] >> 8) & 0xFF);
            digest[di++] = ((d32[2]) & 0xFF);

            digest[di++] = ((d32[3] >> 24) & 0xFF);
            digest[di++] = ((d32[3] >> 16) & 0xFF);
            digest[di++] = ((d32[3] >> 8) & 0xFF);
            digest[di++] = ((d32[3]) & 0xFF);

            digest[di++] = ((d32[4] >> 24) & 0xFF);
            digest[di++] = ((d32[4] >> 16) & 0xFF);
            digest[di++] = ((d32[4] >> 8) & 0xFF);
            digest[di++] = ((d32[4]) & 0xFF);

            return digest;
         }

      private:
         void process_block() 
         {
            uint32_t w[80];
            for (size_t i = 0; i < 16; i++) {
               w[i] = (m_block[i * 4 + 0] << 24);
               w[i] |= (m_block[i * 4 + 1] << 16);
               w[i] |= (m_block[i * 4 + 2] << 8);
               w[i] |= (m_block[i * 4 + 3]);
            }
            for (size_t i = 16; i < 80; i++) {
               w[i] = left_rotate((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]), 1);
            }

            uint32_t a = m_digest[0];
            uint32_t b = m_digest[1];
            uint32_t c = m_digest[2];
            uint32_t d = m_digest[3];
            uint32_t e = m_digest[4];

            for (std::size_t i = 0; i < 80; ++i) 
            {
               uint32_t f = 0;
               uint32_t k = 0;

               if (i < 20) {
                  f = (b & c) | (~b & d);
                  k = 0x5A827999;
               }
               else if (i < 40) {
                  f = b ^ c ^ d;
                  k = 0x6ED9EBA1;
               }
               else if (i < 60) {
                  f = (b & c) | (b & d) | (c & d);
                  k = 0x8F1BBCDC;
               }
               else {
                  f = b ^ c ^ d;
                  k = 0xCA62C1D6;
               }
               uint32_t temp = left_rotate(a, 5) + f + e + k + w[i];
               e = d;
               d = c;
               c = left_rotate(b, 30);
               b = a;
               a = temp;
            }

            m_digest[0] += a;
            m_digest[1] += b;
            m_digest[2] += c;
            m_digest[3] += d;
            m_digest[4] += e;
         }

      private:
         digest32_t m_digest;
         uint8_t m_block[64];
         size_t m_blockByteIndex;
         size_t m_byteCount;
      };
   }

   // UUID format https://tools.ietf.org/html/rfc4122
   // Field	                     NDR Data Type	   Octet #	Note
   // --------------------------------------------------------------------------------------------------------------------------
   // time_low	                  unsigned long	   0 - 3	   The low field of the timestamp.
   // time_mid	                  unsigned short	   4 - 5	   The middle field of the timestamp.
   // time_hi_and_version	      unsigned short	   6 - 7	   The high field of the timestamp multiplexed with the version number.
   // clock_seq_hi_and_reserved	unsigned small	   8	      The high field of the clock sequence multiplexed with the variant.
   // clock_seq_low	            unsigned small	   9	      The low field of the clock sequence.
   // node	                     character	      10 - 15	The spatially unique node identifier.
   // --------------------------------------------------------------------------------------------------------------------------
   // 0                   1                   2                   3
   //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   // |                          time_low                             |
   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   // |       time_mid                |         time_hi_and_version   |
   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   // |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   // |                         node (2-5)                            |
   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   // indicated by a bit pattern in octet 8, marked with N in xxxxxxxx-xxxx-xxxx-Nxxx-xxxxxxxxxxxx
   enum class uuid_variant
   {
      // NCS backward compatibility (with the obsolete Apollo Network Computing System 1.5 UUID format)
      // N bit pattern: 0xxx
      // > the first 6 octets of the UUID are a 48-bit timestamp (the number of 4 microsecond units of time since 1 Jan 1980 UTC);
      // > the next 2 octets are reserved;
      // > the next octet is the "address family"; 
      // > the final 7 octets are a 56-bit host ID in the form specified by the address family
      ncs,
      
      // RFC 4122/DCE 1.1 
      // N bit pattern: 10xx
      // > big-endian byte order
      rfc,
      
      // Microsoft Corporation backward compatibility
      // N bit pattern: 110x
      // > little endian byte order
      // > formely used in the Component Object Model (COM) library      
      microsoft,
      
      // reserved for possible future definition
      // N bit pattern: 111x      
      reserved
   };

   struct uuid_error : public std::runtime_error
   {
#ifdef STDUUID_USE_GSL
      explicit uuid_error(std::string_view message)
         : std::runtime_error(message.data())
      {
      }
#endif

      explicit uuid_error(char const * message)
         : std::runtime_error(message)
      {
      }
   };

   // indicated by a bit pattern in octet 6, marked with M in xxxxxxxx-xxxx-Mxxx-xxxx-xxxxxxxxxxxx
   enum class uuid_version
   {
      none = 0, // only possible for nil or invalid uuids
      time_based = 1,  // The time-based version specified in RFC 4122
      dce_security = 2,  // DCE Security version, with embedded POSIX UIDs.
      name_based_md5 = 3,  // The name-based version specified in RFS 4122 with MD5 hashing
      random_number_based = 4,  // The randomly or pseudo-randomly generated version specified in RFS 4122
      name_based_sha1 = 5   // The name-based version specified in RFS 4122 with SHA1 hashing
   };

   struct uuid
   {
      struct uuid_const_iterator
      {
         using self_type         = uuid_const_iterator;
         using value_type        = uint8_t;
         using reference         = uint8_t const &;
         using pointer           = uint8_t const *;
         using iterator_category = std::random_access_iterator_tag;
         using difference_type   = ptrdiff_t;

      protected:
         pointer ptr = nullptr;
         size_t  index = 0;

         bool compatible(self_type const & other) const noexcept
         {
            return ptr == other.ptr;
         }

      public:
         constexpr explicit uuid_const_iterator(pointer ptr, size_t const index) :
            ptr(ptr), index(index)
         {
         }

         uuid_const_iterator(uuid_const_iterator const & o) = default;
         uuid_const_iterator& operator=(uuid_const_iterator const & o) = default;
         ~uuid_const_iterator() = default;

         self_type & operator++ ()
         {
            if (index >= 16)
               throw std::out_of_range("Iterator cannot be incremented past the end of the data.");
            ++index;
            return *this;
         }

         self_type operator++ (int)
         {
            self_type tmp = *this;
            ++*this;
            return tmp;
         }

         bool operator== (self_type const & other) const
         {
            assert(compatible(other));
            return index == other.index;
         }

         bool operator!= (self_type const & other) const
         {
            return !(*this == other);
         }

         reference operator* () const
         {
            if (ptr == nullptr)
               throw std::bad_function_call();
            return *(ptr + index);
         }

         reference operator-> () const
         {
            if (ptr == nullptr)
               throw std::bad_function_call();
            return *(ptr + index);
         }

         uuid_const_iterator() = default;

         self_type & operator--()
         {
            if (index <= 0)
               throw std::out_of_range("Iterator cannot be decremented past the beginning of the data.");
            --index;
            return *this;
         }

         self_type operator--(int)
         {
            self_type tmp = *this;
            --*this;
            return tmp;
         }

         self_type operator+(difference_type offset) const
         {
            self_type tmp = *this;
            return tmp += offset;
         }

         self_type operator-(difference_type offset) const
         {
            self_type tmp = *this;
            return tmp -= offset;
         }

         difference_type operator-(self_type const & other) const
         {
            assert(compatible(other));
            return (index - other.index);
         }

         bool operator<(self_type const & other) const
         {
            assert(compatible(other));
            return index < other.index;
         }

         bool operator>(self_type const & other) const
         {
            return other < *this;
         }

         bool operator<=(self_type const & other) const
         {
            return !(other < *this);
         }

         bool operator>=(self_type const & other) const
         {
            return !(*this < other);
         }

         self_type & operator+=(difference_type const offset)
         {
            if (static_cast<difference_type>(index) + offset < 0 ||
               static_cast<difference_type>(index) + offset > 16)
               throw std::out_of_range("Iterator cannot be incremented outside data bounds.");

            index += offset;
            return *this;
         }

         self_type & operator-=(difference_type const offset)
         {
            return *this += -offset;
         }

         value_type const & operator[](difference_type const offset) const
         {
            return (*(*this + offset));
         }
      };

      using value_type = uint8_t;

   public:
      constexpr uuid() noexcept : data({}) {};

#ifdef STDUUID_USE_GSL
      explicit uuid(gsl::span<value_type, 16> bytes)
      {
         std::copy(std::cbegin(bytes), std::cend(bytes), std::begin(data));
      }
#endif
      
      template<typename ForwardIterator>
      explicit uuid(ForwardIterator first, ForwardIterator last)
      {
         if (std::distance(first, last) == 16)
            std::copy(first, last, std::begin(data));
      }
      
      constexpr uuid_variant variant() const noexcept
      {
         if ((data[8] & 0x80) == 0x00)
            return uuid_variant::ncs;
         else if ((data[8] & 0xC0) == 0x80)
            return uuid_variant::rfc;
         else if ((data[8] & 0xE0) == 0xC0)
            return uuid_variant::microsoft;
         else
            return uuid_variant::reserved;
      }

      constexpr uuid_version version() const noexcept
      {
         if ((data[6] & 0xF0) == 0x10)
            return uuid_version::time_based;
         else if ((data[6] & 0xF0) == 0x20)
            return uuid_version::dce_security;
         else if ((data[6] & 0xF0) == 0x30)
            return uuid_version::name_based_md5;
         else if ((data[6] & 0xF0) == 0x40)
            return uuid_version::random_number_based;
         else if ((data[6] & 0xF0) == 0x50)
            return uuid_version::name_based_sha1;
         else
            return uuid_version::none;
      }

      constexpr std::size_t size() const noexcept { return 16; }

      constexpr bool is_nil() const noexcept
      {
         for (size_t i = 0; i < data.size(); ++i) if (data[i] != 0) return false;
         return true;
      }

      void swap(uuid & other) noexcept
      {
         data.swap(other.data);
      }

      constexpr uuid_const_iterator begin() const noexcept { return uuid_const_iterator(&data[0], 0); }
      constexpr uuid_const_iterator end() const noexcept { return uuid_const_iterator(&data[0], 16); }

#ifdef STDUUID_USE_GSL
      inline gsl::span<std::byte const, 16> as_bytes() const
      {
         return gsl::span<std::byte const, 16>(reinterpret_cast<std::byte const*>(data.data()), 16);
      }
#endif

      template <typename TChar>
      static uuid from_string(TChar const * const str, size_t const size)
      {
         TChar digit = 0;
         bool firstDigit = true;
         int hasBraces = 0;
         size_t index = 0;
         std::array<uint8_t, 16> data{ { 0 } };

         if (str == nullptr || size == 0)
            throw uuid_error{ "Wrong uuid format" };

         if (str[0] == static_cast<TChar>('{'))
            hasBraces = 1;
         if (hasBraces && str[size - 1] != static_cast<TChar>('}'))
            throw uuid_error{ "Wrong uuid format" };

         for (size_t i = hasBraces; i < size - hasBraces; ++i)
         {
            if (str[i] == static_cast<TChar>('-')) continue;

            if (index >= 16 || !detail::is_hex(str[i]))
            {
               throw uuid_error{ "Wrong uuid format" };
            }

            if (firstDigit)
            {
               digit = str[i];
               firstDigit = false;
            }
            else
            {
               data[index++] = detail::hexpair2char(digit, str[i]);
               firstDigit = true;
            }
         }

         if (index < 16)
         {
            throw uuid_error{ "Wrong uuid format" };
         }

         return uuid{ std::cbegin(data), std::cend(data) };
      }

#ifdef STDUUID_USE_GSL
      static uuid from_string(std::string_view str)
      {
         return from_string(str.data(), str.size());
      }

      static uuid from_string(std::wstring_view str)
      {
         return from_string(str.data(), str.size());
      }
#endif

   private:
      std::array<value_type, 16> data{ { 0 } };

      friend bool operator==(uuid const & lhs, uuid const & rhs) noexcept;
      friend bool operator<(uuid const & lhs, uuid const & rhs) noexcept;

      template <class Elem, class Traits>
      friend std::basic_ostream<Elem, Traits> & operator<<(std::basic_ostream<Elem, Traits> &s, uuid const & id);  
   };

   inline bool operator== (uuid const& lhs, uuid const& rhs) noexcept
   {
      return lhs.data == rhs.data;
   }

   inline bool operator!= (uuid const& lhs, uuid const& rhs) noexcept
   {
      return !(lhs == rhs);
   }

   inline bool operator< (uuid const& lhs, uuid const& rhs) noexcept
   {
      return lhs.data < rhs.data;
   }

   template <class Elem, class Traits>
   std::basic_ostream<Elem, Traits> & operator<<(std::basic_ostream<Elem, Traits> &s, uuid const & id)
   {
      return s << std::hex << std::setfill(static_cast<Elem>('0'))
         << std::setw(2) << (int)id.data[0]
         << std::setw(2) << (int)id.data[1]
         << std::setw(2) << (int)id.data[2]
         << std::setw(2) << (int)id.data[3]
         << '-'
         << std::setw(2) << (int)id.data[4]
         << std::setw(2) << (int)id.data[5]
         << '-'
         << std::setw(2) << (int)id.data[6]
         << std::setw(2) << (int)id.data[7]
         << '-'
         << std::setw(2) << (int)id.data[8]
         << std::setw(2) << (int)id.data[9]
         << '-'
         << std::setw(2) << (int)id.data[10]
         << std::setw(2) << (int)id.data[11]
         << std::setw(2) << (int)id.data[12]
         << std::setw(2) << (int)id.data[13]
         << std::setw(2) << (int)id.data[14]
         << std::setw(2) << (int)id.data[15];
   }

   inline std::string to_string(uuid const & id)
   {
      std::stringstream sstr;
      sstr << id;
      return sstr.str();
   }

   inline std::wstring to_wstring(uuid const & id)
   {
      std::wstringstream sstr;
      sstr << id;
      return sstr.str();
   }

   inline void swap(uuids::uuid & lhs, uuids::uuid & rhs)
   {
      lhs.swap(rhs);
   }

   class uuid_system_generator
   {
   public:
      using result_type = uuid;

      uuid operator()()
      {
#ifdef _WIN32

         GUID newId;
         ::CoCreateGuid(&newId);

         std::array<uint8_t, 16> bytes =
         { {
               (unsigned char)((newId.Data1 >> 24) & 0xFF),
               (unsigned char)((newId.Data1 >> 16) & 0xFF),
               (unsigned char)((newId.Data1 >> 8) & 0xFF),
               (unsigned char)((newId.Data1) & 0xFF),

               (unsigned char)((newId.Data2 >> 8) & 0xFF),
               (unsigned char)((newId.Data2) & 0xFF),

               (unsigned char)((newId.Data3 >> 8) & 0xFF),
               (unsigned char)((newId.Data3) & 0xFF),

               newId.Data4[0],
               newId.Data4[1],
               newId.Data4[2],
               newId.Data4[3],
               newId.Data4[4],
               newId.Data4[5],
               newId.Data4[6],
               newId.Data4[7]
            } };

         return uuid{ std::begin(bytes), std::end(bytes) };

#elif defined(__linux__) || defined(__unix__)

         uuid_t id;
         uuid_generate(id);

         std::array<uint8_t, 16> bytes =
         { {
               id[0],
               id[1],
               id[2],
               id[3],
               id[4],
               id[5],
               id[6],
               id[7],
               id[8],
               id[9],
               id[10],
               id[11],
               id[12],
               id[13],
               id[14],
               id[15]
            } };

         return uuid{ std::begin(bytes), std::end(bytes) };

#elif defined(__APPLE__)
         auto newId = CFUUIDCreate(NULL);
         auto bytes = CFUUIDGetUUIDBytes(newId);
         CFRelease(newId);

         std::array<uint8_t, 16> arrbytes =
         { {
               bytes.byte0,
               bytes.byte1,
               bytes.byte2,
               bytes.byte3,
               bytes.byte4,
               bytes.byte5,
               bytes.byte6,
               bytes.byte7,
               bytes.byte8,
               bytes.byte9,
               bytes.byte10,
               bytes.byte11,
               bytes.byte12,
               bytes.byte13,
               bytes.byte14,
               bytes.byte15
            } };
         return uuid{ std::begin(arrbytes), std::end(arrbytes) };
#elif
         return uuid{};
#endif
      }
   };

   template <typename UniformRandomNumberGenerator>
   class basic_uuid_random_generator 
   {
   public:
      using result_type = uuid;

      basic_uuid_random_generator()
         :generator(new UniformRandomNumberGenerator)
      {
         std::random_device rd;
         generator->seed(rd());
      }

      explicit basic_uuid_random_generator(UniformRandomNumberGenerator& gen) :
         generator(&gen, [](auto) {}) {}
      explicit basic_uuid_random_generator(UniformRandomNumberGenerator* gen) :
         generator(gen, [](auto) {}) {}

      uuid operator()()
      {
         uint8_t bytes[16];
         for (int i = 0; i < 16; i += 4)
            *reinterpret_cast<uint32_t*>(bytes + i) = distribution(*generator);

         // variant must be 10xxxxxx
         bytes[8] &= 0xBF;
         bytes[8] |= 0x80;

         // version must be 0100xxxx
         bytes[6] &= 0x4F;
         bytes[6] |= 0x40;

         return uuid{std::begin(bytes), std::end(bytes)};
      }

   private:
      std::uniform_int_distribution<uint32_t>  distribution;
      std::shared_ptr<UniformRandomNumberGenerator> generator;
   };

   using uuid_random_generator = basic_uuid_random_generator<std::mt19937>;
   
   class uuid_name_generator
   {
   public:
      using result_type = uuid;

      explicit uuid_name_generator(uuid const& namespace_uuid) noexcept
         : nsuuid(namespace_uuid)
      {}

#ifdef STDUUID_USE_GSL
      uuid operator()(std::string_view name)
      {
         reset();
         process_characters(name.data(), name.size());
         return make_uuid();
      }

      uuid operator()(std::wstring_view name)
      {
         reset();
         process_characters(name.data(), name.size());
         return make_uuid();
      }
#endif

   private:
      void reset() 
      {
         hasher.reset();
         uint8_t bytes[16];
         std::copy(std::begin(nsuuid), std::end(nsuuid), bytes);
         hasher.process_bytes(bytes, 16);
      }
      
      template <typename char_type,
                typename = std::enable_if_t<std::is_integral<char_type>::value>>
      void process_characters(char_type const * const characters, size_t const count)
      {
         for (size_t i = 0; i < count; i++) 
         {
            uint32_t c = characters[i];
            hasher.process_byte(static_cast<unsigned char>((c >> 0) & 0xFF));
            hasher.process_byte(static_cast<unsigned char>((c >> 8) & 0xFF));
            hasher.process_byte(static_cast<unsigned char>((c >> 16) & 0xFF));
            hasher.process_byte(static_cast<unsigned char>((c >> 24) & 0xFF));
         }
      }

      void process_characters(const char * const characters, size_t const count)
      {
         hasher.process_bytes(characters, count);
      }

      uuid make_uuid()
      {
         detail::sha1::digest8_t digest;
         hasher.get_digest_bytes(digest);

         // variant must be 0b10xxxxxx
         digest[8] &= 0xBF;
         digest[8] |= 0x80;

         // version must be 0b0101xxxx
         digest[6] &= 0x5F;
         digest[6] |= 0x50;

         return uuid{ digest, digest + 16 };
      }

   private:
      uuid nsuuid;
      detail::sha1 hasher;
   }; 
}

namespace std
{
   template <>
   struct hash<uuids::uuid>
   {
      using argument_type = uuids::uuid;
      using result_type   = std::size_t;

      result_type operator()(argument_type const &uuid) const
      {
         std::hash<std::string> hasher;
         return static_cast<result_type>(hasher(uuids::to_string(uuid)));
      }
   };
}
