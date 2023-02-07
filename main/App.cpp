#include "App.hpp"
#include "Log.hpp"
#include "Cloud.hpp"
#include "driver/Gpio.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace beegram {

Cloud cloud;

static constexpr Gpio::Pin PIN_RED = 0;
static constexpr Gpio::Pin PIN_GREEN = 2;
static constexpr Gpio::Pin PIN_BLUE = 4;
static constexpr Gpio::Pin PIN_BUTTON = 18;

void App::run() {
    unsigned int i = 0;
    Gpio::Hnd ledRed = Gpio::create(PIN_RED, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    if (!ledRed) {
        err("Fail create LED red");
    }
    Gpio::Hnd ledGreen = Gpio::create(PIN_GREEN, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    if (!ledGreen) {
        err("Fail create LED green");
    }
    Gpio::Hnd ledBlue = Gpio::create(PIN_BLUE, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    if (!ledBlue) {
        err("Fail create LED blue");
    }
    Gpio::Hnd btn = Gpio::create(PIN_BUTTON, Gpio::Way::IN, Gpio::OutMode::PUSH_PULL, Gpio::Pull::UP);
    if (!btn) {
        err("Fail create button");
    }
    auto onEdge = [&]() { ledBlue->toggle(); }; // Not thread safe. No debouncing.
    if (!btn->addIsr(onEdge, Gpio::IntrTrig::ANY_EDGE)) {
        err("Fail add ISR to button");
    }
    

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        cloud.cloudStuff();
        info("Button: %s", btn->get() ? "released" : "pressed");

        switch ((i % 4) / 2) {
        case 0:
            ledRed->toggle();
            break;
        case 1:
            ledGreen->toggle();
            break;
        }
        i++;
    }
}

} // namespace beegram
