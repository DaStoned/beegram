#include "Scales.hpp"
#include "Log.hpp"
#include "Param.hpp"
#include "Bosun.hpp"
#include "driver/Hx711.hpp"

#include <string>

using namespace std;

namespace beegram {

class ScalesImpl : public Scales {
public:
    ScalesImpl(Param& param, Bosun& bosun, Hx711& loadSensor)
    : _param(param), _bosun(bosun), _loadSensor(loadSensor)
    {}
    virtual bool init() override;
    virtual bool tare() override;
    virtual float weigh() override;
private:
    static constexpr const char* PKEY_CALIB_WEIGHT_LOW = "scacall_weight";
    static constexpr const char* PKEY_CALIB_LOAD_LOW = "scacall_load";
    static constexpr const char* PKEY_CALIB_WEIGHT_HIGH = "scacalh_weight";
    static constexpr const char* PKEY_CALIB_LOAD_HIGH = "scacalh_load";
    static constexpr float MIN_CALIB_WEIGHT = 0.0;
    static constexpr float MAX_CALIB_WEIGHT = 400.0;

    bool calib(float weight, const char* pkeyLoad, const char* pkeyWeigth);

    Param& _param;
    Bosun& _bosun;
    Hx711& _loadSensor;
    int _tare = 0;
};

bool ScalesImpl::calib(float weight, const char* pkeyLoad, const char* pkeyWeigth) {
    if (weight < MIN_CALIB_WEIGHT || weight > MAX_CALIB_WEIGHT) {
        err("Invalid weight [%f, %f]: %f\n", MIN_CALIB_WEIGHT, MAX_CALIB_WEIGHT, weight);
        return false;
    }
    const int load = _loadSensor.read();
    info("Scales calib weight=%f load=%d", weight, load);
    if (_param.setFloat(pkeyLoad, weight) && _param.setFloat(pkeyWeigth, load)) {
        info("Calib saved");
        return true;
    } else {
        err("Failed to save calib\n");
        return false;
    }
}

bool ScalesImpl::init() {
    _bosun.addCmd(
        "scacall", Cmd(
            "weight\n\tCalibrate scales' low point if loaded with weight in kg",
            [this](const vector<string>& args) {
                fflush(stdout);
                if (2 != args.size()) {
                    err("Need weight low\n");
                    return;
                }
                calib(stof(args[1]), PKEY_CALIB_WEIGHT_LOW, PKEY_CALIB_LOAD_LOW);
            }
        )
    );
    _bosun.addCmd(
        "scacalh", Cmd(
            "weight\n\tCalibrate scales' high point if loaded with weight in kg",
            [this](const vector<string>& args) {
                fflush(stdout);
                if (2 != args.size()) {
                    err("Need weight high\n");
                    return;
                }
                calib(stof(args[1]), PKEY_CALIB_WEIGHT_HIGH, PKEY_CALIB_LOAD_HIGH);
            }
        )
    );
    return true;
}

bool ScalesImpl::tare() {
    _tare = _loadSensor.read();
    return true;
}

float ScalesImpl::weigh() {
    const float x1 = _param.getFloat(PKEY_CALIB_LOAD_LOW, -207124.0F).value();
    const float x2 = _param.getFloat(PKEY_CALIB_LOAD_HIGH, -593571.0F).value();
    const float y1 = _param.getFloat(PKEY_CALIB_WEIGHT_LOW, 0.0F).value();
    const float y2 = _param.getFloat(PKEY_CALIB_WEIGHT_HIGH, 32.0F).value();
    const float rise = (y2 - y1)/(x2 - x1);
    const float offset = y1 - (x1 * rise);
    const float weight = _loadSensor.read() * rise + offset;
    return weight; // TODO: add tare offset
}

Scales::Hnd Scales::create(Param& param, Bosun& bosun, Hx711& loadSensor) {
    return make_unique<ScalesImpl>(param, bosun, loadSensor);
}

} // namespace
