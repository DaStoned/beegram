/**
 * @brief Driver for the Avia Hx711 ADC (for bridge connected load sensors)
*/

#pragma once

#include <memory>
#include <cinttypes>

namespace beegram {

/**
 * Abstract interface for the Hx711 load sensor 
*/
class Hx711 {
public:
    /// @brief Combination of ADC channel and gain is set by total number of pulses
    enum Mode : uint8_t {
        NONE        = 0,    ///< Mode not set
        CH_A_GN128  = 25,   ///< Channel A, gain 128
        CH_B_GN32   = 26,   ///< Channel B, gain 32
        CH_A_GN64   = 27,   ///< Channel A, gain 64
    };
    virtual ~Hx711() = default;

    /**
     * Initialize the ADC. Required before reading samples.
     * @param pinDout GPIO pin number where DOUT is connected
     * @param pinSck GPIO pin number where SCK is connected
     * @param mode Conversion mode
     * @return True if succeeded; false otherwise
    */
    virtual bool init(unsigned int pinDout, unsigned int pinSck, Mode mode) = 0;

    /**
     * @return True if ACD sample is ready for reading; false otherwise
    */
    virtual bool isReady() = 0;

    /**
     * Read an ADC sample
     * @return Raw ADC sample
    */
    virtual int read() = 0;

    /**
     * Create a singleton instance of the Hx711 driver
    */
    static std::unique_ptr<Hx711> create();
};

} // namespace