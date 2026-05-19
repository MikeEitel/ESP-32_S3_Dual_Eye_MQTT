# ESP-32_S3_Dual_Eye_MQTT
This is an augmented version of the thelastoutpostworkshop's Dual EYE waveshare 0.71 board with an XIAO-ESP32-S3

I dislike DHCP in static IOT networks and the last IP digit is also used to define the mqtt name. Easier for debugging. 
So in a correct credential file only the one number in   #define myIP "100"   is enough to compile another device.



Serial Printout on USB:
Status: eye= default_normal, mood= sleepy, approx 81 FPS
+
Message for [esp/S3-EYE-091/command] arrived = L:(3)->I0C
DeviceControll Set: c   0 3 0 0
Mode-0: Sleepy          Bkl-3: 1 1      Eyes-0: ON ON

Status: eye= default_normal, mood= sleepy, approx 80 FPS

MQTT example: 
<img width="984" height="695" alt="image" src="https://github.com/user-attachments/assets/bd297fa4-8201-4778-9adf-9c1eef82cc67" />


 
MQTT command decoding is made via ( ca 500 )
void callback(char* topic, uint8_t* payload, unsigned int length) {

Three mixed behavior changes 1: Mode / 2. Backlight / 3. Close Eye
From (594) via MQTT command Ixx.  xx is a two letter hex witch defines 4x 2 Bits. From low to high: First two change mode. Second Backlight. Third closes Eyes.
Command ? requests the device status.
Command U xx is low use in this device, it changes mqtt timings.
Command X restarts the device.
