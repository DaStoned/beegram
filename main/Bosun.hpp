/**
 * @brief Executes command line commands
*/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace beegram {

class Cmd {
public:
    using Command = std::function<void(const std::vector<std::string>&)>;
    Cmd(const std::string_view& name, const std::string_view& help, const Command& cmd)
    : _name(name), _help(help), _command(cmd)
    {}
    void run(const std::vector<std::string>& words) const { _command(words); }
    const std::string_view& getName() const { return _name; };
    const std::string_view& getHelp() const { return _help; };
private:
    const std::string_view _name;
    const std::string_view _help;
    Command _command;
};

class Bosun {
public:
    using Hnd = std::unique_ptr<Bosun>;
    virtual void addCmd(const Cmd& cmd) = 0;
    virtual void runCmd(const std::vector<std::string>& words) const = 0;
    virtual bool init() = 0;
    static Hnd create();
};

} // namespace
