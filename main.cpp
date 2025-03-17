#include <Wire.h>
#include <Adafruit_CAP1188.h>
#include <TFT_eSPI.h>
#include <SPI.h>


// Define I2C
#define I2C_SDA 21
#define I2C_SCL 22

//this to define the wired for the LED
#define LED_Red 25
#define LED_Yellow 26
#define LED_Green 27

// this is used for the buzzer
#define BUZZER 32
#define LEDC_RESOLUTION 8 
#define LEDC_CHANNEL 0
#define LEDC_TIMER   0
#define CAP1188_IRQ 17
#define BUZZER_FREQ  800

TFT_eSPI tft = TFT_eSPI();


// Initialize CAP1188
Adafruit_CAP1188 cap = Adafruit_CAP1188();

//made for the red light with the delays into consideration
//calculated the delays for 10 sec and the sound for it
void redlight(){
    digitalWrite(LED_Red, HIGH);
    for( int  i = 0; i < 20; i++){
        ledcWrite(LEDC_CHANNEL, 128);
        delay(250);
        ledcWrite(LEDC_CHANNEL, 0);
        delay(250);

    }
    digitalWrite(LED_Red, LOW);

}


//yellow light iwth its own delay values
void redyellowlight(){
    digitalWrite(LED_Yellow, HIGH);
    digitalWrite(LED_Red, HIGH);
    delay(2000);
    digitalWrite(LED_Yellow, LOW);
    digitalWrite(LED_Red, LOW);
}


void yellowlight(){
    digitalWrite(LED_Yellow, HIGH);
    delay(2000);
    digitalWrite(LED_Yellow, LOW);
}



//made like the switches for later usage
bool greenON = false;
bool redON = false;
bool touches = false;


void setup() {
    Serial.begin(9600);
    
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);

    // Check if CAP1188 is found
    if (!cap.begin(0x28)) {  // Use 0x29 if ADDR pin is tied to 3.3V
        Serial.println("CAP1188 not found!");
        while (1);
    }

    tft.init();
    tft.setRotation(1);  // Adjust orientation if needed
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(4);

    //setup the pins for the lights and buzzer
    pinMode(LED_Red, OUTPUT);
    pinMode(LED_Yellow, OUTPUT);
    pinMode(LED_Green, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    ledcSetup(LEDC_CHANNEL, BUZZER_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(BUZZER, LEDC_CHANNEL);


    //initailizing the first round cuz we start from red to yellow then green
    redlight();
    redyellowlight();

    digitalWrite(LED_Green, HIGH);    
    greenON = true;
    
}

unsigned long buzzerStartTime = 0;  //Set up for green timer,
bool buzzerActive = false;          //Using milis to avoid compications with delay
unsigned long greenStartTime = 0;
bool waitingToTurnOff = false;


void loop() {
    touches = false;    //Reset the CAP1188
    cap.writeRegister(0x27, 0x00);  //Reset the CAP1188
    uint8_t touched = cap.touched();    //Checks touches
    touches = (touched != 0);   //True / false if touched or not

    if (greenON) {  //If the green light is on 
        if (!buzzerActive) {
            buzzerStartTime = millis(); //Start tracking time for buzzer
            buzzerActive = true;    //Buzzer is "on"
        }

        if (buzzerActive) { //Handles the buzzer independently from main  thread
            unsigned long currentTime = millis();   //saves current millis
            if ((currentTime - buzzerStartTime) % 2000 < 500) { //Mod 2000 checks if under 500 then "beep"
                ledcWrite(LEDC_CHANNEL, 128); //Buzzer ON for 500ms
            } else {
                ledcWrite(LEDC_CHANNEL, 0);   //Buzzer OFF for 1500ms
            }
        }
    }

    if (touches && greenON && !waitingToTurnOff) {
        waitingToTurnOff = true;    //Bool to set up after "button press"
        greenStartTime = millis();  //Start the 5-second countdown
    }

    if (waitingToTurnOff && millis() - greenStartTime >= 5000) {    //If 5000 ms exceeded (turn off_)
        digitalWrite(LED_Green, LOW);
        greenON = false;
        ledcWrite(LEDC_CHANNEL, 0); //Stop the buzzer
        buzzerActive = false;
        waitingToTurnOff = false;

        yellowlight();  //Green -> Yellow
        redlight(); //Yellow -> red
        redyellowlight();  //Red -> yellow

        digitalWrite(LED_Green, HIGH);  //Turn on green
        greenON = true;
        cap.writeRegister(0x27, 0x00);  //Try to clear CAP1188 register
    }

    touches = false;    //try to clear touch register
}
