#ifndef PANGOLIN_VARS_H
#define PANGOLIN_VARS_H

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>

#include "vars_internal.h"

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

template<typename T>
struct Var
{
  Var(const std::string& name, T default_value = T());
  Var(const std::string& name, T default_value, bool toggle);
  Var(const std::string& name, T default_value, double min, double max);
  Var(_Var& var);

  operator const T& ();
  const T* operator->();
  void operator=(const T& val);

  void init(const std::string& name, T default_value, double min = 0, double max = 1, int flags = 1);

  _Var* var;
  Accessor<T>* a;
};

bool Pushed(Var<bool>& button);

typedef void (*NewVarCallbackFn)(void* data, const std::string& name, _Var& var);
void RegisterNewVarCallback(NewVarCallbackFn callback, void* data, const std::string& filter = "");

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

template<typename T>
inline Var<T>::Var(const std::string& name, T default_value)
{
  init(name,default_value);
}

template<typename T>
inline Var<T>::Var(const std::string& name, T default_value, bool toggle)
{
  init(name,default_value, 0, 1, toggle);
}

template<typename T>
inline Var<T>::Var(const std::string& name, T default_value, double min, double max)
{
  init(name,default_value, min, max);
}

template<typename T>
inline Var<T>::Var(_Var& var)
  : var(&var)
{
  a = Accessor<T>::Create(var);
}

template<typename T>
inline Var<T>::operator const T& ()
{
  return a->Get();
}

template<typename T>
inline const T* Var<T>::operator->()
{
  return &(a->Get());
}

template<typename T>
inline void Var<T>::operator=(const T& val)
{
  a->Set(val);
}

struct NewVarCallback
{
  NewVarCallback(const std::string& filter, NewVarCallbackFn fn, void* data)
    :filter(filter),fn(fn),data(data) {}
  std::string filter;
  NewVarCallbackFn fn;
  void* data;
};

extern std::map<std::string,_Var> vars;
extern std::vector<NewVarCallback> callbacks;

template<typename T>
inline void Var<T>::init(const std::string& name, T default_value, double min, double max, int flags)
{
  std::map<std::string,_Var>::iterator vi = vars.find(name);
  if( vi != vars.end() )
  {
    // found
    var = &vi->second;
    a = Accessor<T>::Create(vi->second);
  }else{
    // not found
    var = &vars[name];
    if( boost::is_same<T,bool>::value ) {
      *var = _Var(new bool, typeid(bool).name() );
      a = new _Accessor<T,bool>( *(bool*)var->val );
    }else if( boost::is_integral<T>::value ) {
      *var = _Var(new int, typeid(int).name() );
      a = new _Accessor<T,int>( *(int*)var->val );
    }else if( boost::is_scalar<T>::value ) {
      *var = _Var(new double, typeid(double).name() );
      a = new _Accessor<T,double>( *(double*)var->val );
    }else{
      *var = _Var(
        new std::string(boost::lexical_cast<std::string>(default_value)),
        typeid(std::string).name()
      );
      a = new _Accessor<T,std::string>( *(std::string*)var->val );
    }
    a->Set(default_value);

    std::vector<std::string> parts;
    boost::split(parts,name,boost::is_any_of("."));

    // Meta info for variable
    var->meta_friendly = parts.size() > 0 ? parts[parts.size()-1] : "";
    var->meta_range[0] = min;
    var->meta_range[1] = max;
    var->meta_flags = flags;

    // notify those watching new variables
    BOOST_FOREACH(NewVarCallback& nvc, callbacks)
      if( boost::starts_with(name,nvc.filter) )
        nvc.fn(nvc.data,name,*var);
  }
}

inline bool Pushed(Var<bool>& button)
{
  bool val = button;
  button = false;
  return val;
}


}

//struct Example { Example() {} Example(int x, int y) :x(x),y(y) {} int x, y; };
//ostream& operator<<(ostream& s, const Example& t) { s << t.x << "," << t.y; return s; }
//istream& operator>>(istream& s, Example& t) {
//  char delim; s >> t.x; s >> delim;
//  if( delim != ',') s.setstate(ios::failbit);
//  s >> t.y; return s;
//}

#endif //PANGOLIN_VARS_H
