#include "Hx711.hpp"
#include "Gpio.hpp"
#include "Log.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace beegram {

/**
 * Implementation of the Hx711 driver interface
*/
class Hx711Impl : public Hx711 {
public:
    enum Events : uint32_t {
        SAMPLE_READY = 1 << 0,
    };
    static constexpr size_t TASK_STACK_LEN_B = 4 * 1024;
    static constexpr unsigned TASK_PRIORITY = tskIDLE_PRIORITY + 2;

    Hx711Impl() = default;
    virtual bool init(unsigned int pinDout, unsigned int pinSck, Mode mode) override;
    virtual bool isReady() override;
    virtual int read() override;
private:
    void run();
    bool sample(int* sampleOut);
    Gpio::Hnd _dout = nullptr;
    Gpio::Hnd _sck = nullptr;
    Mode _mode = Mode::NONE;
    EventGroupHandle_t _evGroup = nullptr;
    Interrupt::Hnd _intr = nullptr;
    int _lastSample = 0;
};

bool Hx711Impl::init(unsigned int pinDout, unsigned int pinSck, Mode mode) {
    _dout = Gpio::create(pinDout, Gpio::Way::IN, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    _sck = Gpio::create(pinSck, Gpio::Way::OUT, Gpio::OutMode::PUSH_PULL, Gpio::Pull::NONE);
    _mode = mode;
    _evGroup = xEventGroupCreate();
    if (!_dout || !_sck || Mode::NONE == _mode || !_evGroup) {
        return false;
    }
    // Create worker thread
    auto runTask = [](void* arg) {
        assert(arg); static_cast<Hx711Impl*>(arg)->run();
    };
    BaseType_t ret = xTaskCreate(runTask, "Hx711", TASK_STACK_LEN_B, this, TASK_PRIORITY, nullptr);
    if (pdPASS != ret) {
        return false;
    }
    // Set up interrupt on sample ready
    auto onReady = [this]() {
        BaseType_t highTask = 0;
        BaseType_t ret = xEventGroupSetBitsFromISR(_evGroup, Hx711Impl::SAMPLE_READY, &highTask);
        if (pdFAIL != ret) {
            portYIELD_FROM_ISR(highTask);
        }
    };
    _intr = _dout->addIsr(onReady, Gpio::IntrTrig::FALLING);
    if (!_intr) {
        err("Fail attach ISR");
        return false;
    }
    return _intr->enable();
}

bool Hx711Impl::isReady() {
    assert(_dout);
    return !_dout->get();
}

int Hx711Impl::read() {
    return _lastSample;
}

bool Hx711Impl::sample(int* sampleOut) {
    if (!isReady()) {
        return false;
    }
    *sampleOut = 0;
    _sck->set(false);
    for (int i = 0; i < _mode; i++) {
        esp_rom_delay_us(1);
        _sck->set(true);
        esp_rom_delay_us(1);
        const bool bitValue = _dout->get();
        _sck->set(false);
        if (i < 24) {
            *sampleOut = (*sampleOut << 1) | (bitValue ? 1 : 0);
        }
    }
    // Extend 2-s complement negative prefix from 24 to 32 bits
    if (*sampleOut & 0x00800000) {
        *sampleOut |= 0xFF000000;
    }
    return true;
}

void Hx711Impl::run() {
    EventBits_t evts;
    while (true) {
        evts = xEventGroupWaitBits(_evGroup, SAMPLE_READY, pdTRUE, pdFALSE, portMAX_DELAY);
        if (evts & SAMPLE_READY) {
            bool ret = _intr->disable();
            assert(ret);
            ret = sample(&_lastSample);
            if (!ret) {
                err("Fail sample ADC");
            }
            ret = _intr->enable();
            assert(ret);
            info("%d", _lastSample);
        }
    }
}

std::unique_ptr<Hx711> Hx711::create() {
    return std::make_unique<Hx711Impl>();
}

} // namespace