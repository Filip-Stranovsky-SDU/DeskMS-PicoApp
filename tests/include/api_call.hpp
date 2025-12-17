#pragma once
#include <string_view>

namespace sxd::commands {
class ApiCall {
public:
    ApiCall() {}
    int operator()(std::string_view arg) {
        last_arg = arg;
        return 42; // return dummy code
    }
    int store_json(std::string_view arg) {
        last_arg = arg;
        return 43; // return dummy code
    }
    void poll(void* arg) {
        last_arg_ptr = arg;
    }

    std::string_view last_arg{};
    void* last_arg_ptr{};
};
}
