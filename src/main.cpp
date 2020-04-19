#include <Arduino.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <SD.h>
#include <FS.h>
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
//#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *source = NULL;
//AudioOutputI2SNoDAC *out;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;
File dir;

// You may need a fast SD card. Set this as high as it will work (40MHz max).
#define SPI_SPEED SD_SCK_MHZ(40)

bool tryToPlayNextFile()
{
  File file = dir.openNextFile();
  if (file)
  {
    if (String(file.name()).endsWith(".mp3"))
    {
      if (source->isOpen())
      {
        source->close();
      }
      if (source->open(file.name()))
      {
        Serial.printf_P(PSTR("Playing '%s' from SD card...\n"), file.name());
        mp3->begin(source, out);
      }
      else
      {
        Serial.printf_P(PSTR("Error opening '%s'\n"), file.name());
      }
    }
    return true;
  }
  return false;
}

void runSerialCommand(char c)
{
  switch (c)
  {
  case 'n':
  case 'N':
    mp3->stop();
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
  audioLogger = &Serial;
  //SPIFFS.begin();
  if (!SD.begin())
    Serial.println("Error SD");
  source = new AudioFileSourceSD();
  //file = new AudioFileSourceSD("/Berlin.mp3");
  //id3 = new AudioFileSourceID3(file);
  //id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  //out = new AudioOutputI2SNoDAC();
  out = new AudioOutputI2S(0, 2, 8, 0);
  out->SetGain(0.5);
  mp3 = new AudioGeneratorMP3();
  //mp3->begin(file, out);
  dir = SD.open("/");
  if (dir.isDirectory())
    audioLogger->printf("is Dir 1\n");
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
  if (mp3->isRunning())
  {

    if (!mp3->loop())
    {
      mp3->stop();
    }
  }
  else
  {

    // audioLogger->printf("MP3 done\n");
    // delay(1000);
    File file = dir.openNextFile();
    if (!tryToPlayNextFile())
    {
      Serial.println(F("Playback form SD card done\n"));
      delay(1000);
    }
  }
}