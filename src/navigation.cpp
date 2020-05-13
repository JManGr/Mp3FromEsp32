#include <Arduino.h>
#include <dirent.h>
#include <SD.h>
#include <FS.h>
#include "AudioDefinitions.h"
#include "navigation.h"
#include "Display.h"

bool isMute;
bool StopPlay = false;
long playNumber = 0;
uint8_t playMode = PLAYMODE_DEFAULT;
float gain = 0.5;
volatile uint32_t soundsInDir = 0;

File tdir;

DIR *theDir = NULL;


void doSoundLoop(void *p)
{
  while (true)
  {
    if (mp3->isRunning())
    {
      if (isMute)
      {
        out->stop();
      }
      else
      {
        if (StopPlay || !mp3->loop())
        {
          StopPlay = false;
          mp3->stop();
        }
      }
    }
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}


File openNextFile()
{
  if (playMode == PLAYMODE_RND)
  {
    randomPlay();
  }
  struct dirent *ent;
  if (theDir == NULL)
  {
    theDir = opendir("/sd/");
  }
  if (theDir != NULL)
  {
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
        char* name=(char*)(file.name());
        // audioLogger->printf_P(PSTR("Playing (%ld) '%s' from SD card...\n"), playNumber, name);
         auto ptr = strtok(name, "-");
         char* l1 = NULL;
         char* l2 = NULL;
         char* l3 = NULL;
         if (ptr != NULL)
         {
           l1 = ptr;
           ptr = strtok(NULL, "-.");
           if (ptr != NULL)
           {
             l2 = ptr;
           }
         }
        else
        {
          l1 = name;
        }
        setLines(l1,l2,l3);

        mp3->begin(source, out);
      }
      else
      {
        audioLogger->printf_P(PSTR("Error opening '%s'\n"), file.name());
      }
    }
    return true;
  }
  return false;
}

uint8_t getPlayMode()
{
  return playMode;
}

void setPlayMode(uint8_t value)
{
  playMode=value;
  setModeIndicator(eModeindicators::RND,playMode== PLAYMODE_RND);
}

bool getIsMute()
{
  return isMute;
}

void setIsMute(bool value)
{
  isMute=value;
  setModeIndicator(eModeindicators::Mute,isMute);
}

void playNextSong()
{
  StopPlay = true;
}

void toggleRandomPlay()
{
  if (playMode == PLAYMODE_RND)
  {
    setPlayMode(PLAYMODE_DEFAULT);
    return;
  }
  setPlayMode(PLAYMODE_RND);
  StopPlay = true;
}

void randomPlay()
{
  playNumber = random(soundsInDir);
  audioLogger->printf("Play %ld/%d\n", playNumber, soundsInDir);
  if (playNumber < 0)
  {
    playNumber = 0;
  }
  seekdir(theDir, playNumber);
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
          audioLogger->printf("(%d) %s\n", i, ent->d_name);
        }
      }
    }
    closedir(dir1);
  }
  else
  {
    /* could not open directory */
    audioLogger->printf("could not open directory\n");
  }
  soundsInDir = i;
  audioLogger->printf("(%d)Files found.\n", i);
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

bool toggleMute()
{
  setIsMute(!isMute);
  return isMute;
}



void navigationSetup(HardwareSerial serial)
{
  //setPlayMode(PLAYMODE_DEFAULT);
  //setIsMute(false);
  randomSeed(analogRead(0));
  audioLogger = &Serial;
  if (!SD.begin())
    audioLogger->println("Error SD");
  source = new AudioFileSourceSD();
  out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S, 8, AudioOutputI2S::APLL_AUTO);
  out->SetGain(gain);
  
  mp3 = new AudioGeneratorMP3();
}
