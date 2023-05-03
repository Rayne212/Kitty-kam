
#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include <SPIFFS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <ESPDateTime.h>


//Replace with your network credentials
const char* ssid = "COX_Highland_2.4";
const char* password = "waterymint250";

//Firebase API Key
#define API_KEY "AIzaSyDnaYNw6ZWcFHE073cN7cEokM54MVdWAlg"

// User Auth
#define USER_EMAIL "clubrydawg@gmail.com"
#define USER_PASSWORD "Rydawglildawg1!"

// Storage Bucket Name
#define STORAGE_BUCKET_ID "kitty-kam.appspot.com"


// OV2640 camera module pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//input pin for camera signal
#define CAM_SIGNAL 13

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;
bool taskCompleted = false;


void initCamera(){
 //Camera configurations
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 20;
  config.fb_count = 1;
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  //initiate wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  //initialize camera
  initCamera();


  //conencting to firebase and signing in
 
  configF.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.token_status_callback = tokenStatusCallback;

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
  
  //connncting to time server
  DateTime.setServer("north-america.pool.ntp.org");
  DateTime.setTimeZone("PST8PDT,M3.2.0,M11.1.0");
  DateTime.begin();
    while (!DateTime.isTimeValid()) {
    DateTime.begin();
  } 

  
 pinMode(CAM_SIGNAL,INPUT);
}

void loop() {
  //only take picture and send if input is high (recieved from esp32)
  if (digitalRead(CAM_SIGNAL) == HIGH) {

    //camera frame buffer
    camera_fb_t * fb = NULL; 
 
   
    Serial.println("Taking a photo...");
   // Take a photo with the camera
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
    }

    //open file and write the photo to it
    File file = SPIFFS.open( "/data/pic.jpg", FILE_WRITE);
    file.write(fb->buf, fb->len);

    // Close the file
    file.close();
    esp_camera_fb_return(fb);
    delay(1);

  if (Firebase.ready()){
    Serial.print("Uploading picture... ");
    // Upload the photo to most recent photo location in Firebase Storage
    //this allows website to display most recent photo
    while (!Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID,  "/data/pic.jpg" , mem_storage_type_flash , "/mostRecent/photo.jpg" , "image/jpg" )){
      Serial.println(fbdo.errorReason());}
    
 
    //upload photo to permenant storage in firebase
    while (!Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID,  "/data/pic.jpg", mem_storage_type_flash , DateTime.toString() , "image/jpg")){
      Serial.println(fbdo.errorReason());}
    

  }
}
}
