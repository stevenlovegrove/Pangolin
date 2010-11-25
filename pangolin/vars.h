#ifndef PANGOLIN_VARS_H
#define PANGOLIN_VARS_H

#include <sstream>
#include <map>
#include <iostream>

namespace pangolin
{

  ////////////////////////////////////////////////
  // Interface
  ////////////////////////////////////////////////

  //! Typeless baseclass
  struct _var
  {
    virtual std::string ToString() = 0;
    virtual bool Set(std::string val) = 0;
  };

  //! Typed variable
  template<typename T>
  struct _tvar : _var
  {
    virtual const T& operator*() const = 0;
    virtual void operator=(T x) = 0;
  };

  //! Typed container for variable
  template<typename T>
  struct _dvar : _tvar<T>
  {
    _dvar(T val);
    std::string ToString();
    bool Set(std::string val);

    const T& operator*() const;
    void operator=(T x);
    T val;
  };

  //! Typed reference for variable
  template<typename T>
  struct _rvar : _tvar<T>
  {
    _rvar(_var& var);

    std::string ToString();
    bool Set(std::string val);

    const T& operator*() const;
    void operator=(T x);
    _var& var;
    mutable T bval;
  };

  template<typename T>
  struct Var : _tvar<T>
  {
    Var(_var& var);
    Var(_dvar<T>& var);

    std::string ToString();
    bool Set(std::string val);

    const T& operator*() const;
    void operator=(T x);

    void Set(T& x);
    T& Ref();

    _rvar<T> wrap;
    _tvar<T>& var;
  };

  template<typename T>
  Var<T> Get(std::string name, T default_value = T() );


  ////////////////////////////////////////////////
  // Implementation
  ////////////////////////////////////////////////

  // _dvar

  template<typename T>
  _dvar<T>::_dvar(T val)
    :val(val)
  {
  }


  template<typename T>
  std::string _dvar<T>::ToString()
  {
    std::ostringstream oss;
    oss << val;
    return oss.str();
  }

  template<typename T>
  bool _dvar<T>::Set(std::string str)
  {
    std::istringstream iss(str);
    iss >> val;
    return iss.fail();
  }

  template<typename T>
  const T& _dvar<T>::operator*() const
  {
    return val;
  }

  template<typename T>
  void _dvar<T>::operator=(T x)
  {
    val = x;
  }

  // _rvar

  template<typename T>
  _rvar<T>::_rvar(_var& var)
    :var(var)
  {
  }

  template<typename T>
  std::string _rvar<T>::ToString()
  {
    return var.ToString();
  }

  template<typename T>
  bool _rvar<T>::Set(std::string str)
  {
    return var.Set(str);
  }

  template<typename T>
  const T& _rvar<T>::operator*() const
  {
    std::istringstream iss(var.ToString());
    iss >> bval;
    return bval;
  }

  template<typename T>
  void _rvar<T>::operator=(T x)
  {
    std::ostringstream oss;
    oss << x;
    var.Set(oss.str());
  }

  // Var

  template<typename T>
  Var<T>::Var(_var& v)
    : wrap(v), var(wrap)
  {
  }

  template<typename T>
  Var<T>::Var(_dvar<T>& v)
    : wrap(v), var(v)
  {
  }

  template<typename T>
  std::string Var<T>::ToString()
  {
    return var.ToString();
  }

  template<typename T>
  bool Var<T>::Set(std::string str)
  {
    return var.Set(str);
  }

  template<typename T>
  const T& Var<T>::operator*() const
  {
    return *var;
  }

  template<typename T>
  void Var<T>::operator=(T x)
  {
    var = x;
  }

  template<typename T>
  void Var<T>::Set(T& x)
  {
    var = x;
  }

  template<typename T>
  T& Var<T>::Ref()
  {
    return *var;
  }


  // Utilities
  extern std::map<std::string,_var*> vars;

  template<typename T>
  Var<T> Get(std::string name, T default_value )
  {
    std::map<std::string,_var*>::iterator vi = vars.find(name);
    if( vi != vars.end() )
    {
      // found
      _dvar<T>* dv = dynamic_cast<_dvar<T>*>(vi->second);
      return dv ? Var<T>(*dv) : Var<T>(*vi->second);
    }else{
      // not found
      _dvar<T>* newvar = new _dvar<T>(default_value);
      vars[name] = newvar;
      return Var<T>(*newvar);
    }
  }

}

//struct Example { int x, y; };
//ostream& operator<<(ostream& s, Example& t) { s << t.x << "," << t.y; return s; }
//istream& operator>>(istream& s, Example& t) {
//  char delim; s >> t.x; s >> delim;
//  if( delim != ',') s.setstate(ios::failbit);
//  s >> t.y; return s;
//}

#endif //PANGOLIN_VARS_H
