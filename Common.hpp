#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Trade = "Trade";
    static std::string Active = "Act";
    static std::string Completed = "Comp";
    static std::string Balance = "Bal";
}

#endif //CLIENSERVERECN_COMMON_HPP
