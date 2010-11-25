#ifndef PANGOLIN_VARS_H
#define PANGOLIN_VARS_H

#include <sstream>
#include <map>
#include <vector>
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
    _rvar(const _rvar<T>& cpy);
    _rvar(_var& var);

    std::string ToString();
    bool Set(std::string val);

    const T& operator*() const;
    void operator=(T x);
    _var* var;
    mutable T bval;
  };

  template<typename T>
  struct Var : _tvar<T>
  {
//    Var(const Var<T>& copy);
    Var(const std::string& name, T default_value = T() );
    Var(_var& var);
    Var(_dvar<T>& var);

    std::string ToString();
    bool Set(std::string val);

    const T& operator*() const;
    void operator=(T x);
    void operator=(const Var<T>& x);

    void Set(T& x);
    T& Ref();

    _rvar<T> wrap;
    _tvar<T>* var;
    bool wrapped;
  };

  template<typename T>
  Var<T> Get(const std::string& name, T default_value = T() );

  typedef void (*NewVarCallbackFn)(const std::string& name, _var& var);

  void RegisterNewVarCallback(NewVarCallbackFn callback, const std::string& filter = "");

  ////////////////////////////////////////////////
  // Implementation
  ////////////////////////////////////////////////

  static _dvar<int> ddummy(0);
  static _var& dummy = ddummy;

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
  _rvar<T>::_rvar(const _rvar<T>& cpy)
    :var(cpy.var)
  {
  }

  template<typename T>
  _rvar<T>::_rvar(_var& var)
    :var(&var)
  {
  }

  template<typename T>
  std::string _rvar<T>::ToString()
  {
    return var->ToString();
  }

  template<typename T>
  bool _rvar<T>::Set(std::string str)
  {
    return var->Set(str);
  }

  template<typename T>
  const T& _rvar<T>::operator*() const
  {
    std::istringstream iss(var->ToString());
    iss >> bval;
    return bval;
  }

  template<typename T>
  void _rvar<T>::operator=(T x)
  {
    std::ostringstream oss;
    oss << x;
    var->Set(oss.str());
  }

  // Var

  template<typename T>
  Var<T>::Var(_var& v)
    : wrap(v), var(&wrap), wrapped(true)
  {
  }

  template<typename T>
  Var<T>::Var(_dvar<T>& v)
    : wrap(v), var(&v), wrapped(false)
  {
  }

//  template<typename T>
//  Var<T>::Var(const Var<T>& copy)
//    :wrap(copy.wrap), var(copy.wrapped ? &wrap : copy.var ), wrapped(copy.wrapped)
//  {
//  }

  template<typename T>
  void Var<T>::operator=(const Var<T>& copy)
  {
    wrap.var = copy.wrap.var;
    wrapped = copy.wrapped;
    var = wrapped ? &wrap : copy.var;
  }

  template<typename T>
  Var<T>::Var(const std::string& name, T default_value )
    : wrap(dummy), var(NULL), wrapped(false)
  {
    *this = Get<T>(name,default_value);
  }

  template<typename T>
  std::string Var<T>::ToString()
  {
    return var->ToString();
  }

  template<typename T>
  bool Var<T>::Set(std::string str)
  {
    return var->Set(str);
  }

  template<typename T>
  const T& Var<T>::operator*() const
  {
    return *(*var);
  }

  template<typename T>
  void Var<T>::operator=(T x)
  {
    *var = x;
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

  struct NewVarCallback
  {
    NewVarCallback(const std::string& filter, NewVarCallbackFn fn)
      :filter(filter),fn(fn) {}
    std::string filter;
    NewVarCallbackFn fn;
  };

  extern std::map<std::string,_var*> vars;
  extern std::vector<NewVarCallback> callbacks;

  template<typename T>
  Var<T> Get(const std::string& name, T default_value )
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

      // notify those watching new variables
      for( std::vector<NewVarCallback>::iterator i = callbacks.begin(); i!=callbacks.end(); ++i )
      {
        if( i->filter.length() <= name.length() && name.compare(0,i->filter.length(),i->filter) == 0 )
        {
          i->fn(name,*newvar);
        }
      }

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
