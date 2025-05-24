/**
 * @brief Micro command shell
*/

#pragma once

#include "driver/uart.h"
#include <memory>

namespace beegram {

class Bosun;

class Ush {
public:
    using Hnd = std::unique_ptr<Ush>;
    virtual bool start(uart_port_t uart) = 0;
    static Hnd create(Bosun& bosun);
};

} // namespace
