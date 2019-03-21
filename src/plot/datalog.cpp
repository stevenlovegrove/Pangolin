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

#include <pangolin/plot/datalog.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

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
            nextBlock = std::unique_ptr<DataLogBlock>(new DataLogBlock(dimensions, max_samples, start_id + samples));
            nextBlock->AddSamples(num_samples,dimensions,data_dim_major);
        }else{
            // Try to copy samples to this block
            const size_t samples_to_copy = std::min(num_samples, SampleSpaceLeft());

            if(dimensions == dim) {
                // Copy entire block all together
                std::copy(data_dim_major, data_dim_major + samples_to_copy*dim, sample_buffer.get()+samples*dim);
                samples += samples_to_copy;
                data_dim_major += samples_to_copy*dim;
            }else{
                // Copy sample at a time, filling with NaN's where needed.
                float* dst = sample_buffer.get();
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

//            // Update Stats
//            for(size_t s=0; s < samples_to_copy; ++s) {
//                for(size_t d = 0; d < dimensions; ++d) {
//                    stats[d].Add(data_dim_major[s*dim + d]);
//                }
//            }

            // Copy remaining data to next block (this one is full)
            if(samples_to_copy < num_samples) {
                nextBlock = std::unique_ptr<DataLogBlock>(new DataLogBlock(dim, max_samples, start_id + Samples()));
                nextBlock->AddSamples(num_samples-samples_to_copy, dimensions, data_dim_major);
            }
        }
    }
}

DataLog::DataLog(unsigned int buffer_size)
    : block_samples_alloc(buffer_size), block0(nullptr), blockn(nullptr), record_stats(true)
{
}

DataLog::~DataLog()
{
    Clear();
}

void DataLog::SetLabels(const std::vector<std::string> & new_labels)
{
    std::lock_guard<std::mutex> l(access_mutex);

    // Create new labels if needed
    for( size_t i= labels.size(); i < new_labels.size(); ++i )
        labels.push_back( std::string() );

    // Add data to existing plots
    for( unsigned int i=0; i<labels.size(); ++i )
        labels[i] = new_labels[i];
}

const std::vector<std::string>& DataLog::Labels() const
{
    return labels;
}

void DataLog::Log(size_t dimension, const float* vals, unsigned int samples )
{
    if(!block0) {
        // Create first block
        block0 = std::unique_ptr<DataLogBlock>(new DataLogBlock(dimension, block_samples_alloc, 0));
        blockn = block0.get();
    }

    if(record_stats) {
        while(stats.size() < dimension) {
            stats.push_back( DimensionStats() );
        }
        for(unsigned int d=0; d<dimension; ++d) {
            DimensionStats& ds = stats[d];
            for(unsigned int s=0; s<samples; ++s) {
                ds.Add(vals[s*dimension+d]);
            }
        }
    }

    blockn->AddSamples(samples,dimension,vals);

    // Update pointer to most recent block.
    while(blockn->NextBlock()) {
        blockn = blockn->NextBlock();
    }
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

void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6, float v7)
{
    const float vs[] = {v1,v2,v3,v4,v5,v6,v7};
    Log(7,vs);
}

void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8)
{
    const float vs[] = {v1,v2,v3,v4,v5,v6,v7,v8};
    Log(8,vs);
}

void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9)
{
    const float vs[] = {v1,v2,v3,v4,v5,v6,v7,v8,v9};
    Log(9,vs);
}

void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10)
{
    const float vs[] = {v1,v2,v3,v4,v5,v6,v7,v8,v9,v10};
    Log(10,vs);
}

void DataLog::Log(const std::vector<float> & vals)
{
    Log(vals.size(), &vals[0]);
}

void DataLog::Clear()
{
    std::lock_guard<std::mutex> l(access_mutex);

    blockn = nullptr;
    block0 = nullptr;

    stats.clear();
}

void DataLog::Save(std::string filename)
{
    std::ofstream csvStream(filename);

      if (!Labels().empty()) {
        csvStream << Labels()[0];

        for (size_t i = 1; i < Labels().size(); ++i) {
          csvStream << "," << Labels()[i];
        }

        csvStream << std::endl;

    }

    const DataLogBlock * block = FirstBlock();

    while (block) {

      for (size_t i = 0; i < block->Samples(); ++i) {

        csvStream << block->Sample(i)[0];

        for (size_t d = 1; d < block->Dimensions(); ++d) {

          csvStream << "," << block->Sample(i)[d];

        }

        csvStream << std::endl;

      }

      block = block->NextBlock();

    }
}

const DataLogBlock* DataLog::FirstBlock() const
{
    return block0.get();
}

const DataLogBlock* DataLog::LastBlock() const
{
    return blockn;
}

const DimensionStats& DataLog::Stats(size_t dim) const
{
    return stats[dim];
}

size_t DataLog::Samples() const
{
    if(blockn) {
        return blockn->StartId() + blockn->Samples();
    }
    return 0;
}

const float* DataLog::Sample(int n) const
{
    if(block0) {
        return block0->Sample(n);
    }else{
        return 0;
    }
}

}
