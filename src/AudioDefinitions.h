#ifndef _AUDIODEFINITIONS_H
#define _AUDIODEFINITIONS_H
#endif
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

extern AudioGeneratorMP3 *mp3;
extern AudioFileSourceSD *source;
extern AudioOutputI2S *out;
extern AudioFileSourceID3 *id3;
