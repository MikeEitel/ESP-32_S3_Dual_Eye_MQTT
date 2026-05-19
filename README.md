# ESP-32_S3_Dual_Eye_MQTT
This is an augmented version of the thelastoutpostworkshop's Dual EYE waveshare 0.71 board with an XIAO-ESP32-S3

I dislike DHCP in static IOT networks and the last IP digit is also used to define the mqtt name. Easier for debugging. 
So in a correct credential file only the one number in   #define myIP "100"   is enough to compile another device.
