#pragma once
#include "PushButton.hpp"
#include <cstdint>
#include "DisplayHandler.hpp"
#include "lwip/err.h"      // defines err_t

#include "lwip/ip_addr.h"      // defines err_t

extern "C" {
    #include "tls_common.h"
}

enum class UIState { ALERT, NO_USER, USER_MENU, USER };
namespace sxd {
class ButtonHandler {
    Button& btn;
    DisplayHandler& dh;
    TLS_CLIENT_T* tls_state;
    UIState state = UIState::NO_USER;

    int clickCount = 0;
    absolute_time_t lastClickTime;
    absolute_time_t pressStartTime;
    absolute_time_t lastActionTime = get_absolute_time();

    bool holding = false;

    static constexpr uint64_t DOUBLE_CLICK_MS = 500;
    static constexpr uint64_t HOLD_3S_MS = 3000;
    static constexpr uint64_t HOLD_7S_MS = 7000;
    static constexpr uint64_t HOLD_10S_MS = 10000;

public:
    ButtonHandler(Button& button, DisplayHandler& displayHandler, TLS_CLIENT_T* tls_state);

    // Must be called periodically (e.g., in main loop)
    void update();
    void setAlert();

private:
    void handleClicks(int clicks);

    void switchUserMenu();
    void removeUser();
    void confirm();
    void dismiss();
    void nextUser();
    void dismissMenu();
    void addUser();
};

}