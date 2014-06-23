#include <pebble.h>

static Window *window;

//create the layers for text
static TextLayer *lock_name_text_layer;
static TextLayer *status_text_layer;
static TextLayer *lock_count_text_layer;
static char lock_count_text[7];

//ceate layers for lock image
static BitmapLayer *base_lock_icon_layer;
static GBitmap *base_lock_icon_bitmap = NULL;
static BitmapLayer *top_lock_icon_layer;
static GBitmap *top_lock_icon_bitmap = NULL;

//create layers for lockitron icon image
static BitmapLayer *lockitron_icon_layer;
static GBitmap *lockitron_icon_bitmap = NULL;

//ceate layers for unlock image
static BitmapLayer *base_unlock_icon_layer;
static GBitmap *base_unlock_icon_bitmap = NULL;
static BitmapLayer *top_unlock_icon_layer;
static GBitmap *top_unlock_icon_bitmap = NULL;

#define MAX_NUMBER_OF_LOCKS (10)
#define MAX_LOCK_NAME_LENGTH (25)
	
//define the structure of lockNameItem
typedef struct {
	char text[MAX_LOCK_NAME_LENGTH];
} LockNameList;

//create an instance of LockNameList to hold all the lock names
static LockNameList lock_names[MAX_NUMBER_OF_LOCKS];
//create an int value to hold the index of the current active lock
static int s_active_lock_name_index = 0;
//sn int value of the number of available locks;
static int numberOfLocks = 0;

//an iterater to iterate through the dictionary?
DictionaryIterator *iter;

//static key values used when sending messages to the Pebble
enum {
	UNLOCK_COMMAND = 70,
	LOCK_COMMAND = 60,
	READY_STATE = 25,
	LOCK_NAME_TEXT = 0,
	LOCK_COUNT = 100,
	SELECT_TEXT = 50,
	STATUS_TEXT = 90,
};

//function of LockNameList to get the lock name at index
static LockNameList* get_lock_name_at_index(int index) {
  if (index < 0 || index >= MAX_NUMBER_OF_LOCKS) {
    return NULL;
  }

  return &lock_names[index];
}
//function of LockNamelist to add a lockName to the list
static void lock_names_append(char *data) {
  if (s_active_lock_name_index == MAX_NUMBER_OF_LOCKS) { 
    return;
  }
  strcpy(lock_names[s_active_lock_name_index].text, data);
  s_active_lock_name_index++;
}

//function to iterate through the locks when the user clicks select button
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	//increment the index up to the numberOfLocks then reset
	++s_active_lock_name_index;
	if (s_active_lock_name_index >= numberOfLocks) {
		s_active_lock_name_index = 0;
	}
	//load the name of the active lock into a temporary item
	LockNameList *temp_item = get_lock_name_at_index(s_active_lock_name_index);
	//update the text fields
	text_layer_set_text(lock_name_text_layer, temp_item->text);
	text_layer_set_text(status_text_layer, "");
	snprintf(lock_count_text, 7, "%d of %d", (s_active_lock_name_index+1), numberOfLocks);
	text_layer_set_text(lock_count_text_layer, lock_count_text);
}

//send lock command on pressing up button
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	//create Tuplet with key LOCK and object of lockName
	Tuplet value = TupletCString(LOCK_COMMAND, text_layer_get_text(lock_name_text_layer));
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
	text_layer_set_text(status_text_layer, "Locking...");
}

//send unlock command on down button
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	Tuplet value = TupletCString(UNLOCK_COMMAND, text_layer_get_text(lock_name_text_layer));
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
	text_layer_set_text(status_text_layer, "Unlocking...");
}

//configure the functions to call when each button is pressed
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

