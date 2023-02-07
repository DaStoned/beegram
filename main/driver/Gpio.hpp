/**
 * @brief Driver for GPIO
*/

#pragma once

#include <memory>
#include <cinttypes>
#include <functional>

namespace beegram {

/**
 * Abstract interface for a module that implements a single GPIO pin
*/
class Gpio {
public:
    /// @brief Identifier for physical GPIO pin
    using Pin = unsigned int;
    /// @brief Handle for a GPIO pin object
    using Hnd = std::unique_ptr<Gpio>;
    /// @brief Function type that's accepted as interrupt service routine
    using Isr = std::function<void()>;
    /// @brief Configure pin direction (can be simultaneously input and output)
    enum Way : uint8_t {
        DISABLED = 0b00, ///< Pin is not used
        IN       = 0b01, ///< Pin is used as input
        OUT      = 0b10, ///< Pin is used as output
    };
    /// @brief Choose output mode. Ignored for inputs.
    enum class OutMode {
        PUSH_PULL,      ///< Output mode is push-pull
        OPEN_DRAIN      ///< Output mode is open-drain/open-collector
    };
    /// @brief Enable pull-up or pull-down resistors (or both simultaneously)
    enum Pull : uint8_t {
        NONE = 0b00, ///< No pull-up or pull-down
        UP   = 0b01, ///< Enable pull-up resistor
        DOWN = 0b10, ///< Enable pull-down resistor
    };
    enum class IntrTrig {
        DISABLED,
        RISING,
        FALLING,
        ANY_EDGE,
        LOW,
        HIGH
    };

    /// @brief Mandatory virtual destructor
    virtual ~Gpio() = default;

    /**
     * Configure a GPIO pin
     * @param way Choose pin direction
     * @param outMode Choose output mode of pin (ignored for input-only pins)
     * @param pull Activate built-in pull-up or pull-down resistors 
     * @return True on success; false on failure
    */
    virtual bool config(Way way, OutMode outMode, Pull pull) = 0;

    /**
     * Set output value of GPIO pin
     * @param value True for high level; false for low
     * @return True on success; false on failure
    */
    virtual bool set(bool value) = 0;

    /**
     * Read input value of GPIO pin
     * @return True for high level; false for low
    */
    virtual bool get() = 0;

    /**
     * Toggle output value of GPIO pin
     * @return True on success; false on failure
    */
    virtual bool toggle() = 0;

    /**
     * Reset GPIO configuration back to its default state
    */
    virtual void reset() = 0;

    virtual bool addIsr(Isr isr, IntrTrig type) = 0;

    /**
     * Allocate and configure a new instance of a GPIO pin object.
     * @param pin Number of physical GPIO pin to configure
     * @param way Choose pin direction
     * @param outMode Choose output mode of pin (ignored for input-only pins)
     * @param pull Activate built-in pull-up or pull-down resistors
     * @return True on success; false on failure
    */
    static Hnd create(Pin pin, Way way, OutMode outMode = OutMode::PUSH_PULL, Pull pull = Pull::NONE);
};

} // namespace