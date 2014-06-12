#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

//ceate layers for lock image
static BitmapLayer *base_lock_icon_layer;
static GBitmap *base_lock_icon_bitmap = NULL;
static BitmapLayer *top_lock_icon_layer;
static GBitmap *top_lock_icon_bitmap = NULL;

//ceate layers for unlock image
static BitmapLayer *base_unlock_icon_layer;
static GBitmap *base_unlock_icon_bitmap = NULL;
static BitmapLayer *top_unlock_icon_layer;
static GBitmap *top_unlock_icon_bitmap = NULL;

#define MAX_NUMBER_OF_LOCKS (10)
#define MAX_LOCK_NAME_LENGTH (25)
typedef struct {
	char text[MAX_LOCK_NAME_LENGTH];
} LockNameItem;
static LockNameItem lock_names[MAX_NUMBER_OF_LOCKS];
static int s_active_lock_name_index = 0;
static int numberOfLocks = 0;

DictionaryIterator *iter;

enum {
  LOCK_COMMAND = 0,         // TUPLE_INT
  LOCK_COMMAND_KEY = 1,  // TUPLE_CSTRING
  READY_STATE = 25,
};

static LockNameItem* get_lock_name_at_index(int index) {
  if (index < 0 || index >= MAX_NUMBER_OF_LOCKS) {
    return NULL;
  }

  return &lock_names[index];
}
static void lock_names_append(char *data) {
  if (s_active_lock_name_index == MAX_NUMBER_OF_LOCKS) { 
    return;
  }
  strcpy(lock_names[s_active_lock_name_index].text, data);
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "setting locknames[%d] to:%s", s_active_lock_name_index, lock_names[s_active_lock_name_index].text);
  s_active_lock_name_index++;
}
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	++s_active_lock_name_index;
	if (s_active_lock_name_index >= numberOfLocks) {
		s_active_lock_name_index = 0;
	}
	LockNameItem *item = get_lock_name_at_index(s_active_lock_name_index);
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "lock name at index %d is:%s", s_active_lock_name_index, item->text);
	text_layer_set_text(text_layer, item->text);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Sending Lock Command");
	
  Tuplet value = TupletCString(LOCK_COMMAND_KEY, "lock");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Sending Unlock Command");
	
  Tuplet values = TupletCString(LOCK_COMMAND_KEY, "Sending Unlock Command");
  dict_write_tuplet(iter, &values);
  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	window_set_background_color(window, GColorBlack);
  GRect bounds = layer_get_bounds(window_layer);
	
  app_message_outbox_begin(&iter);	
	
  //create the base of the lock
  base_lock_icon_layer = bitmap_layer_create(GRect((bounds.size.w - 18), 9, 18, 14));
  layer_add_child(window_layer, bitmap_layer_get_layer(base_lock_icon_layer));
  base_lock_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BASE_LOCK);
  bitmap_layer_set_bitmap(base_lock_icon_layer, base_lock_icon_bitmap);
  
  //create the top of the lock
  top_lock_icon_layer = bitmap_layer_create(GRect((bounds.size.w - 16), 0, 14, 9));
  layer_add_child(window_layer, bitmap_layer_get_layer(top_lock_icon_layer));
  top_lock_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOP_LOCK);
  bitmap_layer_set_bitmap(top_lock_icon_layer, top_lock_icon_bitmap);
	
  //create the base of the unlock
  base_unlock_icon_layer = bitmap_layer_create(GRect((bounds.size.w - 18), (bounds.size.h - 14), 18, 14));
  layer_add_child(window_layer, bitmap_layer_get_layer(base_unlock_icon_layer));
  base_unlock_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BASE_LOCK);
  bitmap_layer_set_bitmap(base_unlock_icon_layer, base_unlock_icon_bitmap);
  
  //create the top of the unlock
  top_unlock_icon_layer = bitmap_layer_create(GRect((bounds.size.w - 26), (bounds.size.h - 23), 14, 9));
  layer_add_child(window_layer, bitmap_layer_get_layer(top_unlock_icon_layer));
  top_unlock_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOP_LOCK);
  bitmap_layer_set_bitmap(top_unlock_icon_layer, top_unlock_icon_bitmap);
	
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, "Open Pebble App to Config");
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
	
  //destroy the bitmaps
  gbitmap_destroy(top_lock_icon_bitmap);
  gbitmap_destroy(base_lock_icon_bitmap);
  gbitmap_destroy(top_unlock_icon_bitmap);
  gbitmap_destroy(base_unlock_icon_bitmap);
  
  //destroy the icons
  bitmap_layer_destroy(top_lock_icon_layer);
  bitmap_layer_destroy(base_lock_icon_layer);
  bitmap_layer_destroy(top_unlock_icon_layer);
  bitmap_layer_destroy(base_unlock_icon_layer);
	
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
	//Tuple *ready_tuple = dict_find(sent, READY_STATE);
	//if (ready_tuple) {
	//	APP_LOG(APP_LOG_LEVEL_DEBUG, "4. Ready State sent");
	//} else {
		////APP_LOG(APP_LOG_LEVEL_DEBUG, "Something sent but it wasn't ready state");
	//}
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "failed to send message");
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
	 // incoming message received
	 Tuple *text_tuple = dict_find(received, 50);
     // Act on the found fields received
	 if (text_tuple) {
		 text_layer_set_text(text_layer, text_tuple->value->cstring);
		 APP_LOG(APP_LOG_LEVEL_DEBUG, "Text is being set to: %s", text_tuple->value->cstring);
	 }
	 Tuple *numberOfLocks_tuple = dict_find(received, 100);
     // Act on the found fields received
	 if (numberOfLocks_tuple) {
		 APP_LOG(APP_LOG_LEVEL_DEBUG, "2. The number of locks is: %d", numberOfLocks_tuple->value->uint8);
		 numberOfLocks = numberOfLocks_tuple->value->uint8;
	 }
	Tuple *lockName_tuple = dict_find(received, 0);
	//if key was 0
	if (lockName_tuple) {
		////APP_LOG(APP_LOG_LEVEL_DEBUG, "Key was 0");
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Lock# %d is:%s", s_active_lock_name_index, lockName_tuple->value->cstring);
			lock_names_append(lockName_tuple->value->cstring);
			text_layer_set_text(text_layer, lockName_tuple->value->cstring);

	} else { //end of if lockName tuple is true
		////APP_LOG(APP_LOG_LEVEL_DEBUG, "3. key was not 0");
	} //end of else
  ////APP_LOG(APP_LOG_LEVEL_DEBUG, "About out send ready state");
  //Tuplet values = TupletCString(READY_STATE, "ready");
  //dict_write_tuplet(iter, &values);
  //app_message_outbox_send();
} //end of in received handler


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "message dropped");
 }

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  //register all the appMessage event handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
	
  //set the size of the appMessage buffer
   const uint32_t inbound_size = 64;
   const uint32_t outbound_size = 64;
   app_message_open(inbound_size, outbound_size);
	
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}