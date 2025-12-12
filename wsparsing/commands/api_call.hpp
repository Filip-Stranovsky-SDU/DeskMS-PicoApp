#pragma once

#include <string_view>

namespace sxd::commands{

class ApiCall {
    public:
        int operator()(std::string_view arg);

};


}