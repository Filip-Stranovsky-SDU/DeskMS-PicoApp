#include "DisplayHandler.hpp"

#include <stdio.h>

static constexpr int ROW_HEIGHT = 10;
static constexpr int START_X = 0;
static constexpr int START_Y = 0;
namespace sxd{
DisplayHandler::DisplayHandler(OLEDDisplay& disp,
                               const char* const* entries,
                               size_t entryCount)
    : display(disp),
      buffer(entries),
      count(entryCount)
{
}

void DisplayHandler::next() {
    if (count == 0) return;

    topIndex++;
    if (topIndex >= count) {
        topIndex = 0;
    }
}

void DisplayHandler::render() {
    if (!buffer_set) {
        return;
    }
    draw();
}

void DisplayHandler::draw() {
    display.clear();

    for (size_t row = 0; row < VISIBLE_ROWS; ++row) {
        size_t idx = topIndex + row;
        if (idx >= count) {
            idx -= count;   // wrap
        }
        if(row == 1) {
            display.writeText(
                START_X,
                START_Y + row * ROW_HEIGHT,
                ">"
            );
            display.writeText(
                START_X + 10,
                START_Y + row * ROW_HEIGHT,
                buffer[idx]
            );
            continue;
        }

        display.writeText(
            START_X,
            START_Y + row * ROW_HEIGHT,
            buffer[idx]
        );
    }

    display.render();
}
const char* DisplayHandler::renderUser() {
    display.clear();
    constexpr size_t row = 1;
    size_t idx = topIndex + row;
    if (idx >= count) {
        idx -= count;   // wrap
    }
     display.writeText(
        START_X,
        START_Y + row * ROW_HEIGHT,
        buffer[idx]
    );
    display.render();
    return buffer[idx];
}

void DisplayHandler::renderAlert() {
    display.clear();
    if(alertType == AlertType::STAND_UP) {
        display.writeText(
            START_X,
            START_Y + 0 * ROW_HEIGHT,
            "Alert: Stand up"
        );
    }
    if(alertType == AlertType::SIT_DOWN) {
        display.writeText(
            START_X,
            START_Y + 0 * ROW_HEIGHT,
            "Alert: Sit down"
        );
    }

    display.writeText(
        START_X,
        START_Y + 1 * ROW_HEIGHT,
        "Accept: 1 click"
    );
    display.writeText(
        START_X,
        START_Y + 2 * ROW_HEIGHT,
        "Decline: 2 click"
    );
    display.render();
}

void DisplayHandler::setBuffer(const char* const* buff, size_t size){
    buffer = buff;
    count = size;
    buffer_set = true;
    printf("BufferSet\n");
}

void DisplayHandler::invalidateBuffer() {
    buffer_set = false;
}
void DisplayHandler::clear() {
    display.clear();
}

void DisplayHandler::alert(int type) {
    alertCallback();
    switch (type) {
        case 1:
            alertType = AlertType::STAND_UP;
            break;
        case 2:
            alertType = AlertType::SIT_DOWN;
            break;
        default:
            alertType = AlertType::NONE;
            break;
    }

}

void DisplayHandler::setAlertCallBack(std::function<void()> callback) {
    alertCallback = callback;
}

}