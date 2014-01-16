#include <pangolin/datalog.h>

#include <limits>
#include <fstream>
#include <iomanip>
#include <stdexcept>

#include <iostream>

namespace pangolin
{

void DataLogBlock::AddSamples(size_t num_samples, size_t dimensions, const float* data_dim_major )
{
    if(nextBlock) {
        // If next block exists, add to it instead
        nextBlock->AddSamples(num_samples, dimensions, data_dim_major);
    }else{
        if(dimensions > dim) {
            // If dimensions is too high for this block, start a new bigger one
            nextBlock = new DataLogBlock(dimensions, max_samples, start_id + samples);
        }else{
            // Try to copy samples to this block
            const size_t samples_to_copy = std::min(num_samples, SampleSpaceLeft());

            if(dimensions == dim) {
                // Copy entire block all together
                std::copy(data_dim_major, data_dim_major + samples_to_copy*dim, sample_buffer+samples*dim);
                samples += samples_to_copy;
                data_dim_major += samples_to_copy*dim;
            }else{
                // Copy sample at a time, filling with NaN's where needed.
                float* dst = sample_buffer;
                for(size_t i=0; i< samples_to_copy; ++i) {
                    std::copy(data_dim_major, data_dim_major + dimensions, dst);
                    for(size_t ii = dimensions; ii < dim; ++ii) {
                        dst[ii] = std::numeric_limits<float>::quiet_NaN();
                    }
                    dst += dimensions;
                    data_dim_major += dimensions;
                }
                samples += samples_to_copy;
            }

            // Copy remaining data to next block (this one is full)
            if(samples_to_copy < num_samples) {
                nextBlock = new DataLogBlock(dim, max_samples, start_id + Samples());
                nextBlock->AddSamples(num_samples-samples_to_copy, dimensions, data_dim_major);
            }
        }
    }
}

DataLog::DataLog(unsigned int buffer_size)
    : block_samples_alloc(buffer_size), blocks(0)
{
}

DataLog::~DataLog()
{
    Clear();
}

void DataLog::SetLabels(const std::vector<std::string> & new_labels)
{
    // Create new labels if needed
    for( unsigned int i= labels.size(); i < new_labels.size(); ++i )
        labels.push_back( std::string() );

    // Add data to existing plots
    for( unsigned int i=0; i<labels.size(); ++i )
        labels[i] = new_labels[i];
}

void DataLog::Log(unsigned int dimension, const float* vals, unsigned int samples )
{
    if(!blocks) {
        // Create first block
        blocks = new DataLogBlock(dimension, block_samples_alloc, 0);
    }

    blocks->AddSamples(samples,dimension,vals);

    // TODO: Compute stats for new samples
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

void DataLog::Log(const std::vector<float> & vals)
{
    Log(vals.size(), &vals[0]);
}

void DataLog::Clear()
{
    if(blocks) {
        blocks->ClearLinked();
        delete blocks;
        blocks = 0;
    }
}

void DataLog::Save(std::string filename)
{
    // TODO: Implement
    throw std::runtime_error("Method not implemented");
}

const DataLogBlock* DataLog::Blocks() const
{
    return blocks;
}

}
