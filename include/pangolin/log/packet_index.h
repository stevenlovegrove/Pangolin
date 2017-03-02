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

#include <map>
#include <fstream>

#include <pangolin/log/packetstream_source.h>

namespace pangolin {

class PacketIndex: std::map<PacketStreamSourceId, std::map<size_t, std::streampos>>
{
public:
    PacketIndex()
    {
    }

    PacketIndex(const json::array& source)
    {
            for (size_t src = 0; src < source.size(); ++src)
            {
                const json::array& row = source[src].get<json::array>();
                for (size_t frame = 0; frame < row.size(); ++frame)
                    add(src, frame, row[frame].get<int64_t>());
            }
    }

    bool has(PacketStreamSourceId source, size_t frame) const {
        return count(source) ? at(source).count(frame) : false;
    }

    std::streampos position(PacketStreamSourceId source, size_t frame) const {
        return at(source).at(frame);
    }

    size_t packetCount(PacketStreamSourceId source) const {
        return count(source) ? at(source).size() : std::numeric_limits<size_t>::max();
    }

    void add(PacketStreamSourceId source, size_t frame, std::streampos position) {
        operator[](source)[frame] = position;
    }

    void add(PacketStreamSourceId source, std::streampos position)
    {
        if (operator[](source).empty()) {
            add(source, 0, position);
        } else {
            // rbegin()->first gives us the value of the last key, which will be the highest frame number so far.
            add(source, operator[](source).rbegin()->first + 1, position);
        }
    }

    json::array json() const
    {
        json::array index;
        for (const auto& src : *this)
        {
            json::array positions;
            for (const auto& frame : src.second)
            {
                positions.push_back(json::value(frame.second));
            }
            index.push_back(positions);
        }

        return index;
    }
};

}
