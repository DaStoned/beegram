/**
 * @brief Driver for GPIO
*/

#pragma once

#include <memory>
#include <cinttypes>

namespace beegram {

class IGpio {
public:
    using Pin = unsigned int;
    using Hnd = std::unique_ptr<IGpio>;
    /// @brief Configure a pin direction as input or output (or both)
    enum Way : uint8_t {
        DISABLED = 0b00,
        IN       = 0b01,
        OUT      = 0b10,
    };
    /// @brief Choose output mode as push-pull or open drain. Ignored for inputs.
    enum class Mode {
        PUSH_PULL,
        OPEN_DRAIN
    };
    /// @brief Enable pull-up or pull-down resistors (or both)
    enum Pull : uint8_t {
        NONE = 0b00, ///< No pull-up or pull-down
        UP   = 0b01, ///< Enable pull-up resistor
        DOWN = 0b10, ///< Enable pull-down resistor
    };

    virtual ~IGpio() = default;

    virtual bool config(Way way, Mode mode, Pull pull) = 0;
    virtual bool set(bool value) = 0;
    virtual bool get() = 0;
    virtual bool toggle() = 0;

    static Hnd create(Pin pin, Way way, Mode mode = Mode::PUSH_PULL, Pull pull = Pull::NONE);
};

} // namespace