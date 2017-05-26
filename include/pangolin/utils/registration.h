#pragma once

#include <functional>

namespace pangolin {

template<typename TokenType>
class Registration
{
public:
    using UnregisterFunc = std::function<void(const TokenType&)>;

    Registration(TokenType token, UnregisterFunc unregister)
        : token(token), unregister(unregister)
    {

    }

    // No copy constructor
    Registration(const Registration&) = delete;

    // Default move constructor
    Registration(Registration&&) = default;

    ~Registration()
    {
        if(unregister) {
            unregister(token);
        }
    }

    const TokenType& Token()
    {
        return token;
    }

private:
    TokenType token;
    UnregisterFunc unregister;
};

}
