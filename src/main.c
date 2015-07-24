#include <pebble.h>

#define KEY_BUTTON    0
#define KEY_VIBRATE   1

#define BUTTON_UP     0
#define BUTTON_SELECT 1
#define BUTTON_DOWN   2
  
  
#define NUM_MENU_SECTIONS 2
#define NUM_MENU_ICONS 3
#define NUM_FIRST_MENU_ITEMS 1
#define NUM_SECOND_MENU_ITEMS 1

static Window *s_main_window;
static MenuLayer *s_menu_layer;
//static TextLayer *s_text_layer;

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

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch(t->key) {
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

// static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
// //   text_layer_set_text(s_text_layer, "Select");

//   send(KEY_BUTTON, BUTTON_SELECT);
// }

// static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
// //   text_layer_set_text(s_text_layer, "Up");

//   send(KEY_BUTTON, BUTTON_UP);
// }

// static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
// //   text_layer_set_text(s_text_layer, "Down");

//   send(KEY_BUTTON, BUTTON_DOWN);
// }

// static void click_config_provider(void *context) {
  // Assign button handlers
//   window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
//   window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//   window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
// }

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    case 1:
      return NUM_SECOND_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Wireless");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Something else");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
          menu_cell_basic_draw(ctx, cell_layer, "Wi-Fi", "ON", NULL);
          break;
//         case 1:
//           // This is a basic menu icon with a cycling icon
//           menu_cell_basic_draw(ctx, cell_layer, "Item", "Select to cycle", NULL);
//           break;
//         case 2: 
//           {
//             menu_cell_basic_draw(ctx, cell_layer, "Icon Item", "Last item", NULL);
//           }
//           break;
      }
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "Final Item");
          break;
      }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0 && cell_index->row == 0){
    send(KEY_BUTTON, BUTTON_SELECT);
  }
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    // This is the menu item with the cycling icon
    case 1:
//       // Cycle the icon
//       s_current_icon = (s_current_icon + 1) % NUM_MENU_ICONS;
//       // After changing the icon, mark the layer to have it updated
//       layer_mark_dirty(menu_layer_get_layer(menu_layer));
      break;
  }

}

/******************************* main_window **********************************/

static void main_window_load(Window *window) {
  // Create main TextLayer
//   s_text_layer = text_layer_create(GRect(0, 0, 144, 168));
//   text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
//   text_layer_set_text(s_text_layer, "Open Android app and press any button.");
//   text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
//   layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
  
  
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

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy main TextLayer
 // text_layer_destroy(s_text_layer);
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
//   window_set_click_config_provider(s_main_window, click_config_provider);
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
