Playback of MP3 files with SD card and ESP32. 
A proof of concept.
Based on ESP8266Audio library (https://github.com/earlephilhower/ESP8266Audio) from Earle F. Philhower, III



Circuit

Micro-SD|ESP32|Headphone|Touch key
--------|-------|----------|-------
VSS|5V (3.3V)|  |  
GND|GND|GND|  
MISO|GPIO19|  |   
MOSI|GPIO23|  |  
SCK|GPIO18|  |  
CS|GPIO05|  |  
|  |GPIO25|Left|  
|  |GPIO26|Right|  			
|  |GPIO24|  | (T5) Gain Minus
|  |GPIO24|  | (T6) Gain Plus
|  |GPIO24|  | (T7) Toggel Random Play
|  |GPIO32|  | (T8) Next Song
|  |GPIO33|  | (T9) Prev. Song