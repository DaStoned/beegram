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
    static constexpr const char* PKEY_TARE_LOAD = "scale_tare";
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
    if (_param.setFloat(pkeyLoad, weight) && _param.setI32(pkeyWeigth, load)) {
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
    _bosun.addCmd(
        "scatare", Cmd(
            "\n\tTare scales to 0 kg",
            [this](const vector<string>& args) {
                fflush(stdout);
                const int load = _loadSensor.read();
                info("Scales tare %d", load);
                _param.setI32(PKEY_TARE_LOAD, load);
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
    // Linear relation between load and weight is y = A * x + B. We need to
    // find the values of A and B. Assuming a calibration with two known
    // points, i.e. (load, weight) values (x_1, y_1) and (x_2, y_2) we can
    // find A = (y_2 - y_1)/(x_2 - x_1) and B = y_1 - (x_1 * A)
    const int x1 = _param.getI32(PKEY_CALIB_LOAD_LOW).value_or(-207124);
    const float y1 = _param.getFloat(PKEY_CALIB_WEIGHT_LOW).value_or(0.0F);
    const int x2 = _param.getI32(PKEY_CALIB_LOAD_HIGH).value_or(-593571);
    const float y2 = _param.getFloat(PKEY_CALIB_WEIGHT_HIGH).value_or(32.0F);
    const float a = (y2 - y1)/(x2 - x1);
    const float b = y1 - (x1 * a);
    const auto tare = _param.getI32(PKEY_TARE_LOAD);
    if (tare.has_value()) {
        // If we've set a load tare value x_t (value of x where y must be 
        // equal to 0), we need to first find the value of x when y_0 == 0:
        // x_0 = (y_0 - B) / A = -B / A. Now tared y_t can be found with:
        // y_t = A * (x - (x_t - x_0)) + B
        const float x0 = -b / a;
        return a * (_loadSensor.read() - (tare.value() - x0)) + b;
    } else {
        // No tare, simple
        return a * _loadSensor.read() + b;
    }
}

Scales::Hnd Scales::create(Param& param, Bosun& bosun, Hx711& loadSensor) {
    return make_unique<ScalesImpl>(param, bosun, loadSensor);
}

} // namespace
