#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "esp_adc_cal.h"

#define ADC_CHANNEL   ADC1_CHANNEL_5  // GPIO33
#define NUM_SAMPLES   1000            // number of samples
#define I2S_NUM         (0)
#define BUFF_SIZE 50000
#define B_MULT BUFF_SIZE/NUM_SAMPLES

#define BUTTON_Ok        32
#define BUTTON_Plus        15
#define BUTTON_Minus        35
#define BUTTON_Back        34


#define DELAY 1000

extern float offset;
extern float toffset;

extern float v_div;
extern float s_div;

extern esp_adc_cal_characteristics_t adc_chars;

extern TaskHandle_t task_menu;
extern TaskHandle_t task_adc;

extern uint8_t current_filter;

//options handler
enum Option {
  None,
  Autoscale,
  Vdiv,
  Sdiv,
  Offset,
  TOffset,
  Filter,
  Stop,
  Mode,
  Single,
  Clear,
  Reset,
  Probe,
  UpdateF,
  Cursor1,
  Cursor2
};

extern int8_t volts_index;

extern int8_t tscale_index;

extern uint8_t opt;

extern bool menu;
extern bool info;
extern bool set_value;

extern float RATE;

extern bool auto_scale;

extern bool full_pix;

extern bool stop;

extern bool stop_change;

extern uint16_t i2s_buff[BUFF_SIZE];

extern bool single_trigger;
extern bool data_trigger;

extern bool updating_screen;
extern bool new_data;
extern bool menu_action;
extern uint8_t digital_wave_option; //0-auto | 1-analog | 2-digital data (SERIAL/SPI/I2C/etc)
extern int btnok,btnpl,btnmn,btnbk;

#endif