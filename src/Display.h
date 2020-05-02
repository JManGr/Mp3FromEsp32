#ifndef _DISPLAY_H
#define _DISPLAY_H
#endif
enum eModeindicators{ Mute=0, RND=3};
extern void setupDisplay();
extern void setLines(char* l1,char* l2,char* l3);
extern void setModeIndicator(eModeindicators mi, bool enable);