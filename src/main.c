#include <pebble.h>

#define KEY_WRITE   0
#define KEY_READ    1
#define KEY_TOGGLE  2
#define KEY_CONTROL 3
#define KEY_STATUS  4
#define KEY_VIBRATE 5

// #define BUTTON_UP     0
// #define BUTTON_SELECT 1
// #define BUTTON_DOWN   2
  
  
#define NUM_MENU_SECTIONS 2
#define MAX_MENU_ITEMS 15

static Window *s_main_window;
static MenuLayer *s_menu_layer;
//static TextLayer *s_text_layer;

static char *menu_sections[NUM_MENU_SECTIONS] = {"Wireless", "Misc"};
static char *menu_item_titles[NUM_MENU_SECTIONS][MAX_MENU_ITEMS] = {{"Wi-Fi"},{"Vibrate", "Ring"}};
static int menu_item_to_send[NUM_MENU_SECTIONS][MAX_MENU_ITEMS] = {{KEY_TOGGLE},{KEY_CONTROL, KEY_CONTROL}};
static uint16_t menu_item_count[] = {1,2};
static char *menu_item_statuses[NUM_MENU_SECTIONS][MAX_MENU_ITEMS];

static char* get_status_text(int status){
  if (status == 1){
    return "ON";
  }
  if (status == -1){
    return "OFF";
  }
  return "";
}
/******************************* AppMessage ***********************************/

static void send(int key, int message) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &message, sizeof(int), true);

  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  int key = (int)(t->key / 256);
  int menu_item_section = (int)((t->key % 256) / 16);
  int menu_item = (t->key % 16);
  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch(key) {
      case KEY_STATUS:
      APP_LOG(APP_LOG_LEVEL_INFO, "STATUS for KEY %d is %d", key, (int)t->value->int32);
        menu_item_statuses[menu_item_section][menu_item] = get_status_text((int)t->value->int32);
        menu_layer_reload_data(s_menu_layer);
        break;
      case KEY_VIBRATE:
        // Trigger vibration
//         text_layer_set_text(s_text_layer, "Vibrate!");
        APP_LOG(APP_LOG_LEVEL_INFO, "Vibrate!");
        vibes_short_pulse();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_INFO, "Unknown key: %d", (int)t->key);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/********************************* Buttons ************************************/

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return menu_item_count[section_index];
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
   menu_cell_basic_header_draw(ctx, cell_layer,  menu_sections[section_index]);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  
    menu_cell_basic_draw(ctx, cell_layer, menu_item_titles[cell_index->section][cell_index->row], menu_item_statuses[cell_index->section][cell_index->row], NULL);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    send(menu_item_to_send[cell_index->section][cell_index->row], (256 * cell_index->section + cell_index->row));
}

/******************************* main_window **********************************/

static void main_window_load(Window *window) {
    // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  send(KEY_READ, 0); //WiFi

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  
}

static void main_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void init(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_outbox_sent(outbox_sent_handler);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
