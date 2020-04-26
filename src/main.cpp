#include <Arduino.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
// #include <SD.h>
#include "AudioDefinitions.h"
#include "navigation.h"

#define SPI_SPEED SD_SCK_MHZ(40)

void doSoundLoop()
{
  if (mp3->isRunning())
  {
    if (StopPlay || !mp3->loop())
    {
      StopPlay = false;
      mp3->stop();
    }
  }
}

void runSerialCommand(char c)
{
  switch (c)
  {
  case 'c':
  case 'C':
    xTaskCreatePinnedToCore(countSongs, "countSongs", 1024 * 10, NULL, 0, NULL, 1);
    break;
  case 'l':
  case 'L':
    xTaskCreatePinnedToCore(listSongs, "listSongs", 1024 * 10, NULL, 0, NULL, 1);
    break;

  case 'n':
  case 'N':
    playNextSong();
    break;
  case 'p':
  case 'P':
    xTaskCreate(playPreviousSong, "playPreviousSong", 1024 * 10, NULL, 0, NULL);
    break;
  case 'r':
  case 'R':
    toggelRandomPlay();
    Serial.printf("Random Pay is %s.\n", (playMode == PLAYMODE_RMD ? "On" : "Off"));
    break;

  case '+':
    incGain();
    Serial.printf("Gain: %.1f\n", gain);
    break;

  case '-':
    decGain();
    Serial.printf("Gain: %.1f\n", gain);
    break;
  default:
    break;
  }
}

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode)
  {
    string += 2;
  }
  int i = 0;
  while (*string && i++ < 40)
  {
    char a = *(string++);
    if (isUnicode)
    {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

#define MAX_USED_TOUCH 5
#define TOUCH_GAIN_PLUS TOUCH_PAD_NUM5
#define TOUCH_GAIN_MINUS TOUCH_PAD_NUM6
#define TOUCH_MODE TOUCH_PAD_NUM7
#define TOUCH_PREV_SONG TOUCH_PAD_NUM8
#define TOUCH_NEXT_SONG TOUCH_PAD_NUM9

typedef struct
{
  touch_pad_t number_TP;
  uint16_t lastValue;
} TTaste;

TTaste tasten[MAX_USED_TOUCH];
touch_pad_t tochNoGpioOrder[MAX_USED_TOUCH] = {TOUCH_GAIN_MINUS, TOUCH_GAIN_PLUS, TOUCH_MODE, TOUCH_PREV_SONG, TOUCH_NEXT_SONG};
static void read_Key_task(void *pvParameter)
{
  uint16_t touch_filter_value = 0;
  printf("Touch-Handler running\n");
  while (1)
  {
    // If open the filter mode, please use this API to get the touch pad count.
    for (uint8_t i = 0; i < MAX_USED_TOUCH; i++)
    {
      auto err = touch_pad_read_filtered(tasten[i].number_TP, &touch_filter_value);
      if (err != ESP_OK)
      {

        printf("Error: %i- %s", err, esp_err_to_name(err));
      }
      //auto dif = tasten[i].lastValue-touch_filter_value;

      // if(tasten[i].no_GPIO==4)
      // printf("id:%i, old %i, new %i\n",tasten[i].no_GPIO,tasten[i].lastValue,touch_filter_value);

      if (tasten[i].lastValue > 1000 && touch_filter_value < 1000)
      {
        printf("id:%i, old %i, new %i\n", tasten[i].number_TP, tasten[i].lastValue, touch_filter_value);
        switch (tasten[i].number_TP)
        {
        case TOUCH_GAIN_MINUS:
          runSerialCommand('-');
          break;
        case TOUCH_GAIN_PLUS:
          runSerialCommand('+');
          break;
        case TOUCH_MODE:
          runSerialCommand('r');
          break;
        case TOUCH_PREV_SONG:
          runSerialCommand('p');
          break;
        case TOUCH_NEXT_SONG:
          runSerialCommand('n');
          break;
        default:
          break;
        }
      }
      tasten[i].lastValue = touch_filter_value;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  printf("Touch-Handler stopped\n");
  vTaskDelete(NULL);
}

void setupTouch()
{
  touch_pad_init();
  touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
  touch_pad_config(TOUCH_NEXT_SONG, 0);
  touch_pad_config(TOUCH_PREV_SONG, 0);
  touch_pad_config(TOUCH_MODE, 0);
  touch_pad_config(TOUCH_GAIN_PLUS, 0);
  touch_pad_config(TOUCH_GAIN_MINUS, 0);

  touch_pad_filter_start(10);

  for (int i = 0; i < MAX_USED_TOUCH; i++)
  {
    tasten[i].number_TP = tochNoGpioOrder[i];
    touch_pad_read_filtered(tasten[i].number_TP, &tasten[i].lastValue);
  }

  xTaskCreate(&read_Key_task, "read_Key_task", 2048, NULL, 5, NULL);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  navigationSetup(Serial);
  runSerialCommand('c');
  setupTouch();
}

void loop()
{
  char c;
  if (Serial.available() > 0)
  {
    // read the next char from input.
    c = Serial.read();
    runSerialCommand(c);
  }
  doSoundLoop();

  if (!mp3->isRunning())
  {
    if (!tryToPlayNextFile())
    {
      Serial.println(F("Playback form SD card done\n"));
      delay(1000);
    }
  }
  delay(1);
}