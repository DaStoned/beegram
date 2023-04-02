#include "App.hpp"
#include "Log.hpp"
#include "Cloud.hpp"
#include "Param.hpp"
#include "Ush.hpp"
#include "Bosun.hpp"
#include "Scales.hpp"
#include "driver/Gpio.hpp"
#include "driver/Hx711.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cassert>

namespace beegram {

Cloud cloud;

static constexpr Gpio::Pin PIN_RED = 0;
static constexpr Gpio::Pin PIN_GREEN = 2;
static constexpr Gpio::Pin PIN_BLUE = 4;
static constexpr Gpio::Pin PIN_BUTTON = 18;
static constexpr Gpio::Pin PIN_LOADSENSOR_DOUT = 22;
static constexpr Gpio::Pin PIN_LOADSENSOR_SCK = 19;

void App::run() {
    unsigned int i = 0;
    auto ledRed = Gpio::create(PIN_RED, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    assert(ledRed);
    auto ledGreen = Gpio::create(PIN_GREEN, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    assert(ledGreen);
    auto ledBlue = Gpio::create(PIN_BLUE, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    assert(ledBlue);
    auto btn = Gpio::create(PIN_BUTTON, Gpio::Way::IN, Gpio::OutMode::PUSH_PULL, Gpio::Pull::UP);
    assert(btn);
    // auto onEdge = [&]() { ledBlue->toggle(); }; // Not thread safe. No debouncing.
    // if (!btn->addIsr(onEdge, Gpio::IntrTrig::ANY_EDGE)) {
    //     err("Fail add ISR to button");
    // }

    auto param = Param::create("nvs", "params");
    assert(param);
    
    uint32_t bootCount = param->getU32("bootCount").value_or(0);
    info("Boot count: %lu", bootCount);
    bool ret = param->setU32("bootCount", bootCount + 1);
    assert(ret);

    auto loadSensor = Hx711::create();
    assert(loadSensor);
    if (!loadSensor->init(PIN_LOADSENSOR_DOUT, PIN_LOADSENSOR_SCK, Hx711::Mode::CH_A_GN64)) {
        err("Fail init load sensor");
    }

    auto bosun = Bosun::create();
    assert(bosun);
    if (!bosun->init()) {
        err("Fail init Bosun");
    }

    auto ush = Ush::create(*bosun);
    assert(ush);
    if (!ush->start(0)) {
        err("Fail start ush");
    }

    auto scales = Scales::create(*param, *bosun, *loadSensor);
    assert(scales);
    if (!scales->init()) {
        err("Fail init Scales");
    }
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        cloud.cloudStuff();
        int sample = loadSensor->read();
        float weight = scales->weigh();
        info("Weight: %0.3f, load: 0x%06X (%d)", weight, sample, sample);

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
