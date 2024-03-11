#pragma once

#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/glsl_program.h>
#include <sophus2/concepts/point.h>

namespace pangolin
{

// Scalar declarations

// A bug in GCC 11 complains about an ambiguity in the overload set for this function, even though the standard
// is clear that the constrained versions should be selected.
// Reference:
// - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111748
// - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102184
// - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105481
// By adding a constraint expression, we make it extra clear to GCC that there is no ambiguity.
template <typename T>
void glUniform(GLint location, T val) requires(!sophus2::concepts::EigenDenseType<T> && !sophus2::concepts::EnumType<T>);

template <typename T>
void glUniform(GLint location, T a, T b);
template <typename T>
void glUniform(GLint location, T a, T b, T c);
template <typename T>
void glUniform(GLint location, T a, T b, T c, T d);

// Vector / Matrix declarations
template <typename T, int R, int C = 1>
void glUniformArray(GLint location, const T* val);

// Specializations...
#define PANGO_DEF_UNIFORM_ARR(type, postfix, R, C)                             \
  template <>                                                                  \
  inline void glUniformArray<type, R, C>(GLint location, const type* val)      \
  {                                                                            \
    glUniform##postfix(location, 1, val);                                      \
  }
PANGO_DEF_UNIFORM_ARR(GLfloat, 1fv, 1, 1)
PANGO_DEF_UNIFORM_ARR(GLint, 1iv, 1, 1)
PANGO_DEF_UNIFORM_ARR(GLuint, 1uiv, 1, 1)
PANGO_DEF_UNIFORM_ARR(GLdouble, 1dv, 1, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 2fv, 2, 1)
PANGO_DEF_UNIFORM_ARR(GLint, 2iv, 2, 1)
PANGO_DEF_UNIFORM_ARR(GLuint, 2uiv, 2, 1)
PANGO_DEF_UNIFORM_ARR(GLdouble, 2dv, 2, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 3fv, 3, 1)
PANGO_DEF_UNIFORM_ARR(GLint, 3iv, 3, 1)
PANGO_DEF_UNIFORM_ARR(GLuint, 3uiv, 3, 1)
PANGO_DEF_UNIFORM_ARR(GLdouble, 3dv, 3, 1)
PANGO_DEF_UNIFORM_ARR(GLfloat, 4fv, 4, 1)
PANGO_DEF_UNIFORM_ARR(GLint, 4iv, 4, 1)
PANGO_DEF_UNIFORM_ARR(GLuint, 4uiv, 4, 1)
PANGO_DEF_UNIFORM_ARR(GLdouble, 4dv, 4, 1)
#undef PANGO_DEF_UNIFORM_ARR

#define PANGO_DEF_UNIFORM_MAT_ARR(type, postfix, R, C)                         \
  template <>                                                                  \
  inline void glUniformArray<type, R, C>(GLint location, const type* val)      \
  {                                                                            \
    glUniformMatrix##postfix(location, 1, false, val);                         \
  }
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 2fv, 2, 2)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3fv, 3, 3)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 4fv, 4, 4)
PANGO_DEF_UNIFORM_MAT_ARR(GLdouble, 2dv, 2, 2)
PANGO_DEF_UNIFORM_MAT_ARR(GLdouble, 3dv, 3, 3)
PANGO_DEF_UNIFORM_MAT_ARR(GLdouble, 4dv, 4, 4)

PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 2x3fv, 2, 3)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3x2fv, 3, 2)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 3x4fv, 3, 4)
PANGO_DEF_UNIFORM_MAT_ARR(GLfloat, 4x3fv, 4, 3)
#undef PANGO_DEF_UNIFORM_MAT_ARR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix)                                \
  template <>                                                                  \
  inline void glUniform<type>(GLint location, type val)                        \
  {                                                                            \
    glUniform1##postfix(location, val);                                        \
  }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix)                                \
  template <>                                                                  \
  inline void glUniform<type>(GLint location, type a, type b)                  \
  {                                                                            \
    glUniform2##postfix(location, a, b);                                       \
  }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix)                                \
  template <>                                                                  \
  inline void glUniform<type>(GLint location, type a, type b, type c)          \
  {                                                                            \
    glUniform3##postfix(location, a, b, c);                                    \
  }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

