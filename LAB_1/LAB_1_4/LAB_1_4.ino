#define NUM_STEPS 10

const int FAN_PIN = 5;
const float increment = 255/NUM_STEPS;

volatile float current_speed = 0;

void setup() 
{
  Serial.begin(9600);
  while(!Serial);
	pinMode(FAN_PIN, OUTPUT);
	analogWrite(FAN_PIN, (int) current_speed);
  Serial.println("Control fan speed with '+' and '-'"); 

}
void inputspeed()
{
    if (Serial.available()>0)
    {
      int inByte = Serial.read();
   
      if ((char)inByte == '+')
      {
        if (current_speed + increment <= 255)
        {
          current_speed += increment;
          Serial.print("Increasing speed:");
          Serial.println(current_speed);
        }
        else
        {
          Serial.println("MAXIMUM SPEED!!");     
        }
      }

      else if((char)inByte == '-')
      {
        if (current_speed - increment >= 0)
        {
          current_speed -= increment;
          Serial.print("Decreasing speed:");
          Serial.println(current_speed);
        }
        else //if ((int)current_speed == 0)
        {
          Serial.println("0 speed!!");     
        }
      }
    
      else
      {
        Serial.println("Wrong input!");
      }
   }
   delay(5);
}

void loop()
{
  inputspeed();
  analogWrite(FAN_PIN, int(current_speed));
}
