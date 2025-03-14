#define SETTINGS_KEY 1

#define REQUEST_STATE_NONE 0
#define REQUEST_STATE_WAIT_FOR_JS_READY 1
#define REQUEST_STATE_SEND_BATTERY_LEVEL 2
#define REQUEST_STATE_SEND_SUCCESSFUL 3
#define REQUEST_STATE_RECEIVED_BAD_RESPONSE 4

#include <pebble.h>

static void send_battery_charge_state_with_timeout();
static void timeout_timer_handler(void *context);

typedef struct ClaySettings {
  bool SendWhenBatteryChanged;
} ClaySettings;

static ClaySettings settings;
static bool s_js_ready;
static bool s_config_open;
static bool s_config_set;
static int8_t s_request_status;
static BatteryChargeState s_charge_state;
static AppTimer *s_timeout_timer;

static Window *s_window;
static TextLayer *s_text_layer;

// Initialize the default settings
static void prv_default_settings() {
  settings.SendWhenBatteryChanged = false;
}

static bool is_js_ready() {
  return s_js_ready;
}

static bool is_config_open() {
  return s_config_open;
}

static void quit_self() {
  window_stack_pop_all(true);
}

static void delayed_quit_self(void *context) {
  quit_self();
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();

  // check if config set
  s_config_set = persist_exists(SETTINGS_KEY);

  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  if (!s_config_set) {
    s_config_set = true;
    text_layer_set_text(s_text_layer, "Config set! You can close this now.");
  }
}

bool parse_time(const char *time_str, int8_t *hours, int8_t *minutes) {
    if (time_str == NULL || strlen(time_str) != 5) {
        return false;  // Invalid time string (must be in HH:MM format)
    }
    
    // Parse the hours
    *hours = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    // Parse the minutes
    *minutes = (time_str[3] - '0') * 10 + (time_str[4] - '0');
    
    // Validate hours and minutes
    if (*hours < 0 || *hours > 23 || *minutes < 0 || *minutes > 59) {
        return false;  // Invalid time range
    }

    return true;
}

static void timeout_timer_handler(void *context) {
  // The timer elapsed because no success was reported
  // Retry the message
  send_battery_charge_state_with_timeout();
}

static void send_battery_charge_state_with_timeout() {
  // Construct and send the message
  DictionaryIterator *iter;
  if(app_message_outbox_begin(&iter) == APP_MSG_OK) {
    int charge_percent = s_charge_state.charge_percent;
    int is_charging = s_charge_state.is_charging ? 1 : 0;
    int is_plugged = s_charge_state.is_plugged ? 1 : 0;
    dict_write_int(iter, MESSAGE_KEY_charge_percent, &charge_percent, sizeof(int), true);
    dict_write_int(iter, MESSAGE_KEY_is_charging, &is_charging, sizeof(int), true);
    dict_write_int(iter, MESSAGE_KEY_is_plugged, &is_plugged, sizeof(int), true);
    app_message_outbox_send();
  }

  // Schedule the timeout timer
  const int interval_ms = 1000;
  s_timeout_timer = app_timer_register(interval_ms, timeout_timer_handler, NULL);
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox sent %d", s_request_status);
  // Successful message, the timeout is not needed anymore for this message
  app_timer_cancel(s_timeout_timer);
  if (s_request_status == REQUEST_STATE_SEND_BATTERY_LEVEL)
  {
    s_request_status = REQUEST_STATE_SEND_SUCCESSFUL;
    static char battery_text[46];
    snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Processing...", s_charge_state.charge_percent);
    text_layer_set_text(s_text_layer, battery_text);
  }
}

// AppMessage receive handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool settingsSet = false;

  Tuple *postRequestSent_t = dict_find(iter, MESSAGE_KEY_postRequestSent);
  if (postRequestSent_t) {
    static char battery_text[64];

    bool isConfigOpen = postRequestSent_t->value->int32;
    if(!isConfigOpen){
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Success!", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);

      // Also close the app when not in config
      const int interval_ms = 500;
      app_timer_register(interval_ms, delayed_quit_self, NULL); // we use a small delay so user can perceive the sent message.
    } else {
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Success! (Close with back button)", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);
    }
  }

  Tuple *httpError_t = dict_find(iter, MESSAGE_KEY_httpError);
  if (httpError_t) {
    s_request_status = REQUEST_STATE_RECEIVED_BAD_RESPONSE;
    static char battery_text[46];
    char *statusCode = httpError_t->value->cstring;
    snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Fail! %s", s_charge_state.charge_percent, statusCode);
    text_layer_set_text(s_text_layer, battery_text);
  }

  Tuple *sendWhenBatteryChanged_t = dict_find(iter, MESSAGE_KEY_sendWhenBatteryChanged);
  if (sendWhenBatteryChanged_t) {
    settings.SendWhenBatteryChanged = sendWhenBatteryChanged_t->value->int32;
    settingsSet = true;
  }
  
  if (settingsSet) {
    prv_save_settings();
    if (settings.SendWhenBatteryChanged) {
      if (!app_worker_is_running()) {
        AppWorkerResult result = app_worker_launch();
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Worker launch result %d", result);
      }
    } else { // stop worker
      if (app_worker_is_running()) {
        app_worker_kill();
      }
    }
  }

  Tuple *ready_t = dict_find(iter, MESSAGE_KEY_JSReady);
  if (ready_t) {
    s_js_ready = true; // safe to send messages
  }

  Tuple *is_config_open_t = dict_find(iter, MESSAGE_KEY_is_config_open);
  if (is_config_open_t) {
    bool config_was_open = is_config_open();
    s_config_open = is_config_open_t->value->int32 == 1;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Config open %d", s_config_open?1:0);

    if (config_was_open && !is_config_open()) { // We close the app automatically since we were setting config
      // close app
      quit_self();
    }
  }
}

static void update(struct tm *tick_time, TimeUnits units_changed) { // runs every second 
  if(s_request_status == REQUEST_STATE_WAIT_FOR_JS_READY)
  {
    static char battery_text[46];
    if (is_js_ready())
    {
      if(is_config_open()) {
        s_request_status = REQUEST_STATE_NONE;
        text_layer_set_text(s_text_layer, "Open for config. (Close with back button when done)");
      } else {
        s_request_status = REQUEST_STATE_SEND_BATTERY_LEVEL;
        snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Sending...", s_charge_state.charge_percent);
        text_layer_set_text(s_text_layer, battery_text);
  
        // tell js to send the charge state
        send_battery_charge_state_with_timeout();
      }
    }
    else
    {
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Waiting to send...", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);
    }
  }
}

static void prv_get_battery_level() {
  s_charge_state = battery_state_service_peek();
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 60, bounds.size.w, 50));
  text_layer_set_text(s_text_layer, "Fetching battery...");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
 
  if (s_config_set) {
    prv_get_battery_level();
    s_request_status = REQUEST_STATE_WAIT_FOR_JS_READY;
  } else {
    text_layer_set_text(s_text_layer, "Config not yet set! Please set this in the Pebble app!");
  }
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  prv_load_settings();

  s_request_status = REQUEST_STATE_NONE; // reset

  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_open(128, 128);

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, update);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;

  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
