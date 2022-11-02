#pragma once

#include <pangolin/gl/glplatform.h>
#include <pangolin/maths/eigen_concepts.h>


namespace pangolin
{

template<typename T, int R, int C=1>
void glUniformArray(GLint location, const T* val);

#define PANGO_DEF_UNIFORM_ARR(type, postfix, R, C) \
    template<> \
    void glUniformArray<type,R,C>(GLint location, const type* val) { \
        glUniform##postfix(location, 1, val); \
    }
PANGO_DEF_UNIFORM_ARR(GLfloat, 1fv,  1, 1)
PANGO_DEF_UNIFORM_ARR(GLint,   1iv,  1, 1)
PANGO_DEF_UNIFORM_ARR(GLuint,  1uiv, 1, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 2fv,  2, 1)
PANGO_DEF_UNIFORM_ARR(GLint,   2iv,  2, 1)
PANGO_DEF_UNIFORM_ARR(GLuint,  2uiv, 2, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 3fv,  3, 1)
PANGO_DEF_UNIFORM_ARR(GLint,   3iv,  3, 1)
PANGO_DEF_UNIFORM_ARR(GLuint,  3uiv, 3, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 4fv,  4, 1)
PANGO_DEF_UNIFORM_ARR(GLint,   4iv,  4, 1)
PANGO_DEF_UNIFORM_ARR(GLuint,  4uiv, 4, 1)
#undef PANGO_DEF_UNIFORM_ARR

#define PANGO_DEF_UNIFORM_MAT_ARR(type, postfix, R, C) \
    template<> \
    void glUniformArray<type,R,C>(GLint location, const type* val) { \
        glUniformMatrix##postfix(location, 1, false, val); \
    }
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 2fv,  2, 2)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3fv,  3, 3)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 4fv,  4, 4)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 2x3fv, 2, 3)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3x2fv, 3, 2)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3x4fv, 3, 4)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 4x3fv, 4, 3)
#undef PANGO_DEF_UNIFORM_MAT_ARR

template<typename T>
void glUniform(GLint location, const T& val);

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix) \
    template<> \
    void glUniform<type>(GLint location, const type& val) { \
        glUniform1##postfix( location, val); \
    }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

namespace detail {
template<typename T>
void glUniformImpl(GLint location, const T& val);

template<class T, size_t R, size_t C>
requires(EigenWithDim<R,C,T>)
void glUniformImpl(GLint location, const T& mat) {
    glUniformArray<typename T::Scalar,R,C>( location, mat.data());
}
}

template<EigenDenseType T>
void glUniform(GLint location, const T& val)
{
    detail::glUniformImpl<T,T::RowsAtCompileTime,T::ColsAtCompileTime>(location, val);
}

template<typename T>
class GlUniform {
public:
    GlUniform(const char* name, const T default_value = {})
        : name_(name), current_value_(default_value), handle_(kHandleInvalid)
    {
    }

    void operator=(const T& new_value) {
        if(handle_ == kHandleInvalid) {
            GLint bound_prog_id;
            glGetIntegerv(GL_CURRENT_PROGRAM, &bound_prog_id);
            handle_ = glGetUniformLocation(bound_prog_id, name_);
            if(GLenum err = glGetError() != GL_NO_ERROR) {

            }
        }

        if(new_value != current_value_) {
            glUniform<T>(handle_, new_value);
            current_value_ = new_value;
        }
    }

    GLint handle() const {
        return handle_;
    }

private:
    static constexpr int kHandleInvalid = -1;

    const char* name_;
    T current_value_;
    int handle_;
};

}
