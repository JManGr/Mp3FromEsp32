#include <Arduino.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
// #include <SD.h>
#include "AudioDefinitions.h"
#include "navigation.h"
#include "ToutchKey.h"
#include "Display.h"

#define SPI_SPEED SD_SCK_MHZ(40)

typedef void (*RunCommand_t)(char c);


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
  case 'm':
  case 'M':
  {
    Serial.printf("Mute is %s.\n", (toggleMute() ? "on" : "off"));
  }
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
    toggleRandomPlay();
    Serial.printf("Random Pay is %s.\n", (getPlayMode() == PLAYMODE_RND ? "On" : "Off"));
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

void setup()
{
  Serial.begin(115200);
  delay(1000);
  navigationSetup(Serial);
  runSerialCommand('c');
  setupTouch(runSerialCommand);
  setupDisplay();
  xTaskCreate(&doSoundLoop, "doSoundLoop", 1024 * 5, NULL, 1, NULL);
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
  //doSoundLoop(NULL);

  if (!mp3->isRunning())
  {
    if (!tryToPlayNextFile())
    {
      Serial.println(F("Playback form SD card done\n"));
      delay(1000);
    }
  }
  delay(200);
}