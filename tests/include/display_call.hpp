#pragma once
#include <string_view>
#include "DisplayHandler.hpp"

namespace sxd::commands {
class DisplayCall {
public:
    DisplayCall(DisplayHandler&) {}
    int operator()(std::string_view arg) { last_arg = arg; return 1; }
    int alert(std::string_view arg) { last_arg = arg; return 2; }

    std::string_view last_arg{};
};
}
