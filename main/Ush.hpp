/**
 * @brief Micro command shell
*/

#pragma once

#include <memory>

namespace beegram {

class Ush {
public:
    using Hnd = std::unique_ptr<Ush>;
    virtual bool start(unsigned int uart) = 0;
    static Hnd create();
};

} // namespace
