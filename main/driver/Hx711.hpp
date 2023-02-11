/**
 * @brief Driver for HX711 ADC for weigh scales
*/

#pragma once

#include <memory>
#include <cinttypes>

namespace beegram {

class LoadSensor {
public:
    enum ChanMode : uint8_t {
        NONE        = 0,
        CH_A_GN128  = 25,
        CH_B_GN32   = 26,
        CH_A_GN64   = 27,
    };
    virtual ~LoadSensor() = default;
    virtual bool init(unsigned int pinDout, unsigned int pinSck, ChanMode mode) = 0;
    virtual bool isReady() = 0;
    virtual int read() = 0;
    static std::unique_ptr<LoadSensor> create();
};

} // namespace