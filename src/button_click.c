#include <pebble.h>
enum {
    KEY_GESTURE = 0,
    GESTURE_1 = 1,
    GESTURE_2 = 2,
    GESTURE_3 = 3,
    KEY_SEND_ROLE= 4,
    KEY_SEND_PHASE=5,
    KEY_SCORE_UPDATE= 6,
    WAITING_ROOM_SCREEN= 7,
    GAME_PLAY_SCREEN=8,
    FINAL_SCREEN=9  
};
static Window *window;
TextLayer *phase_layer, *score_layer, *role_layer, *gesture_layer;
char phase_buffer[64], score_buffer[32], role_buffer[32], gesture_buffer[32];


void send_int(uint8_t key, uint8_t cmd)
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
      
    Tuplet value = TupletInteger(key, cmd);
    dict_write_tuplet(iter, &value);
      
    app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(gesture_layer, "Last Gesture: SELECT");
  send_int(KEY_GESTURE, GESTURE_2);
}
 
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(gesture_layer, "Last Gesture: UP");
  send_int(KEY_GESTURE, GESTURE_1);
}
 
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(gesture_layer, "Last Gesture: DOWN");
  send_int(KEY_GESTURE, GESTURE_3);
}
 
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static TextLayer* init_text_layer(GRect location, GTextAlignment alignment, char * aFont)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_font(layer, fonts_get_system_font(aFont));
  text_layer_set_text_alignment(layer, alignment);
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
    case KEY_SEND_PHASE:
      //Location received
      if (value==WAITING_ROOM_SCREEN) {
        strcpy(string_value, "Waiting Room");
      } else if (value == GAME_PLAY_SCREEN) {
        strcpy(string_value,"Play!");
      } else if (value== FINAL_SCREEN) {
        strcpy(string_value,"Its the end!");
      }
      snprintf(phase_buffer, sizeof("Phase: a long phasename"), "Location: %s", string_value);
      text_layer_set_text(phase_layer, (char*) &phase_buffer);
      break;
    case KEY_SEND_ROLE:
      //Role received
      snprintf(role_buffer, sizeof("Role: a long role name"), "Role: %s", string_value);
      text_layer_set_text(role_layer, (char*) &role_buffer);
      break;
    case KEY_SCORE_UPDATE:
      snprintf(score_buffer, sizeof("Score: XXX"), "Role: %d", value);
      text_layer_set_text(score_layer, (char*) &score_buffer);
      break;
    break;
      
  }
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
  
  phase_layer = init_text_layer(GRect(5, 0, 144, 30), GTextAlignmentLeft, FONT_KEY_GOTHIC_18);
  text_layer_set_text(phase_layer, "Phase: INIT");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(phase_layer));
  
  score_layer = init_text_layer(GRect(0, 30, 138, 30), GTextAlignmentRight, FONT_KEY_GOTHIC_18_BOLD);
  text_layer_set_text(score_layer, "Score: 50");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(score_layer));
 
  role_layer = init_text_layer(GRect(5, 60, 144, 30), GTextAlignmentCenter, FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_text(role_layer, "");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(role_layer));
   
  gesture_layer = init_text_layer(GRect(5, 90, 144, 30), GTextAlignmentCenter, FONT_KEY_GOTHIC_18);
  text_layer_set_text(gesture_layer, "Last Gesture: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(gesture_layer));
  
}

void window_unload(Window *window) {
  //destroy all elements here
  text_layer_destroy(phase_layer);
  text_layer_destroy(score_layer);
  text_layer_destroy(role_layer);
  text_layer_destroy(gesture_layer);
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

