#include "esp_system.h"
#include "Log.hpp"
#include "App.hpp"

static beegram::App app;

extern "C" {

void app_main(void) {
    debug("Starting App");
    app.run();
    warn("Exited App");
    fflush(stdout);
    esp_restart();
}

} // extern "C" {
