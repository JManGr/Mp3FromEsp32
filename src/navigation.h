#ifndef _NAVIGATIO_H 
#define _NAVIGATIO_H 
#endif
#define PLAYMODE_DEFAULT 0
#define PLAYMODE_RMD 1





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