#define LAST_STATE_KEY 2
#include <pebble_worker.h>

static BatteryChargeState s_prev_charge_state;

static void handle_battery(BatteryChargeState charge_state) {
  if (persist_exists(LAST_STATE_KEY)) {
    // Read settings from persistent storage
    persist_read_data(LAST_STATE_KEY, &s_prev_charge_state, sizeof(s_prev_charge_state));

    if(s_prev_charge_state.charge_percent != charge_state.charge_percent || s_prev_charge_state.is_charging != charge_state.is_charging || s_prev_charge_state.is_plugged != charge_state.is_plugged) { 
      // write changes
      s_prev_charge_state = charge_state;
      persist_write_data(LAST_STATE_KEY, &,s_prev_charge_state sizeof(s_prev_charge_state));
      
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery change event!");
      worker_launch_app(); // call the main app
    }
  } else {
    // write changes
    s_prev_charge_state = charge_state;
    persist_write_data(LAST_STATE_KEY, &,s_prev_charge_state sizeof(s_prev_charge_state));

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery change event!");
    worker_launch_app(); // call the main app
  }
}

static void prv_init() {
  s_prev_charge_state = battery_state_service_peek(); // get default value
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