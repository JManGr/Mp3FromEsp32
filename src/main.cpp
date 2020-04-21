#include <Arduino.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <SD.h>
#include <FS.h>
#include <dirent.h>

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
//File dir;
File tdir;

DIR *theDir = NULL;

bool StopPlay = false;
float gain = 0.5;

// You may need a fast SD card. Set this as high as it will work (40MHz max).
#define SPI_SPEED SD_SCK_MHZ(40)
long playNumber = 0;

File openNextFile()
{
  struct dirent *ent;
  if (theDir == NULL)
  {
    theDir = opendir("/sd/");
  }
  if (theDir != NULL)
  {
    /* print all the files and directories within directory */
    if ((ent = readdir(theDir)) != NULL)
    {
      return SD.open("/" + String(ent->d_name));
    }
  }
  return File();
}

bool tryToPlayNextFile()
{
  File file = openNextFile();
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

        Serial.printf_P(PSTR("Playing (%ld) '%s' from SD card...\n"), playNumber, file.name());
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

void playNextSong()
{
  StopPlay = true;
}

void playPreviousSong(void *p)
{
  playNumber = telldir(theDir) - 2;
  if (playNumber < 0)
  {
    playNumber = 0;
  }
  seekdir(theDir, playNumber);
  StopPlay = true;
  vTaskDelete(NULL);
}

void listOrCountSongs(bool showFilenames)
{

  int32_t i = 0;
  DIR *dir1;
  struct dirent *ent;
  if ((dir1 = opendir("/sd/")) != NULL)
  {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir1)) != NULL)
    {
      if (String(ent->d_name).endsWith(".mp3"))
      {
        i++;
        if (showFilenames)
        {
          Serial.printf("(%d) %s\n", i, ent->d_name);
        }
      }
    }
    closedir(dir1);
  }
  else
  {
    /* could not open directory */
    Serial.printf("could not open directory\n");
  }

  Serial.printf_P(PSTR("(%d)Files found.\n"), i);
  vTaskDelete(NULL);
}

void countSongs(void *p)
{
  listOrCountSongs(false);
}
void listSongs(void *p)
{
  listOrCountSongs(true);
}

void incGain()
{
  if (gain < 3.9)
  {
    gain += 0.1;
    out->SetGain(gain);
  }
}

void decGain()
{
  if (gain > 0.1)
  {
    gain -= 0.1;
    out->SetGain(gain);
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
  //dir = SD.open("/");
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