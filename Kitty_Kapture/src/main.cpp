#include <Arduino.h>
#include <HX711.h> //load cell library
#include <WiFi.h>   //wifi library
#include <Callmebot_ESP32.h> //whatsapp api

const char* ssid = "COX_Highland_2.4";
const char* password = "waterymint250";

String phoneNumber = "16194367150";
String apiKey = "2624068";
String messsage = "NEW ALERT : https://kitty-kam.web.app";

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN 15
#define LOADCELL_SCK_PIN  33

//button wiring
#define BUTTON_PIN 17

//camera alert
#define CAMERA_PIN 13

//is_armed LED
#define LED_PIN 27

float prev_weight;
bool is_active;

unsigned long camera_timer;
unsigned long button_timer;

//these variables allow the cam to work on startup
//without them camera_timer would have no reference point
bool is_initial_boot;
bool is_initial_picture;

HX711 scale;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //initialize pins
  pinMode(BUTTON_PIN,INPUT_PULLDOWN);
  pinMode(CAMERA_PIN,OUTPUT);
  pinMode(LED_PIN,OUTPUT);

  Serial.println("Initializing the scale");
  // Initialize scale with data output pin, clock input pin
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // this value is obtained by calibrating the scale with known weights
  //though it is imprecise it won't matter in our case because we only
  //care about the change in weight
  scale.set_scale(2280.f);    
  scale.tare();	
  prev_weight=0.0;

  //scale starts inactive to allow owner to place food
  is_active=false;

  camera_timer=millis(); //initialize camera timer
  button_timer=millis(); //initialize button timer

  //see decleration for explanation
  is_initial_boot=true;
  is_initial_picture=true;
  
  Serial.println("Scale Initialized, currently inactive");
}

void loop() {

  float curr_weight=scale.get_units(1);  //check current weight

  //check if button is pressed and camera hasnt been triggered in the last 3 minutes
  if (digitalRead(BUTTON_PIN)==HIGH and( (millis()>button_timer) or (is_initial_boot==true))){
    is_initial_boot=false;
    Serial.println("pressed button");

    //ensures one button press => one state change
    button_timer=millis()+700; 

    //if button is pressed and currently active enter inactive state
    if (is_active==true){
      is_active=false;
      digitalWrite(LED_PIN,LOW);
      Serial.println("entering inactive state");
    }

    //if button is pressed and currently inactive enter active state
    else{
      is_active=true;
      digitalWrite(LED_PIN,HIGH);
      Serial.println("entering active state");
      curr_weight=scale.get_units(1); //resetting weight
      prev_weight=curr_weight;
      is_initial_picture=true;
    }
  }


  //scale measures difference in weight with such precision
  //and speed that constanly updating the weight to account
  //for food drying and becoming lighter makes for
  //a better product
  if (millis()<camera_timer){
    prev_weight=curr_weight;
  }


  //only see if picture needs to be taken if camera has not been triggered in the last 3 minutes
  if (is_active==true and (millis()>camera_timer or is_initial_picture==true)){

    //if weight has changed by more than 0.2 grams take picture
      if(abs(curr_weight-prev_weight)> 0.3){
        is_initial_picture=false;
        Serial.println("Taking picture");
        camera_timer=millis()+60000; //update camera timer
        sleep(2);
        //update previous weight
        prev_weight=curr_weight;
        
        //send signal to esp32-cam to take picture
        digitalWrite(CAMERA_PIN,HIGH);
        delay(1000);
        digitalWrite(CAMERA_PIN,LOW);
        whatsappMessage(phoneNumber, apiKey, messsage);
  } 
  }
}