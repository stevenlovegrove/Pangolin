#ifndef PANGOLIN_VARS_INTERNAL_H
#define PANGOLIN_VARS_INTERNAL_H

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
//#include <boost/lexical_cast.hpp>
#include <iostream>


namespace pangolin
{

template<typename T, typename S> struct Convert {
  static T Do(const S& src)
  {
  std::ostringstream oss;
    oss << src;
  std::istringstream iss(oss.str());
    T target;
    iss >> target;
    return target;
//    return boost::lexical_cast<T>(src);
  }
};

template<typename T> struct Convert<T,std::string> {
  static T Do(const std::string& src)
  {
    T target;
    std::istringstream iss(src);
    iss >> target;
    return target;
  }
};

template<typename S> struct Convert<std::string, S> {
  static std::string Do(const S& src)
  {
    std::ostringstream oss;
    oss << src;
    return oss.str();
  }
};

template<> struct Convert<std::string, std::string> {
  static std::string Do(const std::string& src)
  {
    return src;
  }
};

struct _Var
{
  _Var() {}
  _Var(void* val, void* val_default, const char* type_name)
    : val(val), val_default(val_default), type_name(type_name), generic(false){}

  void* val;
  void* val_default;

  const char* type_name;

  bool generic;
  std::string meta_friendly;
  double meta_range[2];
  double meta_increment;
  int meta_flags;
};

// Forward declaration
template<typename T, typename S, class Enable1 = void, class Enable2 = void, class Enable3 = void>
struct _Accessor;

struct UnknownTypeException : std::exception {
  char const* what() const throw() { return "Unknown type in generic container"; }
};

template<typename T>
struct Accessor
{
  virtual const T& Get() const = 0;
  virtual void Set(const T& val) = 0;
  static Accessor<T>* Create(const char* typeidname, void* var)
  {
    if( typeidname == typeid(double).name() ) {
      return new _Accessor<T,double>( *(double*)var);
    } else if( typeidname == typeid(int).name() ) {
      return new _Accessor<T,int>( *(int*)var );
    } else if( typeidname == typeid(std::string).name() ) {
      return new _Accessor<T,std::string>( *(std::string*)var );
    } else if( typeidname == typeid(bool).name() ) {
      return new _Accessor<T,bool>( *(bool*)var );
    } else {
      throw UnknownTypeException();
    }
  }
};

template<typename T, typename S>
struct _Accessor<T,S, typename boost::enable_if_c<
    (boost::is_scalar<T>::value || boost::is_same<T,bool>::value) &&
    (boost::is_scalar<S>::value || boost::is_same<S,bool>::value) &&
      !boost::is_same<T,S>::value
  >::type> : Accessor<T>
{
  _Accessor(S& var) : var(var) {
//    std::cout << "scalar" << std::endl;
  }

  const T& Get() const
  {
    cache = (T)var;
    return cache;
  }

  void Set(const T& val)
  {
    var = (S)val;
  }

  S& var;
  mutable T cache;
};

template<typename T>
struct _Accessor<T,T> : Accessor<T>
{
  _Accessor(T& var) : var(var) {
//    std::cout << "same" << std::endl;
  }

  const T& Get() const
  {
    return var;
  }

  void Set(const T& val)
  {
    var = val;
  }
  T& var;
};

template<typename T, typename S>
struct _Accessor<T,S ,typename boost::enable_if_c<
    !((boost::is_scalar<T>::value || boost::is_same<T,bool>::value) &&
    (boost::is_scalar<S>::value || boost::is_same<S,bool>::value)) &&
    !boost::is_same<T,S>::value
>::type> : Accessor<T>
{
  _Accessor(S& var) : var(var) {
//    std::cout << "lexical" << std::endl;
  }

  const T& Get() const
  {
//    cache = boost::lexical_cast<T>(var);
    cache = Convert<T,S>::Do(var);
    return cache;
  }

  void Set(const T& val)
  {
//    var = boost::lexical_cast<S>(val);
    var = Convert<S,T>::Do(val);
  }
  S& var;
  mutable T cache;
};

}

#endif // PANGOLIN_VARS_INTERNAL_H

