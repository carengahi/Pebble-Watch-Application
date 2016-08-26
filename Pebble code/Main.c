#include <pebble.h>
static Window *window;
static TextLayer *hello_layer;
static AppTimer *timer; 
static char msg[100];   //buffer
static int celsius = 1;   //celsius flag
static int time_interval = 2000;  //time interval for auto requesting data
static int standby = 0;   //standby flag
static SimpleMenuLayer *ui_menu;
static SimpleMenuSection simple_menu_section[1];
static SimpleMenuItem menu_item[6]; //array of features


void out_sent_handler(DictionaryIterator *sent, void *context) {
}

//Error message sent if a message cannot be sent to the phone
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  text_layer_set_text(hello_layer, "Error out!");
}

//Computer the exponential power of a number
int power(int number, int power) {
  int result = 1;
  while (power > 0) {
    result *= number;
    power--;
  }
  return result;
}

//Check if a character is a number
int is_digit(char c) {
  if (c >= '0' && c <= '9') return 1;
  else return 0;
}

//Convert a string to a double
double stof(char *s) {
  double num = 0, digit = 1;
  if (*s == '-'){
    s++;
    digit = -1;
  };
  for (int point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1; 
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) digit /= 10.0f;
      num = num * 10.0f + (float)d;
    };
  };
  return num * digit;
}

//Convert a double to a string
void D_to_Char(double num, char* buffer) {
  int digit_left = 0;
  double temp = num;
  while (temp >= 1) {
    temp /= 10;
    digit_left++;
  } 
  // The left of the decimal point
  int left = (int) num;
  int total_digit = 1;
  while (total_digit <= digit_left) {
    int ten = power(10, digit_left - total_digit);
    int number = left / ten;
    buffer[total_digit - 1] = number + '0';
    left = left % ten;
    total_digit++;
  } 
  buffer[total_digit - 1] = '.';
  total_digit++; 
  
  // The right of the decimal point
  double right = num - ((int) num);
  while (total_digit < 7) {
    right *= 10;
    int number = ((int) right) % 10;
    buffer[total_digit - 1] = number + '0';
    total_digit++;
  }
}

//Compute Fahrenheit temperature from Celsius
void CtoF(char* celsius) {
 char buffer[7], f_buffer[100];
 strncpy(buffer, celsius, 6);
 buffer[6] = '\0';
 double cel = stof(buffer);
 double fahrenheit = cel * (9.0 / 5.0) + 32; 
 D_to_Char(fahrenheit, f_buffer);
 for (int i = 0; i < 5; i++) {
   celsius[i] = f_buffer[i];
 }
}

//Handle different keys received from the phone/server, and do something
void in_received_handler(DictionaryIterator *received, void *context) {

  //if key is 0, show current temperature
  Tuple *text_tuple = dict_find(received, 0); 
  if (text_tuple) {
    strcpy(msg, "Temperature: ");
    for(int i = 0; i < 99; i++){
      msg[i] = 0;
    }
      char* temp = text_tuple->value->cstring;
    for (int i = 0; i < 13; i++){
          temp++;
        }      
        strncpy(msg, temp, 5);
    
      if (celsius == 0) {        
      CtoF(msg);
      strcat(msg, " deg F");
    } else {
      strcat(msg, " deg C");
    }
    text_layer_set_text(hello_layer, msg);
    return;  
  }
  
  //if key is 1, run the Arduino timer
  text_tuple = dict_find(received, 1);
  if (text_tuple) {
    strcpy(msg, "Timer running...");
    text_layer_set_text(hello_layer, msg);
    return;    
  }
  
  //if key is 2, run the proximity sensor
  text_tuple = dict_find(received, 2);
  for(int i = 0; i < 99; i++){
      msg[i] = 0;
  }
  if (text_tuple) {
    strcpy(msg, text_tuple->value->cstring);
    strcat(msg, " inches");
    text_layer_set_text(hello_layer, msg);
    return;    
  }
  
  //if key is 3, show the statistics
  text_tuple = dict_find(received, 3);
  if (text_tuple) {
    char* p = text_tuple->value->cstring;
    strcpy(msg, "Low: ");
    char* low = malloc(32);
    char* high = malloc(32);
    char* avg = malloc(32);
    int c = 0;
    strncpy(low, p, 5);
    while (c < 6){
      p++;
      c++;
    }
    strncpy(high, p, 5);
    while (c < 12){
      p++;
      c++;
    }
    strcpy(avg, p);
    
    if (celsius == 0) {
      CtoF(low);
      strcat(msg, low);
      strcat(msg, " F\nHigh: ");
    } else {
      strcat(msg, low);
      strcat(msg, " C\nHigh: ");
    }
    if (celsius == 0) {
      CtoF(high);
      strcat(msg, high);
      strcat(msg, " F\nAvg: ");
    } else {
      strcat(msg, high);
      strcat(msg, " C\nAvg: ");
    }

    if (celsius == 0) {
      CtoF(avg);
      strcat(msg, avg);
      strcat(msg, " F");
    } else {
      strcat(msg, avg);
      strcat(msg, " C");
    }

    text_layer_set_text(hello_layer, msg);
    return;
  } 
  
  //if key is 9, server error, display error message
  text_tuple = dict_find(received, 9);
  if (text_tuple) {
    strcpy(msg, "Cannot connect to server");
    text_layer_set_text(hello_layer, msg);
    return;
  }

  
  //if key is 8, serial connection is broken, display error message
  text_tuple = dict_find(received, 8);
  if (text_tuple) {
    strcpy(msg, "Cannot connect to Arduino");
    text_layer_set_text(hello_layer, msg);
    return;
  }
  
  //Unidentified key, error message
  strcpy(msg, "Error: unfound key!");
  text_layer_set_text(hello_layer, msg);
}

