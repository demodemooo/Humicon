#include <M5StickC.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <math.h>

#include <UNIT_ENV.h>

SHT3X sht30;
//QMP6988 qmp6988;

BluetoothSerial SerialBT;

hw_timer_t * timer = NULL;

int mode = 0;
int count = 0;
int last_count = 0;

int status = 0;

void IRAM_ATTR onTimer() {
  // update data
  count++;
//  count = count % 25;   //1Hz
  count = count % 60;     // 1min
}

float temp = 0.0;
float hum = 0.0;

float hum_1min[60];

float hum_1minAVG(float hum){
  float hum_total = 0.0;
  hum_1min[count] = hum;

  for(int i=0; i<60; i++){
    hum_total += hum_1min[i];
  }

  return hum_total / 60.f;

}

void M5drawing(float temp, float hum){

  switch(status){

    // 0-1: ALART(BLINK)
    case 0:
      M5.Lcd.fillScreen(TFT_RED);
      M5.Lcd.setTextColor(BLACK,RED);
  //    status = 1;
    break;

    case 1:
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(WHITE, BLACK);
 //     status = 0;
    break;

    case 2:
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(WHITE, BLACK);
    break;
  }

  M5.Lcd.setCursor(10, 10); //set the cursor location.
  M5.Lcd.println("Humitest");

  M5.Lcd.setCursor(10, 30);
  M5.Lcd.printf("Temp: %3.2f*C", temp);

  M5.Lcd.setCursor(10, 50);
  M5.Lcd.printf("Humid: %3.2f%%", hum);
 
}

void setup() {
  M5.begin();  //Init M5StickC Plus.
  Serial.begin(115200);
  SerialBT.begin("ESP32_stack");
  
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  
  timer = timerBegin(0, getApbFrequency() / 1000000, true); // 1us
  timerAttachInterrupt(timer, &onTimer, true);
//  timerAlarmWrite(timer, 40000, true);           //40000 = 40ms 25Hz
  timerAlarmWrite(timer, 1000000, true);         // 1Hz
  timerAlarmEnable(timer);

  count = 0;
  last_count = 0;
  status = 0;

  Wire.begin(0,26);                     //Init I2C for Humid sensor
 // qmp6988.init();

  M5.Lcd.setRotation(1);  //Rotate the screen.
  M5.Lcd.setTextFont(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 10); //set the cursor location.
  M5.Lcd.println("Humitest");

  // waiting...
/*  M5.Lcd.setCursor(10, 30);
  M5.Lcd.println("Internal initialize...");
  while(count<58){

  }
*/
 }

void loop() {
  M5.update(); 
  
  Serial.printf("%d\n",status);

  if(sht30.get()==0){
    temp = sht30.cTemp;
    hum = sht30.humidity;
  }

//  Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", sht30.cTemp, sht30.humidity, qmp6988.calcPressure());
 // delay(1000);


 if (M5.BtnA.wasReleased()) {
   status = 2;
  }

  if (M5.BtnB.wasReleased() ) {

  }
   

  if(count != last_count){

    if(hum < 30){
       status++;
       status = status % 2;
    }else{
      status = 2;
    }

    M5drawing(temp, hum);
    Serial.printf("avg:%2.2f\n",hum_1minAVG(hum));
  }

  last_count = count;
}

