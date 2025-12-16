#include "display_call.hpp"
#include <cstring>  // for std::memcpy
#include <string_view>

namespace sxd::commands {

DisplayCall::DisplayCall(DisplayHandler& handler)
    : displayHandler(handler)
{
    // buffer and names are already default-initialized
}

int DisplayCall::operator()(std::string_view arg) {
    // truncate if too long
    size_t len = arg.size();
    if (len >= BUFFER_SIZE) {
        len = BUFFER_SIZE - 1;
    }

    // copy into stack buffer
    std::memcpy(buffer.data(), arg.data(), len);
    buffer[len] = '\0';

    // parse in-place
    parseBuffer(len);

    // push to display
    displayHandler.setBuffer(names, nameCount);
    // displayHandler.render();

    return 0;
}

void DisplayCall::parseBuffer(size_t /*len*/) {
    nameCount = 0;
    char* ptr = buffer.data();

    if (*ptr == '\0') return;  // empty message

    names[nameCount++] = ptr;

    while (*ptr != '\0' && nameCount < MAX_NAMES) {
        if (*ptr == ',') {
            *ptr = '\0';                  // terminate previous name
            ++ptr;
            if (*ptr != '\0') {
                names[nameCount++] = ptr; // next name
            }
        } else {
            ++ptr;
        }
    }
}
int DisplayCall::alert(std::string_view arg) {
    if(arg[0] == '2') displayHandler.alert(2);
    if(arg[0] == '1') displayHandler.alert(1);
    return 0;
}
}