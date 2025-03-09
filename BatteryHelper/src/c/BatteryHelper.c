#define SETTINGS_KEY 1

#define REQUEST_STATE_NONE 0
#define REQUEST_STATE_WAIT_FOR_JS_READY 1
#define REQUEST_STATE_SEND_BATTERY_LEVEL 2
#define REQUEST_STATE_SEND_SUCCESSFUL 3
#define REQUEST_STATE_RECEIVED_BAD_RESPONSE 4

#include <pebble.h>

static void send_with_timeout(int key, int value);
static void timeout_timer_handler(void *context);

typedef struct ClaySettings {
  bool SendWhenAppOpened;
  bool SendWhenBatteryChanged;
  bool SendAtFixedTime;
  int8_t FixedTimeHours;
  int8_t FixedTimeMinutes;
} ClaySettings;

static ClaySettings settings;
static bool s_js_ready;
static bool s_permission_to_close;
static bool s_config_set;
static int8_t s_request_status;
static BatteryChargeState s_charge_state;
static AppTimer *s_timeout_timer;
static int s_last_key;
static int s_last_value;

static Window *s_window;
static TextLayer *s_text_layer;

// Initialize the default settings
static void prv_default_settings() {
  settings.SendWhenAppOpened = true;
  settings.SendWhenBatteryChanged = false;
  settings.SendAtFixedTime = false;
  settings.FixedTimeHours = 0;
  settings.FixedTimeMinutes = 0;
}

static bool comm_is_js_ready() {
  return s_js_ready;
}

static void quit_self() {
  window_stack_pop_all(true);
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
  send_with_timeout(s_last_key, s_last_value);
}

static void send_with_timeout(int key, int value) {
  s_last_key = key;
  s_last_value = value;
  // Construct and send the message
  DictionaryIterator *iter;
  if(app_message_outbox_begin(&iter) == APP_MSG_OK) {
    dict_write_int(iter, key, &value, sizeof(int), true);
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
    snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Waiting for JS...", s_charge_state.charge_percent);
    text_layer_set_text(s_text_layer, battery_text);
  }
}

// AppMessage receive handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool settingsSet = false;

  Tuple *ready_t = dict_find(iter, MESSAGE_KEY_JSReady);
  if (ready_t) {
    s_js_ready = true; // safe to send messages
    s_permission_to_close = false; // we just opened the app so not yet ready to close
  }

  Tuple *postRequestSent_t = dict_find(iter, MESSAGE_KEY_postRequestSent);
  if (postRequestSent_t) {
    static char battery_text[64];

    bool isConfigOpen = postRequestSent_t->value->int32;
    if(!isConfigOpen){
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Success!", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);
      quit_self(); // only close when not in config.
    } else {
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Success! (Close app with back button)", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);
    }
  }

  Tuple *httpError_t = dict_find(iter, MESSAGE_KEY_httpError);
  if (httpError_t) {
    s_request_status = REQUEST_STATE_RECEIVED_BAD_RESPONSE;
    static char battery_text[46];
    int statusCode = httpError_t->value->int32;
    snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Fail! %d", s_charge_state.charge_percent, statusCode);
    text_layer_set_text(s_text_layer, battery_text);
  }
  
  Tuple *sendWhenAppOpened_t = dict_find(iter, MESSAGE_KEY_sendWhenAppOpened);
  if (sendWhenAppOpened_t) {
    settings.SendWhenAppOpened = sendWhenAppOpened_t->value->int32;
    settingsSet = true;
  }

  Tuple *sendWhenBatteryChanged_t = dict_find(iter, MESSAGE_KEY_sendWhenBatteryChanged);
  if (sendWhenBatteryChanged_t) {
    settings.SendWhenBatteryChanged = sendWhenBatteryChanged_t->value->int32;
    settingsSet = true;
  }

  Tuple *sendAtFixedTime_t = dict_find(iter, MESSAGE_KEY_sendAtFixedTime);
  if (sendAtFixedTime_t) {
    settings.SendAtFixedTime = sendAtFixedTime_t->value->int32;
    settingsSet = true;
  }
  
  Tuple *fixedTime_t = dict_find(iter, MESSAGE_KEY_fixedTime);
  if (fixedTime_t) {
    settingsSet = true;
    char *time_str = fixedTime_t->value->cstring;  // Get time string "HH:MM"
    int8_t hours;
    int8_t minutes;
    
    // Parse "HH:MM" format
    if(parse_time(time_str, &hours, &minutes)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Parsed time: %02d:%02d", hours, minutes);
      settings.FixedTimeHours = hours;
      settings.FixedTimeMinutes = minutes;
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to parse time string: %s", time_str);
    }
  }
  
  if (settingsSet) {
    prv_save_settings();
  }
}

static void update(struct tm *tick_time, TimeUnits units_changed) { // runs every second 
  if(s_request_status == REQUEST_STATE_WAIT_FOR_JS_READY)
  {
    static char battery_text[46];
    if (comm_is_js_ready())
    {
      s_request_status = REQUEST_STATE_SEND_BATTERY_LEVEL;
      snprintf(battery_text, sizeof(battery_text), "Battery: %d%%\n Sending...", s_charge_state.charge_percent);
      text_layer_set_text(s_text_layer, battery_text);
      // tell js to send the level to endpoint
      send_with_timeout(MESSAGE_KEY_battery, s_charge_state.charge_percent);
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

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 50));
  text_layer_set_text(s_text_layer, "Fetching battery...");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  if (s_config_set) {
    if (settings.SendWhenAppOpened || settings.SendAtFixedTime) {
      prv_get_battery_level();
      s_request_status = REQUEST_STATE_WAIT_FOR_JS_READY;
    }
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
