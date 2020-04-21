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
File dir ;
File tdir ;
bool StopPlay=false;
float gain=0.5;

// You may need a fast SD card. Set this as high as it will work (40MHz max).
#define SPI_SPEED SD_SCK_MHZ(40)
int32_t playNumber = 0;
bool tryToPlayNextFile()
{
  File file = dir.openNextFile();
  if (file)
  {
    if (String(file.name()).endsWith(".mp3"))
    {
      playNumber++;
      if (source->isOpen())
      {
        source->close();
      }
      if (source->open(file.name()))
      {

        Serial.printf_P(PSTR("Playing (%d) '%s' from SD card...\n"), playNumber, file.name());
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
void playNextSong()
{
  mp3->stop();
}

void playPreviousSong(void *p)
{
  int32_t sn = playNumber - 2;
  if (sn < 0)
  {
    sn = 0;
  }
  tdir = SD.open("/");
  playNumber = 0;
  while (playNumber < sn)
  {
    File f = tdir.openNextFile();
    if (f)
    {
      if (String(f.name()).endsWith(".mp3"))
      {
        playNumber++;
        Serial.printf_P(PSTR("Searching (%d) '%s'\n"), playNumber, f.name());
      }
      f.close();
    }
  }
  
  dir = tdir;
 // mp3->stop();
  StopPlay=true;
  vTaskDelete(NULL);
}

void listSongs(void *p)
{
  tdir = SD.open("/");
  File f = tdir.openNextFile();
  int32_t i = 0;
  while (f)
  {
    if (String(f.name()).endsWith(".mp3"))
    {
      i++;
      Serial.printf_P(PSTR("(%d) '%s'\n"), i, f.name());
    }
    f.close();
    f = tdir.openNextFile();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void incGain()
{
  if(gain<3.9)
  {
    gain+=0.1;
    out->SetGain(gain);
  }
}

void decGain()
{
if(gain>0.1)
  {
    gain-=0.1;
    out->SetGain(gain);
  }
}

void runSerialCommand(char c)
{
  switch (c)
  {
  case 'n':
  case 'N':
    playNextSong();
    break;
  case 'p':
  case 'P':    
    xTaskCreate(playPreviousSong, "playPreviousSong", 1024 * 10, NULL, 0, NULL);
    break;
  case 'l':
  case 'L':
    xTaskCreate(listSongs, "listSongs", 1024 * 10, NULL, 0, NULL);
    break;
    
  case '+':
    incGain();
    Serial.printf("Gain: %.1f\n",gain);
    break;
    
  case '-':
    decGain();
    Serial.printf("Gain: %.1f\n",gain);
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
  out->SetGain(gain);
  mp3 = new AudioGeneratorMP3();
  //mp3->begin(file, out);
  dir = SD.open("/");
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
    if (StopPlay||!mp3->loop())
    {
      StopPlay=false;
      mp3->stop();
    }
  }
  else
  {
    if (!tryToPlayNextFile())
    {
      Serial.println(F("Playback form SD card done\n"));
      delay(1000);
    }
  }
}