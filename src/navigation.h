#ifndef _NAVIGATIO_H 
#define _NAVIGATIO_H 
#endif
#define PLAYMODE_DEFAULT 0
#define PLAYMODE_RND 1





extern bool StopPlay;
extern long playNumber;
extern float gain;
extern volatile uint32_t soundsInDir;

extern void countSongs(void*);
extern void listSongs(void*);
extern void playPreviousSong(void*);
extern void doSoundLoop(void *p);

extern void playNextSong();
extern void toggleRandomPlay();
extern void randomPlay();
extern void incGain();
extern void decGain();
extern bool tryToPlayNextFile();
extern void navigationSetup(HardwareSerial serial);
extern bool toggleMute();
extern uint8_t getPlayMode();
extern bool getIsMute();
