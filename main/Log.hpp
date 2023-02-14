/**
@brief Trace logging
 */

#ifndef _LOG_HPP_
#define _LOG_HPP_

#include "esp_log.h"
#include <string_view>

// Note: path processing naïvely assumes a valid Unix file path containing
// directories and an extension.

/**
 * Find the length of stem in a file name
 * @param path A file name with '/' as path separator and '.' as extension separator
 * @return Number of characters in file stem excluding terminating zero
 */
constexpr size_t stemNameLen(const std::string_view& path) {
    return path.find_last_of('.') - path.find_last_of('/') - 1;
}

// Rudimentary unit test
static_assert(stemNameLen(std::string_view("../foo/bar/MyModule.cpp")) == 8);

/**
 * Get the stem in a file name
 * @param path A file name with '/' as path separator and '.' as extension separator
 * @return A string_view holding the stem of the input file name
 */
constexpr std::string_view stemName(const std::string_view& path) {
    return path.substr(path.find_last_of('/') + 1, stemNameLen(path));
}

// Rudimentary unit test
static_assert(stemName(std::string_view("../foo/bar/MyModule.cpp")) == "MyModule");

/// Helper class for creating a C-style zero-terminated string from a string_view
template <size_t N>
class TerminatedString {
public:
    constexpr TerminatedString(const std::string_view& path) {
        size_t i = 0;
        for (auto it = path.cbegin(); i + 1 < sizeof(_str) && it != path.cend(); i++, it++) {
            _str[i] = *it;
        }
    }
    constexpr const char *str() const {
        return _str;
    }
private:
    char _str[N] {'\0', };
};

/// Module name from the file which includes this header
static constexpr std::string_view moduleName = stemName(__BASE_FILE__);
/// A zero-terminated log prefix from module name, initialized compile-time
static constexpr TerminatedString<moduleName.length() + 1> logPrefix{moduleName};

// Sanity check, assumes all file stems in project are less than 100 chars
static_assert(moduleName.length() < 100);

#define err(args...) ESP_LOGE(logPrefix.str(), args)
#define warn(args...) ESP_LOGW(logPrefix.str(), args)
#define info(args...) ESP_LOGI(logPrefix.str(), args)
#define debug(args...) ESP_LOGD(logPrefix.str(), args)
#define trace(args...) ESP_LOGV(logPrefix.str(), args)

#define err_dump(buf, len) ESP_LOG_BUFFER_HEXDUMP(logPrefix.str(), buf, len, ESP_LOG_ERROR)
#define warn_dump(buf, len) ESP_LOG_BUFFER_HEXDUMP(logPrefix.str(), buf, len, ESP_LOG_WARN)
#define info_dump(buf, len) ESP_LOG_BUFFER_HEXDUMP(logPrefix.str(), buf, len, ESP_LOG_INFO)
#define debug_dump(buf, len) ESP_LOG_BUFFER_HEXDUMP(logPrefix.str(), buf, len, ESP_LOG_DEBUG)
#define trace_dump(buf, len) ESP_LOG_BUFFER_HEXDUMP(logPrefix.str(), buf, len, ESP_LOG_VERBOSE)

#endif // _LOG_HPP_
