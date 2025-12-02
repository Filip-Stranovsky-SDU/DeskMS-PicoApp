#include <stdio.h>
#include "pico/stdlib.h"
#include "Led.hpp"
#include "dbop.h"       

 // Constructor – initializes the LED GPIO
Led::Led(uint pinNumber) : pin(pinNumber), state(false) {
    /*TODO - Write code line*/                              C_Led("Initialize LED GPIO");
    /*TODO - Write code line*/                              C_Led("LED configured as output");
    /*TODO - Write code line*/                              C_Led("Ensure LED starts OFF");
}

// Turn LED ON
void Led::on() {
    /*TODO - Write code line*/                              C_Led("Update LED state to ON");
    /*TODO - Write code line*/                              C_Led("Switch LED ON");
}

// Turn LED OFF
void Led::off() {
    /*TODO - Write code line*/                              C_Led("Update LED state to OFF");
    /*TODO - Write code line*/                              C_Led("Switch LED OFF");
}

// Toggle LED state
void Led::toggle() {
    /*TODO - Write code line*/                              C_Led("Toggle LED state variable");
    /*TODO - Write code line*/                              C_Led("Write toggled state to GPIO");
}

// Set LED state directly
void Led::setState(bool s) {
    /*TODO - Write code line*/                              C_Led("Set LED state variable directly");
    /*TODO - Write code line*/                              C_Led("Write new state to GPIO");
}

// Return current LED state
bool Led::isOn() const {
    return state;                                           C_Led("Return current LED state");
}
