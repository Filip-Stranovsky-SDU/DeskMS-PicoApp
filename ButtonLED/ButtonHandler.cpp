#include "ButtonHandler.hpp"
#include "DisplayHandler.hpp"
extern "C" {
    #include "tls_common.h"
}
#include "pico/stdlib.h"
namespace sxd {
ButtonHandler::ButtonHandler(Button& button, 
                            DisplayHandler& displayHandler,
                            TLS_CLIENT_T* state) : btn(button), dh(displayHandler), tls_state(state) {}

void ButtonHandler::update() {
    // Check if button pressed
    if (btn.isPressed()) {
        if (!holding) {
            holding = true;
            pressStartTime = get_absolute_time();
        }
        lastActionTime = get_absolute_time();

        uint64_t heldMs = absolute_time_diff_us(pressStartTime, get_absolute_time()) / 1000;
        switch (state) {
            case UIState::NO_USER:
                if (heldMs >= HOLD_3S_MS) { switchUserMenu(); holding = false; }
                break;
            case UIState::USER:
                if (heldMs >= HOLD_7S_MS) { removeUser(); holding = false; }
                break;
            case UIState::USER_MENU:
                if (heldMs >= HOLD_3S_MS) { dismissMenu(); holding = false; }
                break;
            default: break;
        }
    } else if (holding) {
        // Button released
        holding = false;
        lastClickTime = get_absolute_time();
        clickCount++;
    }

    // Handle click count timeout
    if (clickCount > 0 && absolute_time_diff_us(lastClickTime, get_absolute_time()) / 1000 > DOUBLE_CLICK_MS) {
        handleClicks(clickCount);
        clickCount = 0;
        lastActionTime = get_absolute_time();
    }

    if (state == UIState::USER_MENU) {
        uint64_t idleMs = absolute_time_diff_us(lastActionTime, get_absolute_time()) / 1000;
        dh.render();
        if (idleMs >= HOLD_10S_MS) {
            addUser();                 // call your function
            lastActionTime = get_absolute_time(); // reset timer to avoid repeated calls
        }
    }
    if (state == UIState::USER) {
        dh.renderUser();
    }
    if (state == UIState::ALERT) {
        dh.renderAlert();
    }

}

void ButtonHandler::handleClicks(int clicks) {
    switch (state) {
        case UIState::ALERT:
            if (clicks == 1) confirm();
            else if (clicks == 2) dismiss();
            break;
        case UIState::USER_MENU:
            if (clicks == 1) nextUser();
            break;
        default: break;
    }
}

void ButtonHandler::switchUserMenu() {
    state = UIState::USER_MENU;
    printf("switchUserMenu");
    dh.invalidateBuffer();
    ws_send_text(tls_state->pcb, "req:names");
    lastActionTime = get_absolute_time();


}

void ButtonHandler::removeUser() {
    state = UIState::NO_USER;
    dh.clear();
    printf("removeUser");
    // TODO: Remove user
}

void ButtonHandler::confirm() {
    state = UIState::USER;
    printf("confirm");
    ws_send_text(tls_state->pcb, "alert:confirm");
    // TODO: Confirm action
    
}

void ButtonHandler::dismiss() {
    state = UIState::USER;
    printf("dismiss");
    ws_send_text(tls_state->pcb, "alert:dismiss");
    // TODO: Dismiss alert
}

void ButtonHandler::nextUser() {
    printf("nextUser");
    lastActionTime = get_absolute_time();

    dh.next();
}

void ButtonHandler::dismissMenu() {
    state = UIState::NO_USER;
    dh.clear();
    printf("dismissMenu");
    // TODO: Dismiss user menu
}

void ButtonHandler::addUser() {
    state = UIState::USER;
    dh.clear();
    const char* user = dh.renderUser();
    printf("User selected: %s\n\n", user);
    char buf[30];
    snprintf(buf, 30,"user:%s" ,user);
    ws_send_text(tls_state->pcb, buf);
    printf("addUser");
}

void ButtonHandler::setAlert() {
    state = UIState::ALERT;
}

}