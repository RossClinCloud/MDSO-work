#include <pebble.h>
  
#define KEY_TEMPERATUREC 0
#define KEY_CONDITIONS 1
#define KEY_TEMPERATUREF 2
#define TAP_NOT_DATA true
#define WAKEUP_REASON 0
#define WAKEUP_PERIOD 10
  
static Window *s_main_window;
static TextLayer *s_time_layer;    // for the current time
static TextLayer *s_weather_layer; // for the temp and weather message 
static TextLayer *s_company_layer; // for the logo
static TextLayer *s_message_layer; // for the TIME IS NOW message 
static WakeupId s_wakeup_id; // notifier for NOW to return to current time

static GFont s_time_font;
static GFont s_weather_font;
static GFont s_company_font;
static GFont s_message_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_mymsg_buffer[128];

  // Compose string of all data
   snprintf(s_mymsg_buffer, sizeof(s_mymsg_buffer), 
    "N X,Y,Z\n0 %d,%d,%d\n1 %d,%d,%d\n2 %d,%d,%d", 
    data[0].x, data[0].y, data[0].z, 
    data[1].x, data[1].y, data[1].z, 
    data[2].x, data[2].y, data[2].z
  );

  //Show the data
  //text_layer_set_text(s_output_layer, s_buffer);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  
//  switch (axis) {
//  case ACCEL_AXIS_X:
//    if (direction != 0) { 
//      if (strcmp(s_time_layer, "NOW!!!") == S_TRUE)
//         update_time();   
//    } else 
//      text_layer_set_text(s_time_layer, "NOW!!!");
//    break;
//    
//  case ACCEL_AXIS_Y:
//    if (direction != 0) {
//       if (strcmp(s_time_layer, "NOW!!!") == S_TRUE)
//         update_time();
//    } else 
//      text_layer_set_text(s_time_layer, "NOW!!!");
//    break;
//    
//  case ACCEL_AXIS_Z:
//    if (direction != 0) {
//      if (strcmp(s_time_layer, "NOW!!!") == S_TRUE)
//         update_time();
//    } else 
//      text_layer_set_text(s_time_layer, "NOW!!!");
//    break;
//  }
  text_layer_set_text(s_time_layer, "NOW!!!");
  
}

static void main_window_load(Window *window) {
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 30, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  //Apply to TextLayer
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
    //Create company TextLayer
  s_company_layer = text_layer_create(GRect(0, 100, 144, 25));
  text_layer_set_background_color(s_company_layer, GColorClear);
  text_layer_set_text_alignment(s_company_layer, GTextAlignmentCenter);
  text_layer_set_text(s_company_layer, "Medidata");
  // Apply system font and add company name to the window
  text_layer_set_font(s_company_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_company_layer));
  
  // Create temperature Layer
  s_weather_layer = text_layer_create(GRect(0, 120, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Getting weather...");
  // Apply a system font and add weather information to Window
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperatureC_buffer[8];
  static char temperatureF_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[48];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATUREC:
      snprintf(temperatureC_buffer, sizeof(temperatureC_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_TEMPERATUREF:
      snprintf(temperatureF_buffer, sizeof(temperatureF_buffer), "%dF", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s, %s", temperatureC_buffer, temperatureF_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Subscribe to the tap service to detect wrist shaking
   if (TAP_NOT_DATA) {
    // Subscribe to the accelerometer tap service
    accel_tap_service_subscribe(tap_handler);
  } 
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  
  // unsubscribe to the accelerometer service  
  if (TAP_NOT_DATA) {
    accel_tap_service_unsubscribe();
  } 
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

