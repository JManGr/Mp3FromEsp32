Playback of MP3 files with SD card and ESP32. 
A proof of concept.
Based on ESP8266Audio library (https://github.com/earlephilhower/ESP8266Audio) from Earle F. Philhower, III



Circuit

Micro-SD|ESP32|Headphone*|Touch key| PCM5102*
--------|-------|----------|-------|-------
VSS|5V (3.3V)|  |  |VIN(3.3V)
GND|GND|GND|  |GNT
MISO|GPIO19|  |   |   
MOSI|GPIO23|  |  |   
SCK|GPIO18|  |  |   
CS|GPIO05|  |  |   
|  |GPIO25|Left|  |LRCK
|  |GPIO26|Right| |BCK
|  |GPIO22|  | |DIN
|  |GPIO24|  | (T5) Gain Minus
|  |GPIO24|  | (T6) Gain Plus
|  |GPIO24|  | (T7) Toggel Random Play
|  |GPIO32|  | (T8) Next Song
|  |GPIO33|  | (T9) Prev. Song


*Headphones or PCM5102 are mutually exclusive. The headphones use the internal DAC or PDM. PCM5102 is an external DAC. It is selected via the second parameter (output_mode) when calling the constructor of AudioOutputI2S. An enum for output_mode is available in class AudioOutputI2S.  
AudioOutputI2S is a class from the ESP8266Audio library