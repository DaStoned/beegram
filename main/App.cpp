#include "App.hpp"
#include "Log.hpp"
#include "Cloud.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace beegram {

Cloud cloud;

void App::run() {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        cloud.cloudStuff();
    }
}

} // namespace beegram
