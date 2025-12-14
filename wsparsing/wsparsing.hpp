#pragma once


#ifdef __cplusplus

extern "C" {
#endif

// ---- C API exposed to .c files ----
int handle_ws_message(const char* buff);
void poll_dispatcher(void* state);

#ifdef __cplusplus
} // extern "C"
#endif


// ---- C++ API (NOT visible to C compiler) ----
#ifdef __cplusplus


#include <string_view>
#include <array>

#include "command_mapping.hpp"
#include "api_call.hpp"

namespace sxd{

class Dispatcher {
public:
    Dispatcher();

    int dispatch_ws_message(std::string_view message);
    void poll(void* arg);


private:
    commands::ApiCall api;
    std::array<CommandMapping, 2> table;

};


}

#endif
