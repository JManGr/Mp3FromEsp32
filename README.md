Playback of MP3 files with SD card and ESP32. 
A proof of concept.
Based on ESP8266Audio library (https://github.com/earlephilhower/ESP8266Audio) from Earle F. Philhower, III

The display control is based on the library ESP8266_SSD1306 by Fabrice Weinberg (https://thingpulse.com)



**Circuit**

Micro-SD|ESP32|Headphone*|Touch key| PCM5102*|SH1106
--------|-------|----------|-------|-------|------
VSS|5V (3.3V)|  |  |VIN(3.3V)|VCC(5V)
GND|GND|GND|  |GND|GND  
MISO|GPIO19|  |   |      |  
MOSI|GPIO23|  |  |      |  
SCK|GPIO18|  |  |      |  
CS|GPIO05|  |  |      |  
|  |GPIO25|Left|  |LRCK|  
|  |GPIO26|Right| |BCK|  
|  |GPIO22|  | |DIN|  
|  |GPIO12|  | (T5) Gain Minus|  |  
|  |GPIO14|  | (T6) Gain Plus|  |  
|  |GPIO27|  | (T7) Toggel Random Play|  |  
|  |GPIO33|  | (T8) Next Song|  |  
|  |GPIO32|  | (T9) Prev. Song|  |
|  |GPIO16|  | |  |SCL
|  |GPIO21|  | |  |SDA


*Headphones or PCM5102 are mutually exclusive. The headphones use the internal DAC or PDM. PCM5102 is an external DAC. It is selected via the second parameter (output_mode) when calling the constructor of AudioOutputI2S. An enum for output_mode is available in class AudioOutputI2S.  
AudioOutputI2S is a class from the ESP8266Audio library