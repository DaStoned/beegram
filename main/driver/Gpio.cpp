#include "Gpio.hpp"
#include "Log.hpp"

#include "driver/gpio.h"
#include <cassert>

namespace beegram {

class GpioImpl : public Gpio {
public:
    GpioImpl(Pin pin, bool initial_value)
    : _pin(pin)
    , _set_value(initial_value)
    , _isr()
    {
        assert(GPIO_IS_VALID_GPIO(static_cast<int>(pin)));
    }
    virtual bool config(Way way, OutMode outMode, Pull pull) override;
    virtual bool set(bool value) override;
    virtual bool get() override;
    virtual bool toggle() override;
    virtual void reset() override;
    virtual bool addIsr(Isr isr, IntrTrig type) override;
private:
    Pin _pin;
    bool _set_value;
    Isr _isr;
};

static gpio_mode_t modeTypeToIdf(Gpio::Way way, Gpio::OutMode outMode) {
    if ((way & Gpio::Way::IN) && (way & Gpio::Way::OUT)) {
        return (outMode == Gpio::OutMode::PUSH_PULL ? GPIO_MODE_INPUT_OUTPUT : GPIO_MODE_INPUT_OUTPUT_OD);
    } else if (way & Gpio::Way::OUT) {
        return (outMode == Gpio::OutMode::PUSH_PULL ? GPIO_MODE_OUTPUT : GPIO_MODE_OUTPUT_OD);
    } else if (way & Gpio::Way::IN) {
        return GPIO_MODE_INPUT;
    } else {
        assert(way == Gpio::Way::DISABLED);
        return GPIO_MODE_DISABLE;
    }
}

bool GpioImpl::config(Way way, OutMode outMode, Pull pull) {
    
    gpio_config_t iocfg = {};
    iocfg.pin_bit_mask = 1ULL << _pin;
    iocfg.mode = modeTypeToIdf(way, outMode);
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

bool GpioImpl::set(bool value) {
    esp_err_t ret = gpio_set_level(static_cast<gpio_num_t>(_pin), value ? 1 : 0);
    if (ESP_OK == ret) {
        _set_value = value;
        return true;
    } else {
        err("Fail GPIO %u set %u: %u %s", _pin, value ? 1 : 0, ret, esp_err_to_name(ret));
        return false;
    }
}

bool GpioImpl::get() {
    return gpio_get_level(static_cast<gpio_num_t>(_pin));
}

bool GpioImpl::toggle() {
    return set(!_set_value);
}

void GpioImpl::reset() {
    gpio_reset_pin(static_cast<gpio_num_t>(_pin));
}

static gpio_int_type_t intrTrigToIdf(Gpio::IntrTrig trig) {
    switch (trig) {
        case Gpio::IntrTrig::DISABLED:  return GPIO_INTR_DISABLE;
        case Gpio::IntrTrig::RISING:    return GPIO_INTR_POSEDGE;
        case Gpio::IntrTrig::FALLING:   return GPIO_INTR_NEGEDGE;
        case Gpio::IntrTrig::ANY_EDGE:  return GPIO_INTR_ANYEDGE;
        case Gpio::IntrTrig::LOW:       return GPIO_INTR_LOW_LEVEL;
        case Gpio::IntrTrig::HIGH:      return GPIO_INTR_HIGH_LEVEL;
        default:                        return GPIO_INTR_DISABLE;
    }
}

bool GpioImpl::addIsr(Isr isr, IntrTrig type) {
    if (!isr) {
        err("Fail add empty ISR");
        return false;
    }
    esp_err_t ret = gpio_set_intr_type(static_cast<gpio_num_t>(_pin), intrTrigToIdf(type));
    if (ESP_OK != ret) {
        err("Fail set intr trigger type: %u %s", ret, esp_err_to_name(ret));
        return false;
    }
    ret = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    if (ESP_OK != ret) {
        err("Fail install per-pin ISR service: %u %s", ret, esp_err_to_name(ret));
        return false;
    }
    // Wrapper to call a C++ object method from C function
    auto wrapIsr = [](void* arg) { 
        assert(arg); assert(static_cast<GpioImpl*>(arg)->_isr); 
        static_cast<GpioImpl*>(arg)->_isr();
    };
    ret = gpio_isr_handler_add(static_cast<gpio_num_t>(_pin), wrapIsr, this);
    if (ESP_OK != ret) {
        err("Fail install ISR: %u %s", ret, esp_err_to_name(ret));
        return false;
    }
    _isr = isr;
    return ESP_OK == gpio_intr_enable(static_cast<gpio_num_t>(_pin));
}

Gpio::Hnd Gpio::create(Gpio::Pin pin, Way way, OutMode outMode, Pull pull) {
    auto io = std::make_unique<GpioImpl>(pin, false);
    if (io->config(way, outMode, pull)) {
        return io;
    } else {
        return nullptr;
    }
}

} // namespace