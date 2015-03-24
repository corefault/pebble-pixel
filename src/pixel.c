#include <pebble.h>

static Window 		*window;
static TextLayer 	*text_time;
static Layer      *seconds_layer;

#define COLUMNS  6
#define ROWS     10
#define BLOCK_X  24
#define MARGIN   1

#ifdef PBL_COLOR
   #define BLOCK_Y  16
   #define THEME_LIMIT   5
#else
   #define BLOCK_Y  11
   #define THEME_LIMIT   1
#endif

GColor     theme_red[THEME_LIMIT];

/**
 * get information about hour
 */
static unsigned short get_display_hour(unsigned short hour ,char* ampm) {
  if (clock_is_24h_style()) {
     *ampm = '\0';
     return hour;
  }
  
  strcpy (ampm, (hour > 12) ? "PM" : "AM");
  unsigned short display_hour = hour % 12;
  return display_hour ? display_hour : 12;
}

/**
 * draw text lines 
 */
static void draw_text(GContext* ctx, int hour, int min) {
   char    buffer[10];
   char    ampm[3] = {0};
   
   graphics_context_set_text_color(ctx, GColorWhite);
   
   hour = get_display_hour(hour, ampm);
#ifdef PBL_COLOR
   if (strlen(ampm) > 0) {
      graphics_draw_text(ctx, ampm, fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,17,144,14), GTextOverflowModeFill, GTextAlignmentCenter, NULL);   
   } 
   snprintf (buffer, sizeof(buffer), "%d", hour);
   graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),GRect(0,32,144,44), GTextOverflowModeFill, GTextAlignmentCenter, NULL);   
   snprintf (buffer, sizeof(buffer), "%d", min);
   graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT),GRect(0,77,144,44), GTextOverflowModeFill, GTextAlignmentCenter, NULL);   
#else
   if (strlen(ampm) > 0) {
      graphics_draw_text(ctx, ampm, fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,115,144,14), GTextOverflowModeFill, GTextAlignmentCenter, NULL);   
   } 
   snprintf (buffer, sizeof(buffer), "%d:%02d", hour, min);
   graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT),GRect(0,124,144,44), GTextOverflowModeFill, GTextAlignmentCenter, NULL);      
#endif
}

/**
 * drawing seconds path
 */
static void seconds_display_callback(Layer *layer, GContext* ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int x = 0, y = 0, i, id = 0;
  int       line = 0;
  
  GRect rc;
  GColor    color = GColorWhite;
  rc.size.w = BLOCK_X - MARGIN;
  rc.size.h = BLOCK_Y - MARGIN;
  
  for (i = 1; i <= t->tm_sec; i++) {

     color = theme_red[id++];
     if (id >= THEME_LIMIT) {
        id = 0;
     }

     rc.origin.x = x;
     rc.origin.y = y;
     
     rc.size.h = BLOCK_Y - MARGIN;
     if (line > 0 && line < 9) {
        rc.size.h++;
     }
          
     graphics_context_set_fill_color(ctx, color);
     graphics_fill_rect(ctx, rc, 0, GCornerNone);

     // next row?
     x += BLOCK_X;
     if (i > 0 && (i % COLUMNS) == 0) {
        x = 0;
        y += BLOCK_Y;
        if (line > 0 && line < 9) {
           y++;
        }
        line++;
     }
  }
  
  //draw am/pm, hour and minute lines
  draw_text(ctx, t->tm_hour, t->tm_min);
}


/**
 * handle time tick 
 */
static void handle_time_tick(struct tm *tick_time, TimeUnits units_changed) {

   layer_mark_dirty(seconds_layer);
}

/**
 * init application 
 */
static void init(void) {
#ifdef PBL_COLOR
  theme_red[0] = GColorJazzberryJam;
  theme_red[1] = GColorBulgarianRose;
  theme_red[2] = GColorDarkCandyAppleRed;
  theme_red[3] = GColorFolly;
  theme_red[4] = GColorSunsetOrange;
#else 
  theme_red[0] = GColorWhite;
#endif
  
  window = window_create();
  window_stack_push(window, true);
  window_set_fullscreen(window, true);
  window_set_background_color(window, GColorBlack);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Init the layer for the minute display
  seconds_layer = layer_create(bounds);
  layer_set_update_proc(seconds_layer, seconds_display_callback);
  layer_add_child(window_layer, seconds_layer);

  tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick);
}

/**
 * deinitialize application
 */
static void deinit(void) {
  window_destroy(window);
}

/**
 * main entry point
 */
int main(void) {
  init();
  app_event_loop();
  deinit();
}
