#include "wsparsing.hpp"

#include "tls_common.h"

#ifdef __cplusplus
#include <cstdio>
#include <string_view>
#include <algorithm>


namespace sxd {
    sxd::Dispatcher dispatcher; // singleton instance

// Dispatcher constructor initializes table with lambdas capturing `this`
Dispatcher::Dispatcher() {
    table = {{
        { "req:", [this](std::string_view arg){ return api(arg); } },
        { "json:", [this](std::string_view arg){ return api.store_json(arg); } }
    }};
}

// Dispatch message: find first matching command prefix and call it
int Dispatcher::dispatch_ws_message(std::string_view message) {
    printf("%s XD\n", message);
    for (auto& entry : table) {
        if (message.substr(0, entry.name.size()) == entry.name) {
            
            std::string_view arg = message.substr(entry.name.size());
            return entry.exec(arg);
        }
    }
    return -1; // no command matched
}

void Dispatcher::poll(void* state) {
    api.poll(state);
}

} // namespace sxd
#endif

// --------------------- C API bridge ---------------------

#ifdef __cplusplus
extern "C" {
#endif

int handle_ws_message(const char* buff) {
    if (!buff) return -1;

#ifdef __cplusplus
    std::string_view view(buff);
    return sxd::dispatcher.dispatch_ws_message(view);
#else
    (void)buff; // unused in pure C
    return -1;
#endif
}

void poll_dispatcher(void* arg) {
    sxd::dispatcher.poll(arg);
}


#ifdef __cplusplus
} // extern "C"
#endif
