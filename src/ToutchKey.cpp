#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ToutchKey.h"

#define MAX_USED_TOUCH 5
#define TOUCH_GAIN_PLUS TOUCH_PAD_NUM5
#define TOUCH_GAIN_MINUS TOUCH_PAD_NUM6
#define TOUCH_MODE TOUCH_PAD_NUM7
#define TOUCH_PREV_SONG TOUCH_PAD_NUM8
#define TOUCH_NEXT_SONG TOUCH_PAD_NUM9

typedef struct
{
  touch_pad_t number_TP;
  RunCommand_t cmd;
  void *pram;
  uint16_t lastValue;
} TTaste;

TTaste tasten[MAX_USED_TOUCH];
touch_pad_t tochNoGpioOrder[MAX_USED_TOUCH] = {TOUCH_GAIN_MINUS, TOUCH_GAIN_PLUS, TOUCH_MODE, TOUCH_PREV_SONG, TOUCH_NEXT_SONG};
const char tochParm[MAX_USED_TOUCH] = {'-', '+', 'r', 'p', 'n'};
static void read_Key_task(void *pvParameter)
{
  uint16_t touch_filter_value = 0;
  printf("Touch-Handler running\n");
  while (1)
  {
    for (uint8_t i = 0; i < MAX_USED_TOUCH; i++)
    {
      auto err = touch_pad_read_filtered(tasten[i].number_TP, &touch_filter_value);
      if (err != ESP_OK)
      {

        printf("Error: %i- %s", err, esp_err_to_name(err));
      }
      if (tasten[i].lastValue > 1000 && touch_filter_value < 1000)
      {
        printf("id:%i, old %i, new %i\n", tasten[i].number_TP, tasten[i].lastValue, touch_filter_value);
        tasten[i].cmd(*((char *)tasten[i].pram));
      }
      tasten[i].lastValue = touch_filter_value;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  printf("Touch-Handler stopped\n");
  vTaskDelete(NULL);
}

void setupTouch(RunCommand_t runCmd)
{
  touch_pad_init();
  touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
  for (int i = 0; i < MAX_USED_TOUCH; i++)
  {
    touch_pad_config(tochNoGpioOrder[i], 0);
    tasten[i].number_TP = tochNoGpioOrder[i];
    tasten[i].cmd =runCmd;
    tasten[i].pram = (void *)&tochParm[i];
    touch_pad_read_filtered(tasten[i].number_TP, &tasten[i].lastValue);
  }

  touch_pad_filter_start(10);
  xTaskCreate(&read_Key_task, "read_Key_task", 2048, NULL, 0, NULL);
}