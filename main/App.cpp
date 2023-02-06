#include "App.hpp"
#include "Log.hpp"
#include "Cloud.hpp"
#include "driver/Gpio.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace beegram {

Cloud cloud;

static constexpr IGpio::Pin PIN_RED = 0;
static constexpr IGpio::Pin PIN_GREEN = 2;
static constexpr IGpio::Pin PIN_BLUE = 4;
static constexpr IGpio::Pin PIN_BUTTON = 18;

void App::run() {
    unsigned int i = 0;
    IGpio::Hnd ledRed = IGpio::create(PIN_RED, IGpio::Way::OUT, IGpio::Mode::PUSH_PULL, IGpio::Pull::NONE);
    if (!ledRed) {
        err("Fail create LED red");
    }
    IGpio::Hnd ledGreen = IGpio::create(PIN_GREEN, IGpio::Way::OUT, IGpio::Mode::PUSH_PULL, IGpio::Pull::NONE);
    if (!ledGreen) {
        err("Fail create LED green");
    }
    IGpio::Hnd ledBlue = IGpio::create(PIN_BLUE, IGpio::Way::OUT, IGpio::Mode::PUSH_PULL, IGpio::Pull::NONE);
    if (!ledBlue) {
        err("Fail create LED red");
    }
    IGpio::Hnd btn = IGpio::create(PIN_BUTTON, IGpio::Way::IN, IGpio::Mode::PUSH_PULL, IGpio::Pull::UP);
    if (!btn) {
        err("Fail create button");
    }
    

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        cloud.cloudStuff();
        info("Button: %s", btn->get() ? "released" : "pressed");

        switch ((i % 6) / 2) {
        case 0:
            ledRed->toggle();
            break;
        case 1:
            ledGreen->toggle();
            break;
        case 2:
            ledBlue->toggle();
            break;
        }
        i++;
    }
}

} // namespace beegram
