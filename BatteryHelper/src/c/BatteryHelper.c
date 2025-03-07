#define SETTINGS_KEY 1
#include <pebble.h>

typedef struct ClaySettings {
  bool SendWhenAppOpened;
  bool SendWhenBatteryChanged;
  bool SendAtFixedTime;
  int8_t FixedTimeHours;
  int8_t FixedTimeMinutes;
} ClaySettings;

static ClaySettings settings;
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

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
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

static void QuitSelf()
{
  window_stack_pop_all(true);
}

// AppMessage receive handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Assign the values to our struct
  /*Tuple *endpoint_t = dict_find(iter, MESSAGE_KEY_endpoint);
  if (endpoint_t) {
    settings.Endpoint = endpoint_t->value->cstring;
  }*/
  
  Tuple *sendWhenAppOpened_t = dict_find(iter, MESSAGE_KEY_sendWhenAppOpened);
  if (sendWhenAppOpened_t) {
    settings.SendWhenAppOpened = sendWhenAppOpened_t->value->int32;
  }

  Tuple *sendWhenBatteryChanged_t = dict_find(iter, MESSAGE_KEY_sendWhenBatteryChanged);
  if (sendWhenBatteryChanged_t) {
    settings.SendWhenBatteryChanged = sendWhenBatteryChanged_t->value->int32;
  }

  Tuple *sendAtFixedTime_t = dict_find(iter, MESSAGE_KEY_sendAtFixedTime);
  if (sendAtFixedTime_t) {
    settings.SendAtFixedTime = sendAtFixedTime_t->value->int32;
  }
  
  Tuple *fixedTime_t = dict_find(iter, MESSAGE_KEY_fixedTime);
  if (fixedTime_t) {
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
  
  prv_save_settings();
}

static void prv_get_battery() {
  BatteryChargeState state = battery_state_service_peek();
  static char battery_text[16];
  snprintf(battery_text, sizeof(battery_text), "Battery: %d%%", state.charge_percent);
  text_layer_set_text(s_text_layer, battery_text);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  //text_layer_set_text(s_text_layer, "Fetching battery...");
  prv_get_battery();
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  prv_load_settings();

  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

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
