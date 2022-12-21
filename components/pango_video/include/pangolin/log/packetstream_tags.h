#pragma once

#include <string>

namespace pangolin
{

using PangoTagType = uint32_t;

static const std::string PANGO_MAGIC = "PANGO";

static const std::string pss_src_driver = "driver";
static const std::string pss_src_id = "id";
static const std::string pss_src_info = "info";
static const std::string pss_src_uri = "uri";
static const std::string pss_src_packet = "packet";
static const std::string pss_src_version = "version";
static const std::string pss_pkt_alignment_bytes = "alignment_bytes";
static const std::string pss_pkt_definitions = "definitions";
static const std::string pss_pkt_size_bytes = "size_bytes";
static const std::string pss_pkt_format_written = "format_written";

unsigned int const TAG_LENGTH = 3;

#define PANGO_TAG(a, b, c) ((c << 16) | (b << 8) | a)
const PangoTagType TAG_PANGO_HDR = PANGO_TAG('L', 'I', 'N');
const PangoTagType TAG_PANGO_MAGIC = PANGO_TAG('P', 'A', 'N');
const PangoTagType TAG_PANGO_SYNC = PANGO_TAG('S', 'Y', 'N');
const PangoTagType TAG_PANGO_STATS = PANGO_TAG('S', 'T', 'A');
const PangoTagType TAG_PANGO_FOOTER = PANGO_TAG('F', 'T', 'R');
const PangoTagType TAG_ADD_SOURCE = PANGO_TAG('S', 'R', 'C');
const PangoTagType TAG_SRC_JSON = PANGO_TAG('J', 'S', 'N');
const PangoTagType TAG_SRC_PACKET = PANGO_TAG('P', 'K', 'T');
const PangoTagType TAG_END = PANGO_TAG('E', 'N', 'D');
#undef PANGO_TAG

inline std::string tagName(int v)
{
  char b[4];
  b[0] = v & 0xff;
  b[1] = (v >> 8) & 0xff;
  b[2] = (v >> 16) & 0xff;
  b[3] = 0x00;
  return std::string(b);
}

}  // namespace pangolin
