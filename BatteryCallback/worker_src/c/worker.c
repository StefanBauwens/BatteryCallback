#include <pebble_worker.h>

static void handle_battery(BatteryChargeState charge_state) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery change event!");
  worker_launch_app(); // call the main app
}

static void prv_init() {
  battery_state_service_subscribe(handle_battery);
}

static void prv_deinit() {
  battery_state_service_unsubscribe();
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}