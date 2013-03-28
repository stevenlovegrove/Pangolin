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

#ifndef PANGOLIN_VARS_INTERNAL_H
#define PANGOLIN_VARS_INTERNAL_H

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
//#include <boost/lexical_cast.hpp>
#include <iostream>


namespace pangolin
{

struct BadInputException : std::exception {
  char const* what() const throw() { return "Failed to serialise type"; }
};

// Generic conversion through serialisation from / to string
template<typename T, typename S> struct Convert {
  static T Do(const S& src)
  {
    std::ostringstream oss;
    oss << src;
    std::istringstream iss(oss.str());
    T target;
    iss >> target;

    if(iss.fail())
      throw BadInputException();

    return target;
    //    return boost::lexical_cast<T>(src);
  }
};

// Apply bool alpha IO manipulator for bool types
template<> struct Convert<bool,std::string> {
  static bool Do(const std::string& src)
  {
    bool target;
    std::istringstream iss(src);
    iss >> target;

    if(iss.fail())
    {
      std::istringstream iss2(src);
      iss2 >> std::boolalpha >> target;
      if( iss2.fail())
        throw BadInputException();
    }

    return target;
  }
};

// From strings
template<typename T> struct Convert<T,std::string> {
  static T Do(const std::string& src)
  {
    T target;
    std::istringstream iss(src);
    iss >> target;

    if(iss.fail())
      throw BadInputException();

    return target;
  }
};

// To strings
template<typename S> struct Convert<std::string, S> {
  static std::string Do(const S& src)
  {
    std::ostringstream oss;
    oss << src;
    return oss.str();
  }
};

// Between strings is just a copy
template<> struct Convert<std::string, std::string> {
  static std::string Do(const std::string& src)
  {
    return src;
  }
};

struct UnknownTypeException : std::exception {
  char const* what() const throw() { return "Unknown type in generic container"; }
};

struct _Var
{
  _Var()
    : val(NULL), val_default(NULL), generic(false)
  {}
  ~_Var(){
    clean();
  }

  void create(void* new_val, void* new_val_default, const char* new_type_name){
    clean();
    val = new_val;
    val_default = new_val_default;
    type_name = new_type_name;
    generic = false;
  }

  void clean(){
    if (val!=NULL){
      if( type_name == typeid(double).name() ) {
        delete (double *) val;
      } else if( type_name == typeid(int).name() ) {
        delete (int *) val;
      } else if( type_name == typeid(std::string).name() ) {
        delete (std::string *) val;
      } else if( type_name == typeid(bool).name() ) {
        delete (bool *) val;
      } else {
        throw UnknownTypeException();
      }
    }

    if (val_default!=NULL){
      if( type_name == typeid(double).name() ) {
        delete (double *) val_default;
      } else if( type_name == typeid(int).name() ) {
        delete (int *) val_default;
      } else if( type_name == typeid(std::string).name() ) {
        delete (std::string *) val_default;
      } else if( type_name == typeid(bool).name() ) {
        delete (bool *) val_default;
      } else {
        throw UnknownTypeException();
      }
    }
  }




  void* val;
  void* val_default;

  const char* type_name;

  bool generic;
  std::string meta_full_name;
  std::string meta_friendly;
  double meta_range[2];
  double meta_increment;
  int meta_flags;
  bool meta_gui_changed;
  bool logscale;
};

// Forward declaration
template<typename T, typename S, class Enable1 = void, class Enable2 = void, class Enable3 = void>
struct _Accessor;

template<typename T>
struct Accessor
{
  virtual ~Accessor() {}
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
      }

      const T& Get() const
      {
        //    try{
        cache = Convert<T,S>::Do(var);
        //    }catch(BadInputException e) {
        //        std::cerr << e.what() << std::endl;
        //    }

        return cache;
      }

      void Set(const T& val)
      {
        //    try{
        var = Convert<S,T>::Do(val);
        //    }catch(BadInputException e) {
        //      std::cerr << "Unable to convert: " << e.what() << std::endl;
        //    }
      }
      S& var;
      mutable T cache;
    };

}

#endif // PANGOLIN_VARS_INTERNAL_H

