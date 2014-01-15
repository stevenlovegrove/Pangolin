/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#ifndef PANGOLIN_DATALOG_H
#define PANGOLIN_DATALOG_H

#include <vector>
#include <string>

namespace pangolin
{

class DataSequence
{
public:
    DataSequence(unsigned int buffer_size = 1024, unsigned size = 0, float val = 0.0f );
    ~DataSequence();

    void Add(float val);
    void Clear();

    float operator[](int i) const;
    float& operator[](int i);

    // Return first and second contiguous blocks of memory.
    const float* FirstBlock(int i, size_t &n) const;
    const float* SecondBlock(int i, size_t &n) const;

    int IndexBegin() const;
    int IndexEnd() const;

    bool HasData(int i) const;

    float Sum() const;
    float Min() const;
    float Max() const;

protected:
    DataSequence(const DataSequence& /*o*/) {}

    int buffer_size;
    float* ys;

    int firstn;
    int n;
    float sum_y;
    float sum_y_sq;
    float min_y;
    float max_y;

};

inline int DataSequence::IndexEnd() const
{
    return firstn + n;
}

inline int DataSequence::IndexBegin() const
{
    return std::max(firstn, IndexEnd()-buffer_size);
}

inline bool DataSequence::HasData(int i) const {
    const int last  = IndexEnd();
    const int first = std::max(firstn, last-buffer_size);
    return first <= i && i < last;
}

class DataLog
{
public:
    typedef std::vector<DataSequence*> SequenceContainer;

    DataLog(unsigned int buffer_size = 10000 );
    ~DataLog();

    void Log(float v);
    void Log(float v1, float v2);
    void Log(float v1, float v2, float v3);
    void Log(float v1, float v2, float v3, float v4);
    void Log(float v1, float v2, float v3, float v4, float v5);
    void Log(float v1, float v2, float v3, float v4, float v5, float v6);
    void Log(unsigned int N, const float * vals);
    void Log(const std::vector<float> & vals);
    void SetLabels(const std::vector<std::string> & labels);
    void Clear();
    void Save(std::string filename);

    inline size_t NumSequences() {
        return sequences.size();
    }

    inline DataSequence& Sequence(size_t id) {
        return *sequences[id];
    }

    inline const DataSequence& Sequence(size_t id) const {
        return *sequences[id];
    }

//protected:
    unsigned int buffer_size;
    int x;
    SequenceContainer sequences;
    std::vector<std::string> labels;
};

}

#endif // PANGOLIN_DATALOG_H
