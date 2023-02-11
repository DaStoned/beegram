#include "Hx711.hpp"
#include "Gpio.hpp"
#include "Log.hpp"

namespace beegram {

class Hx711 : public LoadSensor {
public:
    Hx711() = default;
    virtual bool init(unsigned int pinDout, unsigned int pinSck, ChanMode mode) override;
    virtual bool isReady() override;
    virtual int read() override;
private:
    Gpio::Hnd _dout = nullptr;
    Gpio::Hnd _sck = nullptr;
    ChanMode _mode = ChanMode::NONE;
};

bool Hx711::init(unsigned int pinDout, unsigned int pinSck, ChanMode mode) {
    _dout = Gpio::create(pinDout, Gpio::Way::IN, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    _sck = Gpio::create(pinSck, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    _mode = mode;
    return (_dout && _sck && ChanMode::NONE != _mode);
}

bool Hx711::isReady() {
    assert(_dout);
    return !_dout->get();
}

int Hx711::read() {
    assert(_dout && _sck && ChanMode::NONE != _mode);
    if (!isReady()) {
        err("Not ready for reading");
        return 0;
    }
    int value = 0;
    _sck->set(false);
    for (int i = 0; i < _mode; i++) {
        esp_rom_delay_us(1);
        _sck->set(true);
        esp_rom_delay_us(1);
        if (i < 24) {
            value = (value << 1) | (_dout->get() ? 1 : 0);
        }
        _sck->set(false);
    }
    return value;
}

std::unique_ptr<LoadSensor> LoadSensor::create() {
    return std::make_unique<Hx711>();
}

} // namespace