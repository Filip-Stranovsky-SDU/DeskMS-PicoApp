#include "wsparsing.hpp"

#ifdef __cplusplus
#include <string_view>
#include <algorithm>
namespace sxd {

// Dispatcher constructor initializes table with lambdas capturing `this`
Dispatcher::Dispatcher() {
    table = {{
        { "req:", [this](std::string_view arg){ return api(arg); } }
    }};
}

// Dispatch message: find first matching command prefix and call it
int Dispatcher::dispatch_ws_message(std::string_view message) {
    for (auto& entry : table) {
        if (message.substr(0, entry.name.size()) == entry.name) {
            std::string_view arg = message.substr(entry.name.size());
            return entry.exec(arg);
        }
    }
    return -1; // no command matched
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
    static sxd::Dispatcher dispatcher; // singleton instance
    return dispatcher.dispatch_ws_message(view);
#else
    (void)buff; // unused in pure C
    return -1;
#endif
}

#ifdef __cplusplus
} // extern "C"
#endif
