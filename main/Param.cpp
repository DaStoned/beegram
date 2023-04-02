#include "Param.hpp"
#include "Log.hpp"

#include "nvs_flash.h"

using namespace std;

namespace beegram {

class ParamImpl : public Param {
public:
    ParamImpl(nvs_handle_t nvs)
    : _nvs(nvs)
    {}
    virtual optional<int32_t> getI32(const char* key) override;
    virtual optional<uint32_t> getU32(const char* key) override;
    virtual optional<float> getFloat(const char* key) override;
    virtual bool setI32(const char* key, int32_t val) override;
    virtual bool setU32(const char* key, uint32_t val) override;
    virtual bool setFloat(const char* key, float val) override;
private:
    nvs_handle_t _nvs;
};

optional<int32_t> ParamImpl::getI32(const char* key) {
    int32_t val;
    if (ESP_OK == nvs_get_i32(_nvs, key, &val)) {
        return val;
    } else {
        return nullopt;
    }
}

optional<uint32_t> ParamImpl::getU32(const char* key) {
    uint32_t val;
    if (ESP_OK == nvs_get_u32(_nvs, key, &val)) {
        return val;
    } else {
        return nullopt;
    }
}

optional<float> ParamImpl::getFloat(const char* key) {
    // NVS doesn't support floats, so we use uint32 as storage
    uint32_t val;
    if (ESP_OK == nvs_get_u32(_nvs, key, &val)) {
        return static_cast<float>(val);
    } else {
        return nullopt;
    }
}

bool ParamImpl::setI32(const char* key, int32_t val) {
    return ESP_OK == nvs_set_i32(_nvs, key, val);
}

bool ParamImpl::setU32(const char* key, uint32_t val) {
    return ESP_OK == nvs_set_u32(_nvs, key, val);
}

bool ParamImpl::setFloat(const char* key, float val) {
    return setU32(key, *reinterpret_cast<uint32_t*>(&val));
}

Param::Hnd Param::create(const char* part, const char* ns) {
    esp_err_t ret = nvs_flash_init_partition(part);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err("Fail init part [%s]: %s %d, erasing!", part, esp_err_to_name(ret), ret);
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ret = nvs_flash_erase_partition(part);
        if (ESP_OK != ret) {
            err("Fail erase part [%s]: %s %d", part, esp_err_to_name(ret), ret);
            return nullptr;
        }
        ret = nvs_flash_init_partition(part);
        if (ESP_OK != ret) {
            err("Fail re-init part [%s]: %s %d", part, esp_err_to_name(ret), ret);
            return nullptr;
        }
    }
    nvs_handle_t nvs;
    ret = nvs_open_from_partition(part, ns, NVS_READWRITE, &nvs);
    if (ESP_OK != ret) {
        err("Fail open part [%s] namespace [%s]: %s %d", part, ns, esp_err_to_name(ret), ret);
        return nullptr;
    }
    Param::Hnd param = make_unique<ParamImpl>(nvs);
    assert(param);
    return param;
}

} // namespace