//Error in receiving a key from the phone
void in_dropped_handler(AppMessageResult reason, void *context) {
  text_layer_set_text(hello_layer, "Error in receiving.");
}

//Quit the feature, including stopping the automatic timer, when back button is pressed
void back_button_handler(ClickRecognizerRef recognizer, void* context) {
  app_timer_cancel(timer);  
  window_stack_pop(true);
  window = window_stack_get_top_window();
}

//Handling when Temperature is pressed, and send a key 0 to the phone
void temp_click_handler(ClickRecognizerRef recognizer, void* context) {
    if (standby == 1) {
    text_layer_set_text(hello_layer, "Standby Mode \n No Information Available");
    return;
  } 
  DictionaryIterator *iter = NULL;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletCString(0, "hello?");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
  app_timer_reschedule(timer, time_interval);
}


//When time is up, call this to restart the timer
static void timer_func(void *data) {
  temp_click_handler(NULL, NULL);
  timer = app_timer_register(time_interval, timer_func, NULL);
}

//Registering button for Temperature
void config_provider_current_temp(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, temp_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_button_handler);
}

//Building a GUI for displaying temperature, and constantly call timer restarting for automatic polling
static void temp_GUI(int index, void *ctx) {
  window = window_create();
  text_layer_set_text(hello_layer, "Getting Temperature");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hello_layer));
  window_stack_push(window, true); 
  window_set_click_config_provider(window, config_provider_current_temp);  
  temp_click_handler(NULL, NULL); 
  timer = app_timer_register(3000, timer_func, NULL); 
}


////Handling when Arduino Timer is pressed, and send a key 1 to the phone
void arduino_timer_click_handler(ClickRecognizerRef recognizer, void* context) {
  DictionaryIterator *iter = NULL;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletCString(1, "hello?");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

//Registering button for Arduino Timer
void config_provider_timer(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, arduino_timer_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_button_handler);
}


//Building a GUI for Arduino Timer
static void timer_GUI(int index, void *ctx) { 
  window = window_create();
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hello_layer));
  window_stack_push(window, true);
  window_set_click_config_provider(window, config_provider_timer); 
  arduino_timer_click_handler(NULL, NULL); 
}

////Handling when Proximity is pressed, and send a key 2 to the phone
void proximity_handler(ClickRecognizerRef recognizer, void* context) {
    if (standby == 1) {
      text_layer_set_text(hello_layer, "Standby Mode \n No Information Available");
      return;
  }  
  DictionaryIterator *iter = NULL;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletCString(2, "hello?");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

//Registering button for Proximity
void config_provider_proximity(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, proximity_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_button_handler);
}

//Handling when Statistics is pressed, and send a key 3 to the phone
void stat_click_handler(ClickRecognizerRef recognizer, void* context) {
  DictionaryIterator *iter = NULL;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletCString(3, "hello?");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
  app_timer_reschedule(timer, time_interval);
}

//Another timer specifically for Statistics, same thing as the timer above
static void stat_timer_func(void *data) {
  stat_click_handler(NULL, NULL);
  timer = app_timer_register(time_interval, stat_timer_func, NULL);
}

////Another timer specifically for Proximity, same thing as the timer above
static void proximity_timer_func(void *data) {
  proximity_handler(NULL, NULL);
  timer = app_timer_register(time_interval, proximity_timer_func, NULL);
}

//Building a GUI for Proximity sensor to display distance
static void proximity_GUI(int index, void *ctx) {
  window = window_create();
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hello_layer));
  text_layer_set_text(hello_layer, "Getting distance...");
  window_stack_push(window, true); 
  window_set_click_config_provider(window, config_provider_proximity);  
  proximity_handler(NULL, NULL);
  timer = app_timer_register(3000, proximity_timer_func, NULL); 
}

//Registering button for Statistics
void config_provider_statistics(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, stat_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_button_handler);
}

//Building a GUI for Statistics to display statistics
static void statistics_GUI(int index, void *ctx) {
  window = window_create();
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hello_layer));
  text_layer_set_text(hello_layer, "Getting Statistics...");
  window_stack_push(window, true); 
  window_set_click_config_provider(window, config_provider_statistics);  
  stat_click_handler(NULL, NULL); 
  timer = app_timer_register(3000, stat_timer_func, NULL); 
}

//Handling when Standby is pressed, and send a key 4 or 5 to the phone
void standby_handler() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (standby) {
    Tuplet value = TupletCString(4, "hello？");
    dict_write_tuplet(iter, &value);
  } else {
    Tuplet value = TupletCString(5, "hello?");
    dict_write_tuplet(iter, &value);
  }
  app_message_outbox_send();
}

//Change the text on the Menu to indicate which mode we are in
static void standby_GUI(int index, void *ctx) {
  if (standby) {
    menu_item[3].title = "Standby Mode";
    standby = false;
  } else {
    menu_item[3].title = "Action Mode";
    standby = true;
  }
  standby_handler();
  layer_mark_dirty(simple_menu_layer_get_layer(ui_menu));
}

//Handling when Celsius/Fahrenheit is pressed, and send a key 6 or 7 to the phone
void change_units_handler() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if (celsius == 1) {
    Tuplet value = TupletCString(6, "hello?");
    dict_write_tuplet(iter, &value);
  } else {
    Tuplet value = TupletCString(7, "hello?");
    dict_write_tuplet(iter, &value);
  }
  app_message_outbox_send();
}

//Change the text on the menu to indicate which unit we see
static void units_GUI(int index, void *ctx) {
  if (celsius == 1) {
    menu_item[1].title = "Fahrenheit";
    celsius = 0;
  } else {
    menu_item[1].title = "Celsius";
    celsius = 1;
  }
  change_units_handler();
  layer_mark_dirty(simple_menu_layer_get_layer(ui_menu));
}



//Create the main menu
static void window_load(Window *window) {  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  menu_item[0] = (SimpleMenuItem) {
    .title = "Temperature",
    .callback = temp_GUI,
  };
  
  menu_item[4] = (SimpleMenuItem) {
    .title = "Arduino Timer",
    .callback = timer_GUI,
  };
  
  menu_item[5] = (SimpleMenuItem) {
    .title = "Proximity",
    .callback = proximity_GUI,
  };
  
    menu_item[2] = (SimpleMenuItem) {
    .title = "Statistics",
    .callback = statistics_GUI,
  };
  
  menu_item[1] = (SimpleMenuItem) {
    .title = "Celsius",
    .callback = units_GUI,
  };
  
   menu_item[3] = (SimpleMenuItem) {
    .title = "Standby Mode",
    .callback = standby_GUI,
  };
  
  simple_menu_section[0] = (SimpleMenuSection) {
    .num_items = 6,
    .items = menu_item,
  };

  ui_menu = simple_menu_layer_create(bounds, window, simple_menu_section, 1, NULL);
  
  hello_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 100 } });
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, simple_menu_layer_get_layer(ui_menu));
}


//Destroy the main menu
static void window_unload(Window *window) {
  text_layer_destroy(hello_layer);
  simple_menu_layer_destroy(ui_menu);
}

//Initialize the GUI
static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  
  const bool animated = true;
  window_stack_push(window, animated);
}

//Destroy the main menu
static void deinit(void) {
  window_destroy(window);
}

//Run the program
int main(void) {
  init();
  app_event_loop();
  deinit();
}