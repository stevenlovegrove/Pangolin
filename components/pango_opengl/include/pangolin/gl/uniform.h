#pragma once

#include <pangolin/gl/glplatform.h>
#include <pangolin/maths/eigen_concepts.h>
#include <pangolin/maths/eigen_scalar_methods.h>

namespace pangolin
{

// Scalar declarations
template<typename T> void glUniform(GLint location, T val);
template<typename T> void glUniform(GLint location, T a, T b);
template<typename T> void glUniform(GLint location, T a, T b, T c);
template<typename T> void glUniform(GLint location, T a, T b, T c, T d);

// Vector / Matrix declarations
template<typename T, int R, int C=1>
void glUniformArray(GLint location, const T* val);


// Specializations...
#define PANGO_DEF_UNIFORM_ARR(type, postfix, R, C) \
    template<> \
    inline void glUniformArray<type,R,C>(GLint location, const type* val) { \
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
    inline void glUniformArray<type,R,C>(GLint location, const type* val) { \
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

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix) \
    template<> \
    inline void glUniform<type>(GLint location, type val) { \
        glUniform1##postfix( location, val); \
    }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix) \
    template<> \
    inline void glUniform<type>(GLint location, type a, type b) { \
        glUniform2##postfix( location, a, b); \
    }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix) \
    template<> \
    inline void glUniform<type>(GLint location, type a, type b, type c) { \
        glUniform3##postfix( location, a, b, c); \
    }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix) \
    template<> \
    inline void glUniform<type>(GLint location, type a, type b, type c, type d) { \
        glUniform4##postfix( location, a, b, c, d); \
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

template<EnumType T>
void glUniform(GLint location, T val)
{
    using U = std::underlying_type_t<T>;
    glUniform(location, static_cast<U>(val));
}

template<typename T>
class GlUniform {
public:
    GlUniform(const char* name, const T default_value = {})
        : name_(name),
        default_value_(default_value),
        current_value_(default_value),
        handle_(kHandleInvalid)
    {
    }

    void setDefault() const {
        setValue(default_value_);
    }

    void setValue(const T& new_value) const {
        const bool needs_init = handle_ == kHandleInvalid;

        if(needs_init) {
            GLint bound_prog_id;
            PANGO_GL(glGetIntegerv(GL_CURRENT_PROGRAM, &bound_prog_id));
            PANGO_CHECK(bound_prog_id != 0, "This method can only be called with the corresponding program already bound.");
            handle_ = glGetUniformLocation(bound_prog_id, name_);
            PANGO_CHECK(handle_ != -1, "Name '{}' doesn't correspond to a used uniform (may have been optimized out).", name_);
        }

        if(needs_init || anyTrue(new_value != current_value_) ) {
            glUniform<T>(handle_, new_value);
            current_value_ = new_value;
        }
    }

    const T& getValue() const {
        return current_value_;
    }

    void operator=(const T& new_value) const {
        setValue(new_value);
    }

    GLint handle() const {
        return handle_;
    }

private:
    static constexpr int kHandleInvalid = -1;

    const char* name_;
    T default_value_;
    mutable T current_value_;
    mutable int handle_;
};

}