//function to load the window
static void window_load(Window *window) {
	//get the root window layer
	Layer *window_layer = window_get_root_layer(window);
	//set the window to black
	window_set_background_color(window, GColorBlack);
	//set the bounds to the bounds of the window layer
	GRect bounds = layer_get_bounds(window_layer);
	
	//now that the window has loaded begin sending messages
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
	
  //create the lock_name_text_layer
  lock_name_text_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_color(lock_name_text_layer, GColorWhite);
  text_layer_set_background_color(lock_name_text_layer, GColorBlack);
  text_layer_set_text(lock_name_text_layer, "Open Pebble App to Config");
  text_layer_set_text_alignment(lock_name_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(lock_name_text_layer));
	
  //create the status_text_layer
  status_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { (bounds.size.w - 18), 20 } });
  text_layer_set_text_color(status_text_layer, GColorWhite);
  text_layer_set_background_color(status_text_layer, GColorBlack);
  text_layer_set_text(status_text_layer, "");
  text_layer_set_text_alignment(status_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(status_text_layer));

  //create the lock_count_text_layer
  lock_count_text_layer = text_layer_create((GRect) { .origin = { 0, 93 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_color(lock_count_text_layer, GColorWhite);
  text_layer_set_background_color(lock_count_text_layer, GColorBlack);
  text_layer_set_text(lock_count_text_layer, "");
  text_layer_set_text_alignment(lock_count_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(lock_count_text_layer));
	
  //create the lockitron_icon
  lockitron_icon_layer = bitmap_layer_create(GRect((50), 61, 95, 106));
  layer_add_child(window_layer, bitmap_layer_get_layer(lockitron_icon_layer));
  base_lock_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOCKITRON_ICON);
  bitmap_layer_set_bitmap(lockitron_icon_layer, lockitron_icon_bitmap);
}

//destroy all resources when unloading
static void window_unload(Window *window) {
  text_layer_destroy(lock_name_text_layer);
  text_layer_destroy(status_text_layer);
  text_layer_destroy(lock_count_text_layer);
	
  //destroy the bitmaps
  gbitmap_destroy(top_lock_icon_bitmap);
  gbitmap_destroy(base_lock_icon_bitmap);
  gbitmap_destroy(top_unlock_icon_bitmap);
  gbitmap_destroy(base_unlock_icon_bitmap);
  gbitmap_destroy(lockitron_icon_bitmap);
  
  //destroy the icons
  bitmap_layer_destroy(top_lock_icon_layer);
  bitmap_layer_destroy(base_lock_icon_layer);
  bitmap_layer_destroy(top_unlock_icon_layer);
  bitmap_layer_destroy(lockitron_icon_layer);
	
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered

 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "failed to send message");
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
	 // incoming message received
	 Tuple *text_tuple = dict_find(received, SELECT_TEXT);
     // Act on the found fields received
	 if (text_tuple) {
		 text_layer_set_text(lock_name_text_layer, text_tuple->value->cstring);
		 text_layer_set_text(lock_count_text_layer, "");
		 text_layer_set_text(status_text_layer, "");
	 }
	 Tuple *numberOfLocks_tuple = dict_find(received, LOCK_COUNT);
     // Act on the found fields received
	 if (numberOfLocks_tuple) {
		 numberOfLocks = numberOfLocks_tuple->value->uint8;
		 text_layer_set_text(lock_name_text_layer, "Loading locks...");
		 text_layer_set_text(status_text_layer, "Please wait");
		 text_layer_set_text(lock_count_text_layer, "");
	 }
	Tuple *lockName_tuple = dict_find(received, LOCK_NAME_TEXT);
	//if key was 0
	if (lockName_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Lock# %d is:%s", s_active_lock_name_index, lockName_tuple->value->cstring);
		lock_names_append(lockName_tuple->value->cstring);
		//check to see if your at the end of the locks and if so finally update screen with lock name
		if (s_active_lock_name_index >= numberOfLocks) {
			text_layer_set_text(lock_name_text_layer, lockName_tuple->value->cstring);
			text_layer_set_text(status_text_layer, "");
		} //end of checking to see if the active lock is higher than the number of locks
		snprintf(lock_count_text, 7, "%d of %d", s_active_lock_name_index, numberOfLocks);
		text_layer_set_text(lock_count_text_layer, lock_count_text);
	} //end of lockName_tuple being true
	Tuple *status_text_tuple = dict_find(received, STATUS_TEXT);
	if (status_text_tuple) {
		//werid things happen if the active lock name index is >= numberOfLocks when updating status
		if (s_active_lock_name_index >= numberOfLocks) {
			s_active_lock_name_index = (s_active_lock_name_index - 1); 
		}
		text_layer_set_text(status_text_layer, status_text_tuple->value->cstring);
		//sometimes the select text changes to status text so here I'm setting it back to the current lock name;
		LockNameList *temp_item = get_lock_name_at_index(s_active_lock_name_index);
		text_layer_set_text(lock_name_text_layer, temp_item->text);
		 APP_LOG(APP_LOG_LEVEL_DEBUG, "index:%d", s_active_lock_name_index);
		
		//if successfully locked or unlocked then vibrate once
		if (strcmp(status_text_tuple->value->cstring, "unlocked!") == 0 || strcmp(status_text_tuple->value->cstring, "locked!") == 0) {
			vibes_short_pulse();
		}

	 }
} //end of in received handler


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
	////APP_LOG(APP_LOG_LEVEL_DEBUG, "message dropped");
 }

static void init(void) {
	//create a window and store it in the window variable
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
	
	window_set_window_handlers( window, (WindowHandlers) {
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