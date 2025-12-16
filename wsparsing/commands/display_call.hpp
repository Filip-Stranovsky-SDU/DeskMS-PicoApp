#pragma once

#include <array>
#include "DisplayHandler.hpp"
#include <string_view>


namespace sxd::commands{

class DisplayCall {
public:
    static constexpr size_t BUFFER_SIZE = 4096;
    static constexpr size_t MAX_NAMES   = 32;

    DisplayCall(DisplayHandler& handler);

    int operator()(std::string_view arg);
    int alert(std::string_view arg);

private:
    DisplayHandler& displayHandler;

    std::array<char, BUFFER_SIZE> buffer;  // stack-allocated array
    const char* names[MAX_NAMES];
    size_t nameCount = 0;

    void parseBuffer(size_t len);
};

}