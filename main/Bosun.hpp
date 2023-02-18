/**
 * @brief Executes command line commands
*/

#pragma once

#include <vector>
#include <string>
#include <memory>

namespace beegram {

class Bosun {
public:
    using Hnd = std::unique_ptr<Bosun>;
    virtual void parseRun(const std::vector<std::string>& words) = 0;
    static Hnd create();
};

} // namespace
