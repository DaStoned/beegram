#include "Gpio.hpp"
#include "Log.hpp"

#include "driver/gpio.h"
#include <cassert>

namespace beegram {

class Gpio : public IGpio {
public:
    Gpio(Pin pin, bool initial_value)
    : _pin(pin)
    , _set_value(initial_value)
    {
        assert(GPIO_IS_VALID_GPIO(static_cast<int>(pin)));
    }
    virtual bool config(Way way, Mode mode, Pull pull) override;
    virtual bool set(bool value) override;
    virtual bool get() override;
    virtual bool toggle() override;
private:
    Pin _pin;
    bool _set_value;
};

static gpio_mode_t modeTypeToIdf(IGpio::Way way, IGpio::Mode mode) {
    if ((way & IGpio::Way::IN) && (way & IGpio::Way::OUT)) {
        return (mode == IGpio::Mode::PUSH_PULL ? GPIO_MODE_INPUT_OUTPUT : GPIO_MODE_INPUT_OUTPUT_OD);
    } else if (way & IGpio::Way::OUT) {
        return (mode == IGpio::Mode::PUSH_PULL ? GPIO_MODE_OUTPUT : GPIO_MODE_OUTPUT_OD);
    } else if (way & IGpio::Way::IN) {
        return GPIO_MODE_INPUT;
    } else {
        assert(way == IGpio::Way::DISABLED);
        return GPIO_MODE_DISABLE;
    }
}

bool Gpio::config(Way way, Mode mode, Pull pull) {
    
    gpio_config_t iocfg = {};
    iocfg.pin_bit_mask = 1ULL << _pin;
    iocfg.mode = modeTypeToIdf(way, mode);
    iocfg.pull_up_en = (pull & Pull::UP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    iocfg.pull_down_en = (pull & Pull::DOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    iocfg.intr_type = GPIO_INTR_DISABLE;
    esp_err_t ret = gpio_config(&iocfg);
    if (ESP_OK == ret) {
        if (way & Way::OUT) {
            return set(_set_value); // Set initial value if output
        } else {
            return true;
        }
    } else {
        err("Fail GPIO %u config: %u %s", _pin, ret, esp_err_to_name(ret));
        return false;
    }
}

bool Gpio::set(bool value) {
    esp_err_t ret = gpio_set_level(static_cast<gpio_num_t>(_pin), value ? 1 : 0);
    if (ESP_OK == ret) {
        _set_value = value;
        return true;
    } else {
        err("Fail GPIO %u set %u: %u %s", _pin, value ? 1 : 0, ret, esp_err_to_name(ret));
        return false;
    }
}

bool Gpio::get() {
    assert(false);
    return false;
}

bool Gpio::toggle() {
    return set(!_set_value);
}

IGpio::Hnd IGpio::create(IGpio::Pin pin, Way way, Mode mode, Pull pull) {
    auto io = std::make_unique<Gpio>(pin, false);
    if (io->config(way, mode, pull)) {
        return io;
    } else {
        return nullptr;
    }
}

} // namespace