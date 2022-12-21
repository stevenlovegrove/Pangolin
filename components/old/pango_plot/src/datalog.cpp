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
#include <limits>

namespace pangolin
{

void DataLogBlock::AddSamples(
    size_t num_samples, size_t dimensions, float const* data_dim_major)
{
  if (nextBlock) {
    // If next block exists, add to it instead
    nextBlock->AddSamples(num_samples, dimensions, data_dim_major);
  } else {
    if (dimensions > dim) {
      // If dimensions is too high for this block, start a new bigger one
      nextBlock = std::unique_ptr<DataLogBlock>(
          new DataLogBlock(dimensions, max_samples, start_id + samples));
      nextBlock->AddSamples(num_samples, dimensions, data_dim_major);
    } else {
      // Try to copy samples to this block
      const size_t samples_to_copy = std::min(num_samples, SampleSpaceLeft());

      if (dimensions == dim) {
        // Copy entire block all together
        std::copy(
            data_dim_major, data_dim_major + samples_to_copy * dim,
            sample_buffer.get() + samples * dim);
        samples += samples_to_copy;
        data_dim_major += samples_to_copy * dim;
      } else {
        // Copy sample at a time, filling with NaN's where needed.
        float* dst = sample_buffer.get();
        for (size_t i = 0; i < samples_to_copy; ++i) {
          std::copy(data_dim_major, data_dim_major + dimensions, dst);
          for (size_t ii = dimensions; ii < dim; ++ii) {
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
      if (samples_to_copy < num_samples) {
        nextBlock = std::unique_ptr<DataLogBlock>(
            new DataLogBlock(dim, max_samples, start_id + Samples()));
        nextBlock->AddSamples(
            num_samples - samples_to_copy, dimensions, data_dim_major);
      }
    }
  }
}

DataLog::DataLog(unsigned int buffer_size) :
    block_samples_alloc(buffer_size),
    block0(nullptr),
    blockn(nullptr),
    record_stats(true)
{
}

DataLog::~DataLog() { Clear(); }

void DataLog::SetLabels(std::vector<std::string> const& new_labels)
{
  std::lock_guard<std::mutex> l(access_mutex);

  // Create new labels if needed
  for (size_t i = labels.size(); i < new_labels.size(); ++i)
    labels.push_back(std::string());

  // Add data to existing plots
  for (unsigned int i = 0; i < labels.size(); ++i) labels[i] = new_labels[i];
}

std::vector<std::string> const& DataLog::Labels() const { return labels; }

void DataLog::Log(size_t dimension, float const* vals, unsigned int samples)
{
  if (!block0) {
    // Create first block
    block0 = std::unique_ptr<DataLogBlock>(
        new DataLogBlock(dimension, block_samples_alloc, 0));
    blockn = block0.get();
  }

  if (record_stats) {
    while (stats.size() < dimension) {
      stats.push_back(DimensionStats());
    }
    for (unsigned int d = 0; d < dimension; ++d) {
      DimensionStats& ds = stats[d];
      for (unsigned int s = 0; s < samples; ++s) {
        ds.Add(vals[s * dimension + d]);
      }
    }
  }

  blockn->AddSamples(samples, dimension, vals);

  // Update pointer to most recent block.
  while (blockn->NextBlock()) {
    blockn = blockn->NextBlock();
  }
}

void DataLog::Log(float v)
{
  float const vs[] = {v};
  Log(1, vs);
}

void DataLog::Log(float v1, float v2)
{
  float const vs[] = {v1, v2};
  Log(2, vs);
}

void DataLog::Log(float v1, float v2, float v3)
{
  float const vs[] = {v1, v2, v3};
  Log(3, vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4)
{
  float const vs[] = {v1, v2, v3, v4};
  Log(4, vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5)
{
  float const vs[] = {v1, v2, v3, v4, v5};
  Log(5, vs);
}
void DataLog::Log(float v1, float v2, float v3, float v4, float v5, float v6)
{
  float const vs[] = {v1, v2, v3, v4, v5, v6};
  Log(6, vs);
}

void DataLog::Log(
    float v1, float v2, float v3, float v4, float v5, float v6, float v7)
{
  float const vs[] = {v1, v2, v3, v4, v5, v6, v7};
  Log(7, vs);
}

void DataLog::Log(
    float v1, float v2, float v3, float v4, float v5, float v6, float v7,
    float v8)
{
  float const vs[] = {v1, v2, v3, v4, v5, v6, v7, v8};
  Log(8, vs);
}

void DataLog::Log(
    float v1, float v2, float v3, float v4, float v5, float v6, float v7,
    float v8, float v9)
{
  float const vs[] = {v1, v2, v3, v4, v5, v6, v7, v8, v9};
  Log(9, vs);
}

void DataLog::Log(
    float v1, float v2, float v3, float v4, float v5, float v6, float v7,
    float v8, float v9, float v10)
{
  float const vs[] = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
  Log(10, vs);
}

void DataLog::Log(std::vector<float> const& vals)
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

  DataLogBlock const* block = FirstBlock();

  size_t i = 0;

  while (block) {
    const size_t blockEnd = i + block->Samples();

    for (; i < blockEnd; ++i) {
      csvStream << block->Sample(i)[0];

      for (size_t d = 1; d < block->Dimensions(); ++d) {
        csvStream << "," << block->Sample(i)[d];
      }

      csvStream << std::endl;
    }

    block = block->NextBlock();
  }
}

DataLogBlock const* DataLog::FirstBlock() const { return block0.get(); }

DataLogBlock const* DataLog::LastBlock() const { return blockn; }

DimensionStats const& DataLog::Stats(size_t dim) const { return stats[dim]; }

size_t DataLog::Samples() const
{
  if (blockn) {
    return blockn->StartId() + blockn->Samples();
  }
  return 0;
}

float const* DataLog::Sample(int n) const
{
  if (block0) {
    return block0->Sample(n);
  } else {
    return 0;
  }
}

}  // namespace pangolin
