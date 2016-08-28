#include <pebble.h>
#define HEIGHT 168.0
#define WIDTH 144.0
#define ROUND_HEIGHT 180
#define ROUND_WIDTH 180

Window *window;
Layer *gc;
time_t temp;
struct tm *tick_time;

const int timeRectSize=11;
const int dateRectSize=13;

GColor backgroundColor;
GColor drawColors[4];

uint16_t hour, minute, second;
uint16_t day, month, year;

int dispHeight;
int dispWidth;

void setDrawColors(GColor colors[]){
  drawColors[0]=colors[0];
  drawColors[1]=colors[1];
  drawColors[2]=colors[2];
  drawColors[3]=colors[3];
}

void drawBinaryRect(Layer *layer, GContext *gcx, int num, int numSize, GRect bounds){
  unsigned char tmp[numSize];
  
  for(int i=numSize-1;i>=0; i--, num/=2){
    tmp[i]=num%2;
  }
  
  int rectWidth=bounds.size.w/numSize;
  GRect rect={{bounds.origin.x, bounds.origin.y}, {rectWidth, bounds.size.h}};
  
  bounds.origin.x--;
  bounds.origin.y--;
  bounds.size.w+=2;
  bounds.size.h+=2;
  
  graphics_context_set_stroke_color(gcx, drawColors[3]);
  graphics_draw_rect(gcx, bounds);
  
  for(int i=0;i<numSize;i++){
    graphics_context_set_stroke_color(gcx, drawColors[2]);
    graphics_context_set_fill_color(gcx, drawColors[tmp[i]]);
    graphics_fill_rect(gcx, rect, 0, GCornerNone);
    graphics_draw_rect(gcx, rect);
    rect.origin.x+=rectWidth;
  }
}

void drawColon(Layer *layer, GContext *gcx, int cntrX, int cntrY){
  GRect colonPart={{cntrX, cntrY-4}, {3, 3}};
  drawBinaryRect(layer, gcx, 1, 1, colonPart);
  
  GRect colonPart2={{cntrX-3, cntrY+1}, {3, 3}};
  drawBinaryRect(layer, gcx, 1, 1, colonPart2);
}

void drawTime(Layer *layer, GContext *gcx){
  int cntrX=dispWidth/2;
  int cntrY=dispHeight/2;
  
  GRect hours={{cntrX-timeRectSize*11/2, cntrY-timeRectSize-1}, {timeRectSize*5, timeRectSize}};
  drawBinaryRect(layer, gcx, hour, 5, hours);
  
  drawColon(layer, gcx, cntrX, cntrY-timeRectSize/2);
  
  GRect minutes={{cntrX+timeRectSize/2, cntrY-timeRectSize-1}, {timeRectSize*6, timeRectSize}};
  drawBinaryRect(layer, gcx, minute, 6, minutes);
  
  GRect seconds={{cntrX-timeRectSize*3, cntrY+1}, {timeRectSize*6, timeRectSize}};
  drawBinaryRect(layer, gcx, second, 6, seconds);
}

void drawDate(Layer *layer, GContext *gcx){
  int cntrX=dispWidth/2;
  int cntrYDay=dispHeight/4;
  int cntrYYear=dispHeight*3/4;
  
  GRect days={{cntrX-dateRectSize*11/2, cntrYDay-dateRectSize/2}, {dateRectSize*5, dateRectSize}};
  drawBinaryRect(layer, gcx, day, 5, days);
  
  drawColon(layer, gcx, cntrX, cntrYDay);
  
  GRect months={{cntrX+dateRectSize/2, cntrYDay-dateRectSize/2}, {dateRectSize*4, dateRectSize}};
  drawBinaryRect(layer, gcx, month, 4, months);
  
  GRect years={{cntrX-dateRectSize*7/2, cntrYYear-dateRectSize/2}, {dateRectSize*7, dateRectSize}};
  drawBinaryRect(layer, gcx, year%100, 7, years);
}

void drawWatch(Layer *layer, GContext *gcx){
  drawTime(layer, gcx);
  drawDate(layer, gcx);
}

static void update_time(){
  temp=time(NULL);
  tick_time=localtime(&temp);
  
  hour=tick_time->tm_hour;
  if(!clock_is_24h_style())
    hour=hour%12;
  minute=tick_time->tm_min;
  second=tick_time->tm_sec;
  day=tick_time->tm_mday;
  month=tick_time->tm_mon;
  year=tick_time->tm_year;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changes){
  update_time();
  layer_mark_dirty(gc);
}

void load_window(Window *win){
  Layer *window_layer=window_get_root_layer(win);
  GRect bounds=layer_get_bounds(window_layer);
  
  gc=layer_create(bounds);
  
  layer_add_child(window_layer, gc);
  layer_set_update_proc(gc, drawWatch);
}

void unload_window(Window *win){
  layer_destroy(gc);
}

void init(){
  dispWidth=PBL_IF_ROUND_ELSE(ROUND_WIDTH, WIDTH);
  dispHeight=PBL_IF_ROUND_ELSE(ROUND_HEIGHT, HEIGHT);
  
  GColor defaultColors[]={GColorOxfordBlue, GColorYellow, GColorGreen, GColorDarkGreen};
  setDrawColors(defaultColors);
  
  window=window_create();
  backgroundColor=GColorBulgarianRose;
  window_set_background_color(window, backgroundColor);
  
  window_set_window_handlers(window, (WindowHandlers){
    .load=load_window,
    .unload=unload_window
  });
  
  window_stack_push(window, true);
  
  update_time();
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void deinit(){
  window_destroy(window);
}

int main(){
  init();
  
  app_event_loop();
  
  deinit();
  
  return 0;
}