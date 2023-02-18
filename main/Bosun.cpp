#include "Bosun.hpp"
#include "Log.hpp"

#include <functional>
#include <map>

using namespace std;

namespace beegram {

class Cmd {
public:
    using Command = function<void(const vector<string>&)>;
    Cmd(const char* name, const char* help, Command cmd)
    : _name(name)
    , _help(help)
    , _command(cmd)
    {}
    virtual void run(const vector<string>& words);
    string get_name() const { return _name; };
private:
    const string _name;
    const string _help;
    Command _command;
};

void Cmd::run(const vector<string>& words) {
    _command(words);
}

class BosunImpl : public Bosun {
public:
    virtual void parseRun(const vector<string>& words) override;
    void add(Cmd* cmd);
private:
    map<string, Cmd*> _cmds;
};

void BosunImpl::parseRun(const vector<string>& words) {
    if (words.size() <= 0) {
        err("No command words");
        return;
    }
    if (_cmds.find(words[0]) == _cmds.end()) {
        err("Unknown command [%s]", words[0].c_str());
        return;
    }
    _cmds[words[0]]->run(words);
}

void BosunImpl::add(Cmd* cmd) {
    _cmds.emplace(cmd->get_name(), cmd);
}

Bosun::Hnd Bosun::create() {
    auto bosun = make_unique<BosunImpl>();
    auto cmd = new Cmd("help", "Print command help", [](const vector<string>&) { printf("Trying to help\n"); });
    assert(cmd);
    bosun->add(cmd);
    return bosun;
}

} // namespace
