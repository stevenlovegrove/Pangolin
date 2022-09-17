#pragma once

#include <pangolin/gl/glsl.h>

namespace pangolin {

class GlSlUtilities
{
public:
    inline static GlSlProgram& OffsetAndScale(float offset, float scale) {
        GlSlProgram& prog = Instance().prog_offsetscale;
        prog.Bind();
        prog.SetUniform("offset", offset);
        prog.SetUniform("scale",  scale);
        return prog;
    }

    inline static GlSlProgram& OffsetAndScaleGamma(float offset, float scale) {
        GlSlProgram& prog = Instance().prog_offsetscalegamma;
        prog.Bind();
        prog.SetUniform("offset", offset);
        prog.SetUniform("scale",  scale);
        return prog;
    }

    inline static GlSlProgram& Scale(float scale, float bias = 0.0f) {
        GlSlProgram& prog = Instance().prog_scale;
        prog.Bind();
        prog.SetUniform("scale", scale);
        prog.SetUniform("bias",  bias);
        return prog;
    }

    inline static void UseNone()
    {
        glUseProgram(0);
    }

protected:
    static GlSlUtilities& Instance() {
        // TODO: BUG: The GlSLUtilities instance needs to be tied
        // to the OpenGL context, not the thread, or globally.
#ifndef PANGO_NO_THREADLOCAL
        thread_local
#else
        static
#endif
            GlSlUtilities instance;
        return instance;
    }

    // protected constructor
    GlSlUtilities() {
        const char* source_scale =
            "uniform float scale;"
            "uniform float bias;"
            "uniform sampler2D tex;"
            "void main() {"
            "  vec2 uv = gl_TexCoord[0].st;"
            "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
            "    gl_FragColor = texture2D(tex,uv);"
            "    gl_FragColor.xyz *= scale;"
            "    gl_FragColor.xyz += vec3(bias,bias,bias);"
            "  }else{"
            "    float v = 0.1;"
            "    gl_FragColor.xyz = vec3(v,v,v);"
            "  }"
            "}";
        prog_scale.AddShader(GlSlFragmentShader, source_scale);
        prog_scale.Link();

        // shader performs automatically gamma correction, assuming that image data is linear
        // maps to (approximate) sRGB
        const char* source_offsetscalegamma =
            "uniform float offset;"
            "uniform float scale;"
            "uniform sampler2D tex;"
            "void main() {"
            "  vec2 uv = gl_TexCoord[0].st;"
            "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
            "    gl_FragColor = texture2D(tex,gl_TexCoord[0].st);"
            "    gl_FragColor.xyz += vec3(offset,offset,offset);"
            "    gl_FragColor.xyz *= scale;"
            "    gl_FragColor.xyz = pow(gl_FragColor.xyz,vec3(0.45,0.45,0.45));"
            "  }else{"
            "    float v = 0.1;"
            "    gl_FragColor.xyz = vec3(v,v,v);"
            "  }"
            "}";
        prog_offsetscalegamma.AddShader(GlSlFragmentShader, source_offsetscalegamma);
        prog_offsetscalegamma.Link();

        const char* source_offsetscale =
            "uniform float offset;"
            "uniform float scale;"
            "uniform sampler2D tex;"
            "void main() {"
            "  vec2 uv = gl_TexCoord[0].st;"
            "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
            "    gl_FragColor = texture2D(tex,gl_TexCoord[0].st);"
            "    gl_FragColor.xyz += vec3(offset,offset,offset);"
            "    gl_FragColor.xyz *= scale;"
            "  }else{"
            "    float v = 0.1;"
            "    gl_FragColor.xyz = vec3(v,v,v);"
            "  }"
            "}";
        prog_offsetscale.AddShader(GlSlFragmentShader, source_offsetscale);
        prog_offsetscale.Link();
    }

    GlSlProgram prog_scale;
    GlSlProgram prog_offsetscale;
    GlSlProgram prog_offsetscalegamma;
};

}
