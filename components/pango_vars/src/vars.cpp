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

#include <pangolin/var/varextra.h>
#include <pangolin/var/varstate.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/picojson.h>
#include <pangolin/utils/transform.h>

#include <iostream>
#include <fstream>

using namespace std;

namespace pangolin
{

void ParseVarsFile(const string& filename)
{
    VarState::I().LoadFromFile(filename, VarState::FileKind::config);
}

void LoadJsonFile(const std::string& filename, const string &prefix)
{
    VarState::I().LoadFromFile(filename, VarState::FileKind::json);
}

void SaveJsonFile(const std::string& filename, const string &prefix)
{
    VarState::I().SaveToFile(filename, VarState::FileKind::json);
}

}
