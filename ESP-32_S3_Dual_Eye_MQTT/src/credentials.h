/*Made by Mike Eitel to store device credenitals on one place
  No rights reserved when private use. Otherwise contact author.

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files. The above copyright notice and this permission notice shall 
  be included in all copies or substantial portions of the software.
*/

// Wifi acess definitions of network to connect to  
#define wifi_ssid "xxx"
#define wifi_password "yyy"

#define MyIP "091"                                      // Device ID

// Wifi definitions of this device  
//IPAddress staticIP(192,168,x,atoi(myIP));             // IOT device IP
IPAddress staticIP(1,1,1,atoi(MyIP));       // REPLACE !!!!
IPAddress subnet(255,255,255,0);                       // Network subnet size
//IPAddress gateway(192,168,x,y);                      // Network router IP
IPAddress gateway(1,1,1,1);                // REPLACE !!!!

// Mosquitto MQTT Broker definitions
#define mqtt_server    "192.168.x.z"                           // IOT MQTT server IP
#define mqtt_user      "admin"
#define mqtt_password  "admin"
#define mqtt_port      1883
#define WiFi_timeout    101                             // How many times to try before give up
#define mqtt_timeout     11                             // How many times to try before try Wifi reconnect

// MQTT Topics
#define mytype          "esp/S3-EYE-"                   // Client Typ
#define iamclient       mytype MyIP                     // Client name 
#define in_topic        iamclient "/command"            // This common input is received from MQTT
#define out_param       iamclient "/signal"             // Wifi signal strength is send to MQTT
#define out_status      iamclient "/status"             // This is a general message send to MQTT
#define out_watchdog    iamclient "/watchdog"           // A watchdog bit send to MQTT
#define out_sensors     iamclient "/sensors"            // This is a list of usable external sensors send to MQTT
#define out_topic       iamclient "/loop"               // This helper debug variable can be send to MQTT
#define mqtt_debug      iamclient "/debug"              // This is send to MQTT   Debug only
#define mqtt_out_hum0   iamclient "/eye/0"              // This is send to MQTT

// Errors send as values in test mode
// error =  -7    
// error =  -6    
// error =  -5    
// error =  -4    
// error =  -3    
// error =  -2    
// error =  -1    Wrong command received
// error =   0    Normal status
// error =   1    MQTT first time connected
// error =   2    MQTT Reconnect succesfull
// error =   3    Command received

// Constant how often the mqtt message is send
// Constant how often the mqtt message is send
#if defined(TEST)
  long readinterval =  3000;                            // Interval at which device does MQTT reading
  long sendinterval =  6000;                            // Interval at which sensor data is send via mqtt
#else
  long readinterval =  3000; //30000;                            // Interval at which to publish sensor readings
  long sendinterval = 60000; //449000;                           // Interval at which sensor data is send via mqtt 
#endif