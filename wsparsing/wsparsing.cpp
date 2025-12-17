#include "wsparsing.hpp"

#ifdef __cplusplus
#include <cstdio>
#include <string_view>
#include <algorithm>


namespace sxd {
    // NEEDS TO BE INITIALIZED BEFORE WEBSOCKET
    Dispatcher* defaultDispatcher = nullptr; // actual definition

// Dispatcher constructor initializes table with lambdas capturing `this`
Dispatcher::Dispatcher(DisplayHandler& display) : dc(display) {
    table = {{
        { "req:", [this](std::string_view arg){ return api(arg); } },
        { "json:", [this](std::string_view arg){ return api.store_json(arg); } },
        { "names:", [this](std::string_view arg){ return dc(arg); }},
        { "alert:", [this](std::string_view arg){ return dc.alert(arg); }}
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

void Dispatcher::poll(void* state) {
    api.poll(state);
}
void Dispatcher::set_dispatch_table(const std::array<CommandMapping, DISPATCH_TABLE_SIZE> &dt) {
    table = dt;
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
    if(!sxd::defaultDispatcher) {
        printf("Default Dispatcher uninitialized!!!!!!!!!!!!\n");
        return -1;
    }
    std::string_view view(buff);
    printf("%s XD\n", view.data());
    return sxd::defaultDispatcher->dispatch_ws_message(view);
#else
    (void)buff; // unused in pure C
    return -1;
#endif
}

void poll_dispatcher(void* arg) {
    if(!sxd::defaultDispatcher) {
        printf("Default Dispatcher uninitialized!!!!!!!!!!!!\n");
        return;
    }
    sxd::defaultDispatcher->poll(arg);
}


#ifdef __cplusplus
} // extern "C"
#endif
