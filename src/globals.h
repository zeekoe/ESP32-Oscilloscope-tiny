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


float offset = 0;
float toffset = 0;

float v_div = 825;
float s_div = 10;

#define DELAY 1000

esp_adc_cal_characteristics_t adc_chars;

TaskHandle_t task_menu;
TaskHandle_t task_adc;

uint8_t current_filter = 1;

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

int8_t volts_index = 0;

int8_t tscale_index = 0;

uint8_t opt = None;

bool menu = false;
bool info = true;
bool set_value  = false;

float RATE = 1000; //in ksps --> 1000 = 1Msps

bool auto_scale = false;

bool full_pix = true;

bool stop = false;

bool stop_change = false;

uint16_t i2s_buff[BUFF_SIZE];

bool single_trigger = false;
bool data_trigger = false;

bool updating_screen = false;
bool new_data = false;
bool menu_action = false;
uint8_t digital_wave_option = 0; //0-auto | 1-analog | 2-digital data (SERIAL/SPI/I2C/etc)
int btnok,btnpl,btnmn,btnbk;