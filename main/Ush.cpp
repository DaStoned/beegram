#include "Ush.hpp"
#include "Log.hpp"
#include "Bosun.hpp"

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <cinttypes>
#include <span>
#include <deque>
#include <vector>
#include <cctype>

using namespace std;

namespace beegram {

class UshImpl : public Ush {
public:
    UshImpl(Bosun& bosun)
    : _bosun(bosun)
    {}
    virtual bool start(uart_port_t uart) override;
private:
    static constexpr size_t RX_BUF_LEN_B = 256;
    static constexpr size_t TX_BUF_LEN_B = 256;
    static constexpr size_t EV_QUEUE_LEN = 10;
    static constexpr size_t USH_STACK_LEN_B = 4 * 1024;
    static constexpr size_t USH_STACK_PRIO = tskIDLE_PRIORITY;
    static constexpr size_t LINE_BUF_MAX_LEN = 128;

    void onData(const span<const uint8_t>& data);
    void run();

    uart_port_t _uart = UART_NUM_0;
    QueueHandle_t _evq = nullptr;
    deque<char> _line;
    Bosun& _bosun;
};

bool UshImpl::start(uart_port_t uart) {
    _uart = uart;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {
            .allow_pd = 1,
            .backup_before_sleep = 1
        },
    };
    esp_err_t ret = uart_param_config(_uart, &uart_config);
    if (ESP_OK != ret) {
        err("Fail UART config: %s", esp_err_to_name(ret));
        return false;
    }
    ret = uart_set_pin(_uart, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ESP_OK != ret) {
        err("Fail set UART pins: %s", esp_err_to_name(ret));
        return false;
    }
    ret = uart_driver_install(_uart, RX_BUF_LEN_B, TX_BUF_LEN_B, EV_QUEUE_LEN, &_evq, 0);
    if (ESP_OK != ret) {
        err("Fail install UART driver: %s", esp_err_to_name(ret));
        return false;
    }
    auto runTask = [](void* arg) {
        assert(arg); static_cast<UshImpl*>(arg)->run();
    };
    BaseType_t taskRet = xTaskCreate(runTask, "ush", USH_STACK_LEN_B, this, USH_STACK_PRIO, nullptr);
    if (pdPASS != taskRet) {
        err("Fail create task: %d", taskRet);
        return false;
    }
    return true;
}

deque<char>& trimSpace(deque<char>& q) {
    while (q.size() > 0 && isspace(q.front())) {
        q.pop_front();
    }
    while (q.size() > 0 && isspace(q.back())) {
        q.pop_back();
    }
    return q;
}

static vector<string> splitWords(deque<char>& line) {
    vector<string> words;
    while (line.size()) {
        while (line.size() && isspace(line.front())) {
            line.pop_front();
        }
        string word;
        while (line.size() && !isspace(line.front())) {
            word.append({line.front()});
            line.pop_front();
        }
        if (word.size()) {
            words.emplace_back(word);
        }
    }
    return words;
}

void UshImpl::onData(const span<const uint8_t>& data) {
    trace_dump(data.data(), data.size_bytes());
    for (uint8_t chr: data) {
        if (chr == '\n' || chr == '\r') {
            debug("Parse line [%s]", string(_line.begin(), _line.end()).c_str());
            auto words = splitWords(trimSpace(_line));
            for (auto& w : words) {
                info("Found: %s", w.c_str());
            }
            if (words.size()) {
                _bosun.runCmd(words);
            }
            _line.clear();
        } else if (_line.size() >= LINE_BUF_MAX_LEN) {
            warn("Line buffer full (%u B)", _line.size());
        } else if (isprint(chr)) {
            _line.emplace_back(chr);
        } else {
            warn("Ignore char 0x%02X", chr);
        }
    }
}

void UshImpl::run() {
    uart_event_t ev;
    uint8_t buf[RX_BUF_LEN_B];
    while (true) {
        if (pdTRUE != xQueueReceive(_evq, &ev, portMAX_DELAY)) {
            warn("Queue timeout");
            continue;
        }
        switch (ev.type) {
        case UART_DATA: {
            int read;
            trace("UART_DATA len=%u%s", ev.size, ev.timeout_flag ? " RX TOUT" : "");
            // Suck the RX buffer dry
            do {
                read = uart_read_bytes(_uart, buf, sizeof(buf), 0);
                if (read > 0) {
                    uart_write_bytes(_uart, buf, read); // Echo back
                    onData(span{buf, static_cast<size_t>(read)});
                } else if (read < 0) {
                    warn("UART read error: %d", read);
                }
            } while (read > 0);
            break;
        }
        case UART_BREAK:
            debug("UART_BREAK");
            break;
        case UART_BUFFER_FULL:
            debug("UART_BUFFER_FULL");
            break;
        case UART_FIFO_OVF:
            debug("UART_FIFO_OVF");
            break;
        case UART_FRAME_ERR:
            debug("UART_FRAME_ERR");
            break;
        case UART_PARITY_ERR:
            debug("UART_PARITY_ERR");
            break;
        case UART_DATA_BREAK:
            debug("UART_DATA_BREAK");
            break;
        case UART_PATTERN_DET:
            debug("UART_PATTERN_DET");
            break;
        default:
            warn("Unknown UART event type=%u len=%u", static_cast<unsigned int>(ev.type), ev.size);
        }
    }
}

unique_ptr<Ush> Ush::create(Bosun& bosun) {
    return make_unique<UshImpl>(bosun);
}


} // namespace
