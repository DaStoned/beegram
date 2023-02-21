#include "Bosun.hpp"
#include "Log.hpp"

#include <map>

using namespace std;

namespace beegram {

class BosunImpl : public Bosun {
public:
    virtual void addCmd(const std::string_view& name, const Cmd& cmd) override;
    virtual void runCmd(const vector<string>& words) const override;
    virtual bool init() override;
private:
    map<string, Cmd> _cmds;
};

void BosunImpl::addCmd(const std::string_view& name, const Cmd& cmd) {
    _cmds.emplace(name, cmd);
}

void BosunImpl::runCmd(const vector<string>& words) const {
    if (words.size() <= 0) {
        err("No command words");
        return;
    }
    if (_cmds.find(words[0]) == _cmds.end()) {
        err("Unknown command [%s]", words[0].c_str());
        return;
    }
    _cmds.at(words[0]).run(words);
}

bool BosunImpl::init() {
    addCmd(
        "help", Cmd(
            "Print all commands and their help messages", 
            [this](const vector<string>& args) { 
                for (auto& [key, val]: _cmds) {
                    printf("%-8s %s\n", key.c_str(), val.getHelp().data());
                }
            }
        )
    );
    return true;
}

Bosun::Hnd Bosun::create() {
    return make_unique<BosunImpl>();
}

} // namespace
