#include <math.h>
#include <LiquidCrystal_PCF8574.h>

#define TRUE 1
#define FALSE 0
#define TIME_TO_TIMEOUT 1800000
#define PIR 9
#define MIC 7
#define HEATER 11
#define FAN 6
#define INCREMENT 2.55
#define LED_PIN 8


LiquidCrystal_PCF8574 lcd(0x27);
int LCD_page = 0;             // This variable indicates which page the LCD is currently showing.
const int B = 4275;           // B value of the thermistor
const int R0 = 100000;        // R0 = 100k
const int pinTempSensor = A0; // Grove - Temperature Sensor connect to A0

unsigned long t = 0;
unsigned long t_0 = 0;
unsigned long t_1 = 0;
unsigned long t_clap = 0;
unsigned long t_LCD = 0;
volatile unsigned long t_MIC = 0;    //these variables are needed only in the separate timers scenario
volatile unsigned long t_PIR = 0;   //these variables are needed only in the separate timers scenario

float temperature = -274;

bool start = false;
int temp_max_fan_P = 30;  //default values fan_temps with people:  max 30 e min 25, will be OVERWRITTEN IN SETUP
int temp_min_fan_P = 25;
int temp_max_fan = 35;    //default values fan_temps without people:  max 35 e min 28, will be OVERWRITTEN IN SETUP
int temp_min_fan = 28;

int temp_min_heat_P = 18;  //default values heater_temps with people:  max 22 e min 18, will be OVERWRITTEN IN SETUP
int temp_max_heat_P = 22;
int temp_min_heat = 10;    //default values heater_temps without people:  max 20 e min 10, will be OVERWRITTEN IN SETUP
int temp_max_heat = 20;

int Loud_Noise = 0;      
int Clap = 0;
int clap_flag = 0;
int flag_mic = 0;
int flag_pir = 0;
int loudSoundReset = 0;

float mic_change_clap=0;
float mic_change = 0;   
float potHeat = 0;
float potFan = 0;

volatile bool human_present = false;
volatile int mic_count = 0;
volatile int mic_count_clap=0;


