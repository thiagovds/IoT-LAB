#include <math.h>
#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27);

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0

void setup()
{
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Lab 1.6 Starting");
    lcd.begin(16,2);
    lcd.setBacklight(255);
}
 
void loop()
{
    int a = analogRead(pinTempSensor);

    
    float R = 1023.0/a-1.0;
    R = R0*R;
 
    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

    lcd.home();
    lcd.clear();
    lcd.print("Temperature:");
    lcd.print(temperature);
    
    Serial.print("temperature = ");
    Serial.println(temperature);
    delay(3000);

}
