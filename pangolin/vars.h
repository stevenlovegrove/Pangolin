#ifndef PANGOLIN_VARS_H
#define PANGOLIN_VARS_H

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
  Var(_Var& var);
  operator const T& ();
  const T* operator->();
  void operator=(const T& val);

  Accessor<T>* a;
};

typedef void (*NewVarCallbackFn)(const std::string& name, _Var& var);
void RegisterNewVarCallback(NewVarCallbackFn callback, const std::string& filter = "");

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

template<typename T>
inline Var<T>::Var(_Var& var)
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
  NewVarCallback(const std::string& filter, NewVarCallbackFn fn)
    :filter(filter),fn(fn) {}
  std::string filter;
  NewVarCallbackFn fn;
};

extern std::map<std::string,_Var> vars;
extern std::vector<NewVarCallback> callbacks;

template<typename T>
Var<T>::Var(const std::string& name, T default_value)
{
  std::map<std::string,_Var>::iterator vi = vars.find(name);
  if( vi != vars.end() )
  {
    // found
    a = Accessor<T>::Create(vi->second);
  }else{
    _Var* var = &vars[name];
    // not found
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

//    // notify those watching new variables
//    for( std::vector<NewVarCallback>::iterator i = callbacks.begin(); i!=callbacks.end(); ++i )
//    {
//      if( i->filter.length() <= name.length() && name.compare(0,i->filter.length(),i->filter) == 0 )
//      {
//        i->fn(name,*newvar);
//      }
//    }
  }
}

template<typename T>
struct _tvar {};

}

#endif //PANGOLIN_VARS_H
