/**
 * @brief Scales for weighing
*/

#pragma once

#include <memory>

namespace beegram {

class Param; class Bosun; class Hx711;

class Scales {
public:
    using Hnd = std::unique_ptr<Scales>;
    virtual bool init() = 0;
    virtual bool tare() = 0;
    virtual float weigh() = 0;
    static Hnd create(Param& param, Bosun& bosun, Hx711& loadSensor);
};

} // namespace
