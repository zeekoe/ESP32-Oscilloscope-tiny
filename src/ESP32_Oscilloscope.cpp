#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <soc/syscon_reg.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "esp_adc_cal.h"
#include "filters.h"
#include "globals.h"
#include "i2s.h"
#include "screen.h"
#include "data_analysis.h"
#include "options_handler.h"

void characterize_adc();
void setup();
void loop();

void core0_task( void * pvParameters );
void core1_task( void * pvParameters );

//#define DEBUG_SERIAL
//#define DEBUG_BUFF

void IRAM_ATTR btok()
{
  btnok = 1;
}
void IRAM_ATTR btplus()
{
  btnpl = 1;
}
void IRAM_ATTR btminus()
{
  btnmn = 1;
}
void IRAM_ATTR btback()
{
  btnbk = 1;
}
void setup() {
  Serial.begin(115200);

  configure_i2s(1000000);
  
  setup_screen();
  
  pinMode(BUTTON_Ok , INPUT);
  pinMode(BUTTON_Plus , INPUT);
  pinMode(BUTTON_Minus , INPUT);
  pinMode(BUTTON_Back , INPUT);
  attachInterrupt(BUTTON_Ok, btok, RISING);
  attachInterrupt(BUTTON_Plus, btplus, RISING);
  attachInterrupt(BUTTON_Minus, btminus, RISING);
  attachInterrupt(BUTTON_Back, btback, RISING);

  characterize_adc();
#ifdef DEBUG_BUF
  debug_buffer();
#endif

  xTaskCreatePinnedToCore(
    core0_task,
    "menu_handle",
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &task_menu,  /* Task handle. */
    0); /* Core where the task should run */

  xTaskCreatePinnedToCore(
    core1_task,
    "adc_handle",
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    3,  /* Priority of the task */
    &task_adc,  /* Task handle. */
    1); /* Core where the task should run */
}


void core0_task( void * pvParameters ) {

  (void) pvParameters;

  for (;;) {
    menu_handler();

    if (new_data || menu_action) {
      new_data = false;
      menu_action = false;

      Serial.println("CORE0 - before updating screen");

      updating_screen = true;
      update_screen(i2s_buff, RATE);
      updating_screen = false;
      vTaskDelay(pdMS_TO_TICKS(10));
      Serial.println("CORE0");
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

void core1_task( void * pvParameters ) {

  (void) pvParameters;

  for (;;) {
    if (!single_trigger) {
      while (updating_screen) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      if (!stop) {
        if (stop_change) {
          i2s_adc_enable(I2S_NUM_0);
          stop_change = false;
        }
        ADC_Sampling(i2s_buff);
        new_data = true;
      }
      else {
        if (!stop_change) {
          i2s_adc_disable(I2S_NUM_0);
          i2s_zero_dma_buffer(I2S_NUM_0);
          stop_change = true;
        }
      }
      Serial.println("CORE1");
      vTaskDelay(pdMS_TO_TICKS(300));
    }
    else {
      float old_mean = 0;
      while (single_trigger) {
        stop = true;
        ADC_Sampling(i2s_buff);
        float mean = 0;
        float max_v, min_v;
        peak_mean(i2s_buff, BUFF_SIZE, &max_v, &min_v, &mean);

        //signal captured (pp > 0.4V || changing mean > 0.2V) -> DATA ANALYSIS
        if ((old_mean != 0 && fabs(mean - old_mean) > 0.2) || to_voltage(max_v) - to_voltage(min_v) > 0.05) {
          float freq = 0;
          float period = 0;
          uint32_t trigger0 = 0;
          uint32_t trigger1 = 0;

          //if analog mode OR auto mode and wave recognized as analog
          bool digital_data = !false;
          if (digital_wave_option == 1) {
            trigger_freq_analog(i2s_buff, RATE, mean, max_v, min_v, &freq, &period, &trigger0, &trigger1);
          }
          else if (digital_wave_option == 0) {
            digital_data = digital_analog(i2s_buff, max_v, min_v);
            if (!digital_data) {
              trigger_freq_analog(i2s_buff, RATE, mean, max_v, min_v, &freq, &period, &trigger0, &trigger1);
            }
            else {
              trigger_freq_digital(i2s_buff, RATE, mean, max_v, min_v, &freq, &period, &trigger0);
            }
          }
          else {
            trigger_freq_digital(i2s_buff, RATE, mean, max_v, min_v, &freq, &period, &trigger0);
          }

          single_trigger = false;
          new_data = true;
          Serial.println("Single GOT");
          //return to normal execution in stop mode
        }

        vTaskDelay(pdMS_TO_TICKS(1));   //time for the other task to start (low priorit)

      }
      vTaskDelay(pdMS_TO_TICKS(300));
    }
  }
}

void loop() {}


void characterize_adc() {
  esp_adc_cal_characterize(
    (adc_unit_t)ADC_UNIT_1,
    (adc_atten_t)ADC_CHANNEL,
    (adc_bits_width_t)ADC_WIDTH_BIT_12,
    1100,
    &adc_chars);
}
