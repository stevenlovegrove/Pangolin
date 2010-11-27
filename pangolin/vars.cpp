#include "vars.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace pangolin
{

map<string,_Var> vars;
vector<NewVarCallback> callbacks;

void RegisterNewVarCallback(NewVarCallbackFn callback, void* data, const std::string& filter)
{
  callbacks.push_back(NewVarCallback(filter,callback,data));
}

}
