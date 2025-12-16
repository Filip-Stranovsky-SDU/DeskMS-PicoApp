#pragma once

#include "OLEDDisplay.hpp"
#include <cstddef>
#include <array>
#include <functional>

namespace sxd {

enum class AlertType { STAND_UP, SIT_DOWN, NONE };


class DisplayHandler {
public:
    static constexpr size_t VISIBLE_ROWS = 3;

    DisplayHandler(OLEDDisplay& disp,
                   const char* const* entries,
                   size_t entryCount);

    void next();     // scroll
    void render();   // redraw current state
    const char* renderUser();
    void renderAlert();
    void setBuffer(const char* const* buff, size_t size);
    void invalidateBuffer();
    void clear();
    void alert(int type);

    void setAlertCallBack(std::function<void()> callback);
        

private:
    OLEDDisplay& display;
    volatile bool buffer_set = false;
    std::function<void()> alertCallback;
    const char* const* buffer;   // external buffer
    size_t count;
    size_t topIndex = 0;
    std::array<char, 20> username;
    AlertType alertType = AlertType::NONE;
    void draw();
};
}