#include <pangolin/datalog.h>

#include <limits>
#include <fstream>
#include <iomanip>

namespace pangolin
{

DataSequence::DataSequence(unsigned int buffer_size, unsigned size, float val )
    : buffer_size(buffer_size), ys(0), firstn(size), n(0), sum_y(0), sum_y_sq(0),
      min_y(std::numeric_limits<float>::max()),
      max_y(std::numeric_limits<float>::min())
{
    ys = new float[buffer_size];
}

DataSequence::~DataSequence()
{
    delete[] ys;
}

void DataSequence::Add(float val)
{
    operator[](n++) = val;

    min_y = std::min(min_y,val);
    max_y = std::max(max_y,val);
    sum_y += val;
    sum_y_sq += val*val;
}

void DataSequence::Clear()
{
    n = 0;
    firstn = 0;
    sum_y = 0;
    sum_y_sq = 0;
    min_y = std::numeric_limits<float>::max();
    max_y = std::numeric_limits<float>::min();
}

const float* DataSequence::FirstBlock(int i, size_t& n) const
{
    int index = (i-firstn) % buffer_size;
    n = buffer_size - index;
    return &ys[index];
}

const float* DataSequence::SecondBlock(int i, size_t& n) const
{
    n = buffer_size;
    return ys;
}

float DataSequence::operator[](int i) const
{
    return ys[(i-firstn) % buffer_size];
}

float& DataSequence::operator[](int i)
{
    return ys[(i-firstn) % buffer_size];
}

float DataSequence::Sum() const
{
    return sum_y;
}

float DataSequence::Min() const
{
    return min_y;
}

float DataSequence::Max() const
{
    return max_y;
}

DataLog::DataLog(unsigned int buffer_size)
    : buffer_size(buffer_size), x(0)
{
}

DataLog::~DataLog()
{
    for(SequenceContainer::iterator ids = sequences.begin(); ids != sequences.end(); ++ids) {
        delete *ids;
    }
    sequences.clear();
}

void DataLog::Clear()
{
    x = 0;
    sequences.clear();
}

void DataLog::Save(std::string filename)
{
    if( sequences.size() > 0 )
    {
        std::ofstream f(filename.c_str());
        for( int n=sequences[0]->IndexBegin(); n < sequences[0]->IndexEnd(); ++n )
        {
            f << std::setprecision(12) << Sequence(0)[n];
            for( unsigned s=1; s < NumSequences(); ++s ) {
                f << ", " << Sequence(s)[n];
            }
            f << std::endl;
        }
        f.close();
    }
}

void DataLog::Log(const std::vector<float> & vals)
{
    Log(vals.size(), &vals[0]);
}

void DataLog::Log(unsigned int N, const float * vals)
{
    // Create new plots if needed
    for( unsigned int i= sequences.size(); i < N; ++i )
        sequences.push_back(new DataSequence(buffer_size,x,0));

    // Add data to existing plots
    for( unsigned int i=0; i<N; ++i )
        Sequence(i).Add(vals[i]);

    // Fill missing data
    for( unsigned int i=N; i<sequences.size(); ++i )
        Sequence(i).Add(0.0f);

    ++x;
}

void DataLog::SetLabels(const std::vector<std::string> & new_labels)
{
    // Create new labels if needed
    for( unsigned int i= labels.size(); i < new_labels.size(); ++i )
        labels.push_back(std::string("N/A"));

    // Add data to existing plots
    for( unsigned int i=0; i<labels.size(); ++i )
        labels[i] = new_labels[i];
}

void DataLog::Log(float v)
{
    const float vs[] = {v};
    Log(1,vs);
}

void DataLog::Log(float v1, float v2)
{
    const float vs[] = {v1,v2};
    Log(2,vs);
}

void DataLog::Log(float v1, float v2, float v3)
{
    const float vs[] = {v1,v2,v3};
    Log(3,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4)
{
    const float vs[] = {v1,v2,v3,v4};
    Log(4,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5)
{
    const float vs[] = {v1,v2,v3,v4,v5};
    Log(5,vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6)
{
    const float vs[] = {v1,v2,v3,v4,v5,v6};
    Log(6,vs);
}

}