#define PANGO_DEF_UNIFORM_SCALAR(type, postfix)                                \
  template <>                                                                  \
  inline void glUniform<type>(GLint location, type a, type b, type c, type d)  \
  {                                                                            \
    glUniform4##postfix(location, a, b, c, d);                                 \
  }
PANGO_DEF_UNIFORM_SCALAR(GLint, i)
PANGO_DEF_UNIFORM_SCALAR(GLuint, ui)
PANGO_DEF_UNIFORM_SCALAR(GLfloat, f)
#undef PANGO_DEF_UNIFORM_SCALAR

// Specialization for bool, which we must set via the integer method.
template <>
inline void glUniform<bool>(GLint location, bool val)
{
  glUniform1i(location, static_cast<GLint>(val));
}

namespace detail
{
template <typename T>
void glUniformImpl(GLint location, const T& val);

template <class T, size_t R, size_t C>
requires(sophus2::concepts::EigenWithDim<R, C, T>) void glUniformImpl(
    GLint location, const T& mat)
{
  glUniformArray<typename T::Scalar, R, C>(location, mat.data());
}
}  // namespace detail

template <sophus2::concepts::EigenDenseType T>
void glUniform(GLint location, const T& val)
{
  detail::glUniformImpl<T, T::RowsAtCompileTime, T::ColsAtCompileTime>(
      location, val);
}

template <sophus2::concepts::EnumType T>
void glUniform(GLint location, T val)
{
  using U = std::underlying_type_t<T>;
  glUniform(location, static_cast<U>(val));
}

template <typename T>
class GlUniform
{
  public:
  GlUniform(Shared<GlSlProgram> prog, const char* name) :
      prog_(prog), name_(name), current_value_(T{}), handle_(kHandleInvalid)
  {
  }

  void reset() const { handle_ = kHandleInvalid; }

  void setValue(const T& new_value)
  {
    if (handle_ == kHandleInvalidNoRetry) return;

    const bool needs_init = handle_ == kHandleInvalid;

    if (needs_init) {
      subscribeToProgramEvents();
      GLint bound_prog_id = 0;
      PANGO_GL(glGetIntegerv(GL_CURRENT_PROGRAM, &bound_prog_id));
      if (!bound_prog_id) {
        PANGO_WARN("Unable to set uniform. No program bound.");
        return;
      }
      handle_ = glGetUniformLocation(bound_prog_id, name_.c_str());
      if (handle_ == kHandleInvalid) {
        PANGO_WARN("No uniform '{}'. May have been optimized out.", name_);
        handle_ = kHandleInvalidNoRetry;
      }
    }

    if (needs_init || sophus2::anyTrue(new_value != current_value_)) {
      glUniform<T>(handle_, new_value);
      current_value_ = new_value;
    }
  }

  const T& getValue() const { return current_value_; }

  void operator=(const T& new_value) { setValue(new_value); }

  GLint handle() const { return handle_; }

  private:
  void subscribeToProgramEvents()
  {
    prog_connection_ =
        prog_->signalEvent().connect([this](const GlSlProgram::Event& event) {
          if (event == GlSlProgram::Event::program_unlinked) {
            reset();
          }
        });
  }

  static constexpr int kHandleInvalid = -1;
  static constexpr int kHandleInvalidNoRetry = -2;

  Shared<GlSlProgram> prog_;
  std::string name_;
  std::optional<sigslot::scoped_connection> prog_connection_;
  mutable T current_value_;
  mutable int handle_;
};

}  // namespace pangolin
