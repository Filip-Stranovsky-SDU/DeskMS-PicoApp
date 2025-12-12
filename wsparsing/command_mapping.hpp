#pragma once

#include <string_view>
#include <functional>

namespace sxd {

struct CommandMapping {
    std::string_view name;
    std::function<int(std::string_view)> exec;
};

}