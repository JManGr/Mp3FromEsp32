#ifndef _NAVIGATIO_H 
#define _NAVIGATIO_H 
#endif
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

#define PLAYMODE_DEFAULT 0
#define PLAYMODE_RMD 1




extern AudioGeneratorMP3 *mp3;
extern AudioFileSourceSD *source;
extern AudioOutputI2S *out;
extern AudioFileSourceID3 *id3;
extern bool StopPlay;
extern long playNumber;
extern uint8_t playMode;
extern float gain;
extern volatile uint32_t soundsInDir;

extern void countSongs(void*);
extern void listSongs(void*);
extern void playPreviousSong(void*);
extern void playNextSong();
extern void toggelRandomPlay();
extern void randomPlay();
extern void incGain();
extern void decGain();
extern bool tryToPlayNextFile();
extern void navigationSetup(HardwareSerial serial);
