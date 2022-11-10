// The MIT License
// Copyright © 2017 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


// Similar to https://www.shadertoy.com/view/XlXBWs, but with a triangular filter kernel,
// which produces less flickering animations that a box filter. Luckily, it's still easily
// integrable analytically!
//
// Info: https://iquilezles.org/articles/filterableprocedurals
//
// More filtered patterns:  https://www.shadertoy.com/playlist/l3KXR1

// --- analytically triangle-filtered checkerboard ---

vec2 pri( in vec2 x )
{
    // see https://www.shadertoy.com/view/MtffWs
    vec2 h = fract(x/2.0)-0.5;
    return x*0.5 + h*(1.0-2.0*abs(h));
}

float checkersTextureGradTri( in vec2 p, in vec2 ddx, in vec2 ddy )
{
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;       // filter kernel
    vec2 i = (pri(p+w)-2.0*pri(p)+pri(p-w))/(w*w); // analytical integral (box filter)
    return 0.5 - 0.5*i.x*i.y;                      // xor pattern
}
