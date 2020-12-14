#pragma once

#include <memory>
#include <map>
#include <pangolin/utils/param_set.h>

namespace pangolin
{

class FactoryInterface {
public:
    using Name = std::string;
    using Precedence = int32_t;

    virtual ~FactoryInterface(){};

    virtual std::map<Name,Precedence> Schemes() const = 0;

    virtual const char* Description() const = 0;

    virtual ParamSet Params() const = 0;
};

template<typename T>
class TypedFactoryInterface : public FactoryInterface
{
public:
    virtual std::unique_ptr<T> Open(const Uri& uri) = 0;
};

}
