/**
 * @brief Wrapper for system interrupt
*/

#pragma once

namespace beegram {

#include <memory>
#include <cinttypes>
#include <functional>

class Interrupt {
public:
    /// @brief Function type that's accepted as interrupt service routine
    using Isr = std::function<void()>;
    using Hnd = std::shared_ptr<Interrupt>;
    virtual ~Interrupt() = default;
    virtual bool attach(const Isr& isr) = 0;
    virtual bool enable() = 0;
    virtual bool disable() = 0;
};

} // namespace
