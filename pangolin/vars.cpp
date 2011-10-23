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

#include "vars.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace boost;

namespace pangolin
{

boost::ptr_unordered_map<string,_Var> vars;
vector<NewVarCallback> new_var_callbacks;
vector<GuiVarChangedCallback> gui_var_changed_callbacks;

void RegisterNewVarCallback(NewVarCallbackFn callback, void* data, const std::string& filter)
{
  new_var_callbacks.push_back(NewVarCallback(filter,callback,data));
}

void RegisterGuiVarChangedCallback(GuiVarChangedCallbackFn callback, void* data, const std::string& filter)
{
  gui_var_changed_callbacks.push_back(GuiVarChangedCallback(filter,callback,data));
}

// Find the open brace preceeded by '$'
const char* FirstOpenBrace(const char* str)
{
  bool symbol = false;

  for(; *str != '\0'; ++str ) {
    if( *str == '$') {
      symbol = true;
    }else{
      if( symbol ) {
        if( *str == '{' ) {
          return str;
        } else {
          symbol = false;
        }
      }
    }
  }
  return 0;
}

// Find the first matching end brace. str includes open brace
const char* MatchingEndBrace(const char* str)
{
  int b = 0;
  for(; *str != '\0'; ++str ) {
    if( *str == '{' ) {
      ++b;
    }else if( *str == '}' ) {
      --b;
      if( b == 0 ) {
        return str;
      }
    }
  }
  return 0;
}

// Recursively expand val
string ProcessVal(const string& val )
{
  string expanded = val;

  while(true)
  {
    const char* brace = FirstOpenBrace(expanded.c_str());
    if(brace)
    {
      const char* endbrace = MatchingEndBrace(brace);
      if( endbrace )
      {
        iterator_range<const char*> out(brace-1,endbrace+1);
        iterator_range<const char*> in(brace+1,endbrace);
        string inexpand = ProcessVal(copy_range<string>(in));
        Var<string> var(inexpand,"#");
        if( !((const string)var).compare("#"))
        {
            std::cerr << "Unabled to expand: [" << inexpand << "].\nMake sure it is defined and terminated with a semi-colon." << endl << endl;
        }
        ostringstream oss;
        oss << copy_range<string>(iterator_range<const char*>(expanded.c_str(), brace-1));
        oss << (const string)var;
        oss << copy_range<string>(iterator_range<const char*>(endbrace+1, expanded.c_str() + expanded.length() ));
        expanded = oss.str();
//        expanded = replace_range_copy(expanded,out,(const string)var);
//        cout << copy_range<std::string>(in) << endl;
        continue;
      }
    }
    break;
  }

  return expanded;
}

void AddVar(const string& name, const string& val )
{
  boost::ptr_unordered_map<std::string,_Var>::iterator vi = vars.find(name);
  const bool exists_already = vi != vars.end();

  string full = ProcessVal(val);
  Var<string> var(name);
  var = full;

  // Mark as upgradable if unique
  if(!exists_already)
  {
    var.var->generic = true;
  }
}

#ifdef ALIAS
void AddAlias(const string& alias, const string& name)
{
    boost::ptr_unordered_map<std::string,_Var>::iterator vi = vars.find(name);

    if( vi != vars.end() )
    {
        //cout << "Adding Alias " << alias << " to " << name << endl;
        _Var * v = vi->second;
        vars[alias].create(v->val,v->val_default,v->type_name);
        vars[alias].meta_friendly = alias;
        v->generic = false;
        vars[alias].generic = false;
    }else{
        cout << "Variable " << name << " does not exist to alias." << endl;
    }
}
#endif

void ParseVarsFile(const string& filename)
{
  ifstream f(filename.c_str());

  if( f.is_open() )
  {
    while( !f.bad() && !f.eof())
    {
      const int c = f.peek();

      if( isspace(c) )
      {
        // ignore leading whitespace
        f.get();
      }else{
        if( c == '#' || c == '%' )
        {
          // ignore lines starting # or %
          string comment;
          getline(f,comment);
        }else{
          // Otherwise, find name and value, seperated by '=' and ';'
          string name;
          string val;
          getline(f,name,'=');
          getline(f,val,';');
          boost::trim_if(name, boost::is_any_of(" \t\n\r"));
          boost::trim_if(val, boost::is_any_of(" \t\n\r"));

          if( name.size() >0 && val.size() > 0 )
          {
            if( !val.substr(0,1).compare("@") )
            {
#ifdef ALIAS
                AddAlias(name,val.substr(1));
#endif
            }else{
                AddVar(name,val);
            }
          }
        }
      }
    }
    f.close();
  }else{
    cerr << "Unable to open '" << filename << "' for configuration data" << endl;
  }
}

}
