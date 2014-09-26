#include <pebble.h>

static Window *window;

//create the layers for text
static TextLayer *lock_name_text_layer;
static TextLayer *lock_status_text_layer;
static TextLayer *lock_count_text_layer;
static char lock_count_text[7];

//ceate layers for lock image
static GBitmap *s_res_image_lockitron_lock;
static GBitmap *s_res_image_locked;
static GBitmap *s_res_image_unlocked;
static GFont s_res_gothic_18_bold;
static ActionBarLayer *actionbar_layer;
static BitmapLayer *lockitron_lock_bitmap_layer;

#define MAX_NUMBER_OF_LOCKS (10)
#define MAX_LOCK_NAME_LENGTH (36)

//create an array of char to hold the lock names
char lock_names_list[MAX_NUMBER_OF_LOCKS][MAX_LOCK_NAME_LENGTH];

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

//function to iterate through the locks when the user clicks select button
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	//check to make sure the select text isn't set to Open Pebble App to config, meaning no locks are loaded
	if (strcmp(text_layer_get_text(lock_name_text_layer), "Open Pebble App to") == 0 || strcmp(text_layer_get_text(lock_name_text_layer), "Loading locks...") == 0) {
		return;
	}

	//move to the next active lock unless that number is over the number of locks then start over
	++s_active_lock_name_index;
	if (s_active_lock_name_index >= numberOfLocks) {
		s_active_lock_name_index = 0;
	}

	//update the status text to show which lock your on
	text_layer_set_text(lock_name_text_layer, lock_names_list[s_active_lock_name_index]);
	snprintf(lock_count_text, 7, "%d of %d", (s_active_lock_name_index+1), numberOfLocks);
	text_layer_set_text(lock_status_text_layer, lock_count_text);
}

//send lock command on pressing up button
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	//check to make sure the select text isn't set to Open Pebble App to config, meaning no locks are loaded
	if (strcmp(text_layer_get_text(lock_name_text_layer), "Open Pebble App to") == 0 || strcmp(text_layer_get_text(lock_name_text_layer), "Loading locks...") == 0) {
		return;
	}
	//create Tuplet with key LOCK and object of lockName
	Tuplet value = TupletCString(LOCK_COMMAND, lock_names_list[s_active_lock_name_index]);
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
	text_layer_set_text(lock_status_text_layer, "Locking...");
}

//send unlock command on down button
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	//check to make sure the select text isn't set to Open Pebble App to config, meaning no locks are loaded
	if (strcmp(text_layer_get_text(lock_name_text_layer), "Open Pebble App to") == 0 || strcmp(text_layer_get_text(lock_name_text_layer), "Loading locks...") == 0) {
		return;
	}
	Tuplet value = TupletCString(UNLOCK_COMMAND, lock_names_list[s_active_lock_name_index]);
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
	text_layer_set_text(lock_status_text_layer, "Unlocking...");
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
	//Layer *window_layer = window_get_root_layer(window);
	//set the window to black
	window_set_background_color(window, GColorWhite);
	//set the bounds to the bounds of the window layer
	//GRect bounds = layer_get_bounds(window_layer);
	
	s_res_image_lockitron_lock = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOCKITRON_LOCK);
	s_res_image_locked = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOCKED);
	s_res_image_unlocked = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UNLOCKED);
	s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
	// actionbar_layer
	actionbar_layer = action_bar_layer_create();
	action_bar_layer_add_to_window(actionbar_layer, window);
	action_bar_layer_set_background_color(actionbar_layer, GColorBlack);
	action_bar_layer_set_icon(actionbar_layer, BUTTON_ID_UP, s_res_image_locked);
	action_bar_layer_set_icon(actionbar_layer, BUTTON_ID_DOWN, s_res_image_unlocked);
	layer_add_child(window_get_root_layer(window), (Layer *)actionbar_layer);
	action_bar_layer_set_click_config_provider(actionbar_layer, click_config_provider);
	
  // lockitron_lock_bitmap_layer
  lockitron_lock_bitmap_layer = bitmap_layer_create(GRect(37, 51, 50, 50));
  bitmap_layer_set_bitmap(lockitron_lock_bitmap_layer, s_res_image_lockitron_lock);
  layer_add_child(window_get_root_layer(window), (Layer *)lockitron_lock_bitmap_layer);
  
  // lock_name_text_layer
  lock_name_text_layer = text_layer_create(GRect(2, 3, 118, 50));
  text_layer_set_background_color(lock_name_text_layer, GColorClear);
  text_layer_set_text(lock_name_text_layer, "Loading locks...");
  text_layer_set_text_alignment(lock_name_text_layer, GTextAlignmentCenter);
  text_layer_set_font(lock_name_text_layer, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(window), (Layer *)lock_name_text_layer);
  
  // lockstatus_text_layer
  lock_status_text_layer = text_layer_create(GRect(2, 99, 118, 20));
  text_layer_set_text_alignment(lock_status_text_layer, GTextAlignmentCenter);
  text_layer_set_font(lock_status_text_layer, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(window), (Layer *)lock_status_text_layer);
	
  // lockstatus_text_layer
  lock_count_text_layer = text_layer_create(GRect(2, 124, 118, 20));
  text_layer_set_text_alignment(lock_count_text_layer, GTextAlignmentCenter);
  text_layer_set_font(lock_count_text_layer, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(window), (Layer *)lock_count_text_layer);
	
	//now that the window has loaded begin sending messages
	app_message_outbox_begin(&iter);	

}

