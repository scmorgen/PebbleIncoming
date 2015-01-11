#include <pebble.h>
enum {
    KEY_GESTURE = 0,
    GESTURE_1 = 1,
    GESTURE_2 = 2,
    GESTURE_3 = 3,
    KEY_SEND_ROLE= 4,
    KEY_SEND_PHASE=5,
    WAITING_ROOM_SCREEN= 6,
    GAME_PLAY_SCREEN=7,
    FINAL_SCREEN=8  
};
static Window *window;
TextLayer *title_layer, *location_layer, *temperature_layer, *time_layer;
char location_buffer[64], temperature_buffer[32], time_buffer[32], title_buffer[32];


void send_int(uint8_t key, uint8_t cmd)
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
      
    Tuplet value = TupletInteger(key, cmd);
    dict_write_tuplet(iter, &value);
      
    app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(title_layer, "Select");
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_SELECT);
}
 
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(title_layer, "Up");
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_UP);
}
 
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(title_layer, "Down");
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_DOWN);
}
 
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static TextLayer* init_text_layer(GRect location)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_font(layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
 
  return layer;
}

void process_tuple(Tuple *t)
{
  //Get key
  int key = t->key;
 
  //Get integer value, if present
  int value = t->value->int32;
 
  //Get string value, if present
  char string_value[32];
  strcpy(string_value, t->value->cstring);
 
  //Decide what to do
  switch(key) {
    case KEY_LOCATION:
      //Location received
      snprintf(location_buffer, sizeof("Location: couldbereallylongname"), "Location: %s", string_value);
      text_layer_set_text(location_layer, (char*) &location_buffer);
      break;
    case KEY_TEMPERATURE:
      //Temperature received
      text_layer_set_text(title_layer, "Yes we got the temperature");
      snprintf(temperature_buffer, sizeof("Temperature: XX \u00B0C"), "Temperature: %d \u00B0C", value);
      text_layer_set_text(temperature_layer, (char*) &temperature_buffer);
      break;
  }
 
  //Set time this update came in
  time_t temp = time(NULL);
  struct tm *tm = localtime(&temp);
  strftime(time_buffer, sizeof("Last updated: XX:XX"), "Last updated: %H:%M", tm);
  text_layer_set_text(time_layer, (char*) &time_buffer);
}
// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	(void) context;
  //int count= 0;
	Tuple * t = dict_read_first(received);
	while(t !=NULL) {
    //snprintf(title_buffer, sizeof("Count is: XX \u00B0C"), "Count is: %d \u00B0C", count);
    //text_layer_set_text(title_layer, title_buffer);
    
    process_tuple(t); 
    //Get next
    t = dict_read_next(received);
    //count++;
	}
	
}

void window_load(Window *window) {
  //adding window's elements here
  title_layer = init_text_layer(GRect(5, 0, 144, 30));
  text_layer_set_text(title_layer, "Openweathermap.org");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_layer));
  
  location_layer = init_text_layer(GRect(5, 30, 144, 30));
  text_layer_set_text(location_layer, "Location: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(location_layer));
 
  temperature_layer = init_text_layer(GRect(5, 60, 144, 30));
  text_layer_set_text(temperature_layer, "Temperature: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer));
   
  time_layer = init_text_layer(GRect(5, 90, 144, 30));
  text_layer_set_text(time_layer, "Last updated: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  
}

void window_unload(Window *window) {
  //destroy all elements here
  text_layer_destroy(title_layer);
  text_layer_destroy(location_layer);
  text_layer_destroy(temperature_layer);
  text_layer_destroy(time_layer);
}
 
static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  
  //Register AppMessage events
  app_message_register_inbox_received(in_received_handler);           
  app_message_open(512, 512);    //Large input and output buffer sizes
}
 
static void deinit(void) {
  app_message_deregister_callbacks();
  
  window_destroy(window);
}
 
int main(void) {
  init();
 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
 
  app_event_loop();
  deinit();
}

