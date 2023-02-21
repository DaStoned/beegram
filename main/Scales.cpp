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
private:
    static constexpr const char* PKEY_CALIB_WEIGHT_LOW = "scacall_weight";
    static constexpr const char* PKEY_CALIB_LOAD_LOW = "scacall_load";
    static constexpr const char* PKEY_CALIB_WEIGHT_HIGH = "scacalh_weight";
    static constexpr const char* PKEY_CALIB_LOAD_HIGH = "scacalh_load";
    static constexpr float MIN_CALIB_WEIGHT = 0.0;
    static constexpr float MAX_CALIB_WEIGHT = 400.0;
    Param& _param;
    Bosun& _bosun;
    Hx711& _loadSensor;
};

bool ScalesImpl::init() {
    _bosun.addCmd(
        "scacall", Cmd(
            "weight\n\tCalibrate scales' lower point if loaded with weight in kg",
            [this](const vector<string>& args) {
                fflush(stdout);
                if (2 != args.size()) {
                    err("Need weight low\n");
                    return;
                }
                const float weight = stof(args[1]);
                if (weight < MIN_CALIB_WEIGHT || weight > MAX_CALIB_WEIGHT) {
                    err("Invalid weight low [%f, %f]: %f\n", MIN_CALIB_WEIGHT, MAX_CALIB_WEIGHT, weight);
                    return;
                }
                const int load = _loadSensor.read();
                info("Scales calib low weight=%f load=%d", weight, load);
                if (_param.setFloat(PKEY_CALIB_WEIGHT_LOW, weight) && _param.setFloat(PKEY_CALIB_LOAD_LOW, load)) {
                    info("Calib low saved");
                } else {
                    err("Failed to save calib low\n");
                    return;
                }
            }
        )
    );
    return true;
}

Scales::Hnd Scales::create(Param& param, Bosun& bosun, Hx711& loadSensor) {
    return make_unique<ScalesImpl>(param, bosun, loadSensor);
}

} // namespace
