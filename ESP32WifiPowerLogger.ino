#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6dt6WDMoo"
#define BLYNK_TEMPLATE_NAME "Solar Monitor"
#define BLYNK_AUTH_TOKEN "OGpL-BYM3EvTps_cyp9FZF-ZdsXgY3Tw"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_ADS1X15.h>
#include <SPI.h>

Adafruit_ADS1115 ads;

char ssid[] = "LEGASPI";
char pass[] = "Jasper12345";

char auth[] = BLYNK_AUTH_TOKEN;



double ReadVoltage(byte pin){
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if(reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
  //outputs in Volts

}

void setup(void) {

  // Debug console
  Serial.begin(115200);


  //Pin is for External Control of AC output
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  //Initialize the Blynk library
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);


  //ADC initialization
  ads.setGain(GAIN_SIXTEEN);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

BLYNK_WRITE(V2){

    //Control for AC Output
    int pinValue = param.asInt();
    if(pinValue == 1 ){
        digitalWrite(15, HIGH);
    }
    else{
        digitalWrite(15, LOW);
    }
}

void loop(void) {
    
    //Multiplier for the ADC reading
    float multiplier = 0.0078125;

    //Data Measurements and Processing
    double batVoltageValue = ReadVoltage(36) * 10;
    int16_t chargeCurrentReading = ads.readADC_Differential_2_3();
    int16_t dischargeCurrentReading = ads.readADC_Differential_0_1();

    double chargeCurrentValue = abs(chargeCurrentReading) * multiplier / 1.5;
    double dischargeCurrentValue = abs(dischargeCurrentReading) * multiplier / 1.5;

    //Kinda useless? was used to avoid negative values but gi absolute naman nako so, ya, useless
    if (chargeCurrentValue < 0 || chargeCurrentValue > 30){
        chargeCurrentValue = 0;
        }
    if (dischargeCurrentValue < 0){
        dischargeCurrentValue = 0;
        }    
    double chargePowerValue = batVoltageValue * chargeCurrentValue;
    double dischargePowerValue = batVoltageValue * dischargeCurrentValue;
    
    //Could have included more processes for total power harvested and consumed but we are limited to the amount of datastream we can send to blynk of which is 5. Can be easily implemented if blynk restrictions are circumvented
    
    // Send it to Blynk
    Blynk.run();
    Blynk.virtualWrite(V0, batVoltageValue);
    Blynk.virtualWrite(V1, chargeCurrentValue);
    Blynk.virtualWrite(V3, chargePowerValue);
    Blynk.virtualWrite(V4, dischargePowerValue);
    //AC control counts as 1 datastream

    
    //For console debugging
    Serial.printf("Battery Voltage = %f\n", batVoltageValue);
    Serial.printf("Battery Charge = %f\n", chargeCurrentValue);
    Serial.printf("Battery Disharge = %f\n", dischargeCurrentValue);
    Serial.printf("Charge Power = %f\n", chargePowerValue);
    Serial.printf("Disharge Power = %f\n", dischargePowerValue);
    
   delay(1000);
   //delay so blynk doesn't block our connection
}