//destroy all resources when unloading
static void window_unload(Window *window) {
  action_bar_layer_destroy(actionbar_layer);
  bitmap_layer_destroy(lockitron_lock_bitmap_layer);
  text_layer_destroy(lock_name_text_layer);
  text_layer_destroy(lock_status_text_layer);
  gbitmap_destroy(s_res_image_lockitron_lock);
  gbitmap_destroy(s_res_image_locked);
  gbitmap_destroy(s_res_image_unlocked);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
 	// outgoing message was delivered
	APP_LOG(APP_LOG_LEVEL_DEBUG, "outgoing message was delivered");
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
 	//outgoing message failed
	APP_LOG(APP_LOG_LEVEL_DEBUG, "outgoing message failed");
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
	 // incoming message received
	 Tuple *text_tuple = dict_find(received, SELECT_TEXT);
     // Act on the found fields received
	 if (text_tuple) {
		 text_layer_set_text(lock_count_text_layer, text_tuple->value->cstring);
		 //COME BACK TO THIS!!!!!!!!!!!!!!!!!!!
//		 text_layer_set_text(lock_count_text_layer, "");
		 //text_layer_set_text(lock_status_text_layer, "");
//		 if (strcmp(text_layer_get_text(lock_name_text_layer), "Open Pebble App to") == 0) {
//			 text_layer_set_text(lock_count_text_layer, "configure Settings");
//		 }
	 }
	 Tuple *numberOfLocks_tuple = dict_find(received, LOCK_COUNT);
     // Act on the found fields received
	 if (numberOfLocks_tuple) {
		 numberOfLocks = numberOfLocks_tuple->value->uint8;
		 text_layer_set_text(lock_name_text_layer, "Loading locks...");
		 text_layer_set_text(lock_status_text_layer, "Please wait");
		 text_layer_set_text(lock_count_text_layer, "");
	 }
	Tuple *lockName_tuple = dict_find(received, LOCK_NAME_TEXT);
	//if key was 0
	if (lockName_tuple) {
		//set the incoming lock name to the next active lock name in the list
		strcpy(lock_names_list[s_active_lock_name_index], lockName_tuple->value->cstring);
		s_active_lock_name_index++;
		//check to see if your at the end of the locks and if so finally update screen with lock name
		if (s_active_lock_name_index >= numberOfLocks) {
			text_layer_set_text(lock_name_text_layer, lockName_tuple->value->cstring);
			text_layer_set_text(lock_status_text_layer, "");
		} //end of checking to see if the active lock is higher than the number of locks
		snprintf(lock_count_text, 7, "%d of %d", s_active_lock_name_index, numberOfLocks);
		text_layer_set_text(lock_status_text_layer, lock_count_text);
	} //end of lockName_tuple being true
	Tuple *status_text_tuple = dict_find(received, STATUS_TEXT);
	if (status_text_tuple) {
		//werid things happen if the active lock name index is >= numberOfLocks when updating status
//		if (s_active_lock_name_index >= numberOfLocks) {
//			s_active_lock_name_index = (s_active_lock_name_index - 1); 
//		}

		APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating status text to:%s", status_text_tuple->value->cstring);
		if (strcmp(status_text_tuple->value->cstring, "unlocked!") == 0) {
			text_layer_set_text(lock_status_text_layer, "Unlocked!");
			vibes_short_pulse();
		} else if (strcmp(status_text_tuple->value->cstring, "locked!") == 0) {
			text_layer_set_text(lock_status_text_layer, "Locked!");
			vibes_short_pulse();
		} else if (strcmp(status_text_tuple->value->cstring, "Failed!") == 0) {
			text_layer_set_text(lock_status_text_layer, "Failed!");
			vibes_short_pulse();
		} else {
			text_layer_set_text(lock_status_text_layer, status_text_tuple->value->cstring);
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
	window_set_fullscreen(window, false);
	
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

