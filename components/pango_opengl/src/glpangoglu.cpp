/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <pangolin/gl/glplatform.h>

#include <unordered_map>

namespace pangolin
{

const GLubyte* glErrorString(GLenum err)
{
  switch (err) {
    case GL_NO_ERROR:
      return (GLubyte*)"GL_NO_ERROR: No error has been recorded.";
    case GL_INVALID_ENUM:
      return (GLubyte*)"GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
    case GL_INVALID_VALUE:
      return (GLubyte*)"GL_INVALID_VALUE: A numeric argument is out of range.";
    case GL_INVALID_OPERATION:
      return (GLubyte*)"GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
    case GL_STACK_OVERFLOW:
      return (GLubyte*)"GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.";
    case GL_STACK_UNDERFLOW:
      return (GLubyte*)"GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.";
    case GL_OUT_OF_MEMORY:
      return (GLubyte*)"GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
    case 0x8031: /* not core */
      return (GLubyte*)"GL_TABLE_TOO_LARGE_EXT";
    case 0x8065: /* not core */
      return (GLubyte*)"GL_TEXTURE_TOO_LARGE_EXT";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return (GLubyte*)"GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
    default:
      return (GLubyte*)"[Unknown error code]";
  }
}

}  // namespace pangolin
