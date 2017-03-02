/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pangolin/platform.h>

namespace pangolin
{

using pangoTagType = uint32_t;

const static std::string PANGO_MAGIC = "PANGO";

const unsigned int TAG_LENGTH = 3;

#define PANGO_TAG(a,b,c) ( (c<<16) | (b<<8) | a)
const uint32_t TAG_PANGO_HDR    = PANGO_TAG('L', 'I', 'N');
const uint32_t TAG_PANGO_MAGIC  = PANGO_TAG('P', 'A', 'N');
const uint32_t TAG_PANGO_SYNC   = PANGO_TAG('S', 'Y', 'N');
const uint32_t TAG_PANGO_STATS  = PANGO_TAG('S', 'T', 'A');
const uint32_t TAG_PANGO_FOOTER = PANGO_TAG('F', 'T', 'R');
const uint32_t TAG_ADD_SOURCE   = PANGO_TAG('S', 'R', 'C');
const uint32_t TAG_SRC_JSON     = PANGO_TAG('J', 'S', 'N');
const uint32_t TAG_SRC_PACKET   = PANGO_TAG('P', 'K', 'T');
const uint32_t TAG_END          = PANGO_TAG('E', 'N', 'D');
#undef PANGO_TAG

inline std::string tagName(int v)
{
    char b[4];
    b[0] = v&0xff;
    b[1] = (v>>8)&0xff;
    b[2] = (v>>16)&0xff;
    b[3] = 0x00;
    return std::string(b);
}

#define pss_src_driver "driver"
#define pss_src_id "id"
#define pss_src_info "info"
#define pss_src_uri "uri"
#define pss_src_packet "packet"
#define pss_src_version         "version"
#define pss_pkt_alignment_bytes "alignment_bytes"
#define pss_pkt_definitions "definitions"
#define pss_pkt_size_bytes  "size_bytes"
#define pss_pkt_format_written "format_written"

}