void setup()
{
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Lab 2_1 Starting");
    readTemperature(); // Measure temperature
    setupConfig();                        //choice for the user on how the AC and Heater should work max and min temperatures
  
    pinMode(MIC, INPUT);
    pinMode(PIR, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(HEATER, OUTPUT);
    pinMode(FAN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
  
    lcd.begin(16, 2);
    lcd.setBacklight(255);
  
    attachInterrupt(digitalPinToInterrupt(MIC), soundDetection, LOW);
}

void loop()
{
    delay(10);     //reduce amount of loops
    if (start == false)
    {
        t_0 = millis();               //initial timestamp
        t = t_0;
        t_LCD = t_0;
        t_clap = t_0;
        start = true;
        acSystem();
    }
    serialCommunication(); // Causes delay.
  
    t_1 = millis();        //updates the timestamp at the start of operations
    clapclap();
    if (human_present == false) //da disattivare per void clapclap
    {
        pirDetection();
        if ((t_1 - t_0) >= 10000)
        {
            mic_change = (((float)100 * mic_count) / (float) (t_1 - t_0));
            //Serial.print("Human: ");       Serial.println(human_present);
            Serial.print("mic_count: ");   Serial.println(mic_count);
            Serial.print("mic_change: ");  Serial.println(mic_change);
            mic_count = 0;
            if (mic_change > 5) //Loud sound if change > 5
            {
              Loud_Noise++;
              Serial.print("Loud noise = ");
              Serial.println(Loud_Noise);
              // if (flag_mic == 1)                    //must be controlled
              //{
              //  t_MIC = millis();                  //collects timestamp of last human detection with Mic after human is already in the house
              //}
              if (Loud_Noise >= 10)                    //human is present
              {
                  Loud_Noise = 0;
                  human_present = true;
                  flag_mic = 1;
                  // t_MIC = millis();                  //collects timestamp of last human detection with MIC (for separate timers)
                  Serial.println("human_present: TRUE");
                  readTemperature(); // Measure temperature
                  acSystem();        // Handle airconditioning ///
              }
          }

          if (loudSoundReset >= 1)   //responsible for reseting Loud sound variable
          {
              Serial.print("loudSoundReset = ");
              Serial.println(loudSoundReset);
              Loud_Noise = 0;
              loudSoundReset = 0;
          }
        t_0 = t_1;
        }
  }
    //if((t_1-t_MIC)>=(1.8e6))
    //{                      //compares timestamp now with timestamp when human detected with MIC
    //    timeout_MIC(t_1);
    //    if (flag_mic==0 && flag_pir==0)               //If both timers run out, there is no human in the house.
    //    {
    //        human_present = 0;
    //        Serial.println("human_present: FALSE");
    //    }
    //}

    //if((t_1-t_PIR)>=(3.6e6))
    //{                      //compares timestamp now with timestamp when human detected with PIR
    //    timeout_PIR(t_1);
    //    if (flag_mic==0 && flag_pir==0)               //If both timers run out, there is no human in the house.
    //    {
    //        human_present = 0;
    //        Serial.println("human_present: FALSE");
    //     }
    //}

    if (( (t_1 - t_0) >= (TIME_TO_TIMEOUT) ) && human_present == true)   //ogni 30 minuti
    {
        mic_count = 0;
        t_0 = t_1;
        human_present = false;
        Serial.println("human_present: TIMEOUT");
        readTemperature(); // Measure temperature
        acSystem();        // Handle airconditioning ///
    }

    if (t_1 - t >= (5e3))          //20 seconds for debug           //ogni 5 minuti 300e3
    {
        readTemperature(); // Measure temperature
        acSystem();        // Handle airconditioning ///
        t = millis();
        loudSoundReset++; //da disattivare per void clapclap
    }
    if (t_1 - t_LCD >= (5e3))                                    //ogni 5 secondi
    {
        writeToLCD(); //da aggiustare il refresh display
        t_LCD = t_1;
    }
}


void clapclap() 
{
    if ((t_1 - t_clap) >= 500)
    {
        mic_change_clap = (((float)5*mic_count_clap) / (float) (t_1 - t_clap));
        //Serial.print("mic_count_clap: ");   Serial.println(mic_count_clap);
        //Serial.print("mic_change_clap: ");  Serial.println(mic_change_clap);
        mic_count_clap = 0;
        if (mic_change_clap > 10)
        {
            Clap++;
            Serial.print("Clap = ");
            Serial.println(Clap);
            if (Clap >= 2) 
            {
                loudSoundReset = 0;
                Clap = 0;
                if (clap_flag == 0)
                {
                    digitalWrite(LED_PIN, HIGH);  //SWITCH LIGHT ON/OFF WITH DOUBLE CLAP
                    clap_flag = 1;
                }
                else
                {
                    digitalWrite(LED_PIN, LOW);  //SWITCH LIGHT ON/OFF WITH DOUBLE CLAP
                    clap_flag = 0;
                }
            }
        }
        else Clap =0;
        t_clap = t_1;
    }
}


int checkDati(int dato) 
{
    if (dato > 45 || dato < -25) 
    {
        Serial.println(dato);
        Serial.println("sei sicuro che il dato che vuoi inserire sia corretto? per sicurezza reinseriscilo");
        while (!Serial.available());
        dato = Serial.parseInt();
    }
    return dato;
}

void setupConfig() 
{
    //setup
    int rd;
    Serial.println("Inserisci range temperatura di funzionamento ventola in presenza di persone");
    Serial.print("temp max :") ;
  
    while (!Serial.available());
    rd = checkDati(Serial.parseInt());
    temp_max_fan_P = rd;
    Serial.println(rd);
  
    Serial.print("temp min :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_min_fan_P = rd;
    Serial.println(rd);
  
    Serial.println("Inserisci range temperatura di funzionamento ventola in assenza di persone");
    Serial.print("temp max :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_max_fan = rd;
    Serial.println(rd);
  
    Serial.print("temp min :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_min_fan = rd;
    Serial.println(rd);
  
  
    Serial.println("Inserisci range temperatura di funzionamento riscaldamento in presenza di persone");
    Serial.print("temp max :") ;
    while (!Serial.available());
    rd = checkDati(Serial.parseInt());
    temp_max_heat_P = rd;
    Serial.println(rd);
  
    Serial.print("temp min :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_min_heat_P = rd;
    Serial.println(rd);
  
  
    Serial.println("Inserisci range temperatura di funzionamento riscaldamento in assenza di persone");
    Serial.print("temp max :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_max_heat = rd;
    Serial.println(rd);
  
    Serial.print("temp min :");
    while (!Serial.available());
  
    rd = checkDati(Serial.parseInt());
    temp_min_heat = rd;
    Serial.println(rd);
  
  
    Serial.println("Configurazione completata :)");
    Serial.print("Per modificare i range temperatura inserisci :\n 0 per ventola in presenza di persone \n 1 per ventola in assenza di persone \n ");
    Serial.print("2 per riscaldamento in presenza di persone \n 3 per riscaldamento in assenza di persone\n  ");
    return;
}

void serialCommunication()
{

    if (Serial.available() > 0)
    {
        int rd = checkDati(Serial.parseInt());
        Serial.println(rd);
        switch (rd)
        {
            case 0:
                Serial.println("Inserisci range temperatura di funzionamento ventola in presenza di persone");
                Serial.print("temp max :") ;
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_max_fan_P = rd;
                Serial.println(rd);
        
                Serial.print("temp min :");
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_min_fan_P = rd;
                Serial.println(rd);
        
                break;
            case 1:
                Serial.println("Inserisci range temperatura di funzionamento ventola in assenza di persone");
                Serial.print("temp max :");
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_max_fan = rd;
                Serial.println(rd);
        
                Serial.print("temp min :") ;
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_min_fan = rd;
                Serial.println(rd);
        
                break;
            case 2:
                Serial.println("Inserisci range temperatura di funzionamento riscaldamento in presenza di persone");
                Serial.print("temp max :") ;
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_max_heat_P = rd;
                Serial.println(rd);
        
                Serial.print("temp min :") ;
                while (!Serial.available());
        
                rd = checkDati(Serial.parseInt());
                temp_min_heat_P = rd;
                Serial.println(rd);
      
                break;
            case 3:
      
                Serial.println("Inserisci range temperatura di funzionamento riscaldamento in assenza di persone");
                Serial.print("temp max :");
        
                while (!Serial.available());
                rd = checkDati(Serial.parseInt());
                temp_max_heat = rd;
                Serial.println(rd);
        
                Serial.print("temp min :");
                while (!Serial.available());
                rd = checkDati(Serial.parseInt());
                temp_min_heat = rd;
                Serial.println(rd);
      
                break;
      
            default:
                Serial.println("Comando inserito errato");
                break;
        }
        Serial.print("Per modificare i range temperatura inserisci :\n 0 per ventola in presenza di persone \n 1 per ventola in assenza di persone \n ");
        Serial.print("2 per riscaldamento in presenza di persone \n 3 per riscaldamento in assenza di persone\n  ");
        return;
    }
    return;
}

void acSystem() 
{
    if (human_present == true)
    {
        if (temperature >= temp_min_fan_P)
        {
          if (temperature >= temp_max_fan_P)                 //case for which temperature is more than the maximum
          {
            potFan = 1;
            coolingSystem(potFan); // Passing a percentage to the cooling system
          }
          else
          {
            potFan = (temperature - temp_min_fan_P) / (temp_max_fan_P - temp_min_fan_P);
              coolingSystem(potFan); // Passing a percentage to the cooling system
          }        
        }

        else if (temperature <= temp_max_heat_P)
        {
          if (temperature <= temp_min_heat_P)                    //case for which temperature is less than the minimum
          {
            potHeat = 1;
            heatingSystem(potHeat); // Passing a percentage to the cooling system
          }
          else
          {
              potHeat = 1  - ((temperature - temp_min_heat_P) / (temp_max_heat_P - temp_min_heat_P));
              //Serial.println("pot Heat with people: ");    Serial.println(potHeat); 
              heatingSystem(potHeat);
          }
        }
    }

    else
    {
        if (temperature >= temp_min_fan)
        {
          if (temperature >= temp_max_fan)                 //case for which temperature is more than the maximum
          {
            potFan = 1;
            coolingSystem(potFan); // Passing a percentage to the cooling system
          }
          else
          {
              potFan = (temperature - temp_min_fan) / (temp_max_fan - temp_min_fan);
              coolingSystem(potFan); // Passing a percentage to the cooling system
          }

        }

        else if (temperature <= temp_max_heat)
        {
          if (temperature <= temp_min_heat)                    //case for which temperature is less than the minimum
          {
            potHeat = 1;
            heatingSystem(potHeat); // Passing a percentage to the cooling system
          }
          else
          {
              potHeat = 1  - ((temperature - temp_min_heat) / (temp_max_heat_P - temp_min_heat_P));
              //Serial.println("pot Heat: ");    Serial.println(potHeat); 
              heatingSystem(potHeat);
          }
        }
    }
}

void pirDetection() // detect inside loop - DONE
{
    if (digitalRead(PIR) == HIGH)
    {
        t_PIR = millis();                  //collects timestamp of last human detection
        flag_pir = 1;
        if (human_present == false)
        {
            human_present = true;
            Serial.println("human_present: TRUE PIR");
            t_1 = millis();
            readTemperature(); // Measure temperature
            acSystem();        // Handle airconditioning ///
        }
    }
}


void soundDetection()
{
    mic_count++;                // make interrupt as short as possible
    mic_count_clap++;
}


void timeout_MIC(unsigned long timer) // When no one is detected for more than TIME_TO_TIMEOUT then no human is present - DONE
{
    Serial.println("MIC DETECTION: FALSE");
    t_MIC = timer;
    flag_mic = 0;
}
void timeout_PIR(unsigned long timer) // When no one is detected for more than TIME_TO_TIMEOUT then no human is present - DONE
{
    Serial.println("PIR DETECTION: FALSE");
    t_PIR = timer;
    flag_pir = 0;
}


void readTemperature() // - DONE
{
    int a = analogRead(pinTempSensor);
    float R = 1023.0 / a - 1.0;
    R = R0 * R;
    temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
    return;
}

void heatingSystem(float level) // - DONE
{
    analogWrite(HEATER, level * 255);
    Serial.print(" HEATER power: %"); Serial.println(level*100);                        
}

void coolingSystem(float level) // - DONE
{
    analogWrite(FAN, level * 255);
    Serial.print(" FAN power: %"); Serial.println(level*100);                        
}

void writeToLCD() // Da vedere se printa tutto correttamente. - DONE
{
    lcd.home();
    lcd.clear();
    if (LCD_page == 0)
    {
        //Format:
        // T:{temperature} Pres:{human_present}
        // AC:{fan_percent}% HT:{heat_percent}%
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(temperature);
        lcd.print(" Pres:");
        lcd.print((human_present) ? 'T' : 'F');
  
        lcd.setCursor(0, 1);
        lcd.print("AC:"); //Three spaces to go to the second line.
        lcd.print(potFan * 100);
        lcd.print("% HT:");
        lcd.print(potHeat * 100);
        LCD_page = 1;
    }
    else
    {
        // Format:
        // AC m:{set_point} M:{set_point}
        // HT m:{set_point} M:{set_point}
        lcd.setCursor(0, 0);
        lcd.print("AC m:");
        lcd.print((human_present) ? temp_min_fan_P : temp_min_fan); //
        lcd.print(" M:");
        lcd.print((human_present) ? temp_max_fan_P : temp_max_fan);
  
        lcd.setCursor(0, 1);
        lcd.print("HT m:");
        lcd.print((human_present) ? temp_min_heat_P : temp_min_heat);
        lcd.print(" M:");
        lcd.print((human_present) ? temp_max_heat_P : temp_max_heat);
        LCD_page = 0;
    }
}
