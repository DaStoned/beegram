/**
 * @brief Interface for key-value storage of parameters
*/

#pragma once

#include <cinttypes>
#include <memory>
#include <optional>

namespace beegram {

class Param {
public:
    using Hnd = std::unique_ptr<Param>;
    virtual ~Param() = default;
    virtual std::optional<int32_t> getI32(const char* key, std::optional<int32_t> fb = std::nullopt) = 0;
    virtual std::optional<uint32_t> getU32(const char* key, std::optional<uint32_t> fb = std::nullopt) = 0;
    virtual std::optional<float> getFloat(const char* key, std::optional<float> fb = std::nullopt) = 0;
    virtual bool setI32(const char* key, int32_t val) = 0;
    virtual bool setU32(const char* key, uint32_t val) = 0;
    virtual bool setFloat(const char* key, float val) = 0;
    static Hnd create(const char* part, const char* ns);
};

} // namespace
