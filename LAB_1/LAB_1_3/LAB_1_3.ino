const int LED_PIN = 12;
const int PIR_PIN = 2;

volatile int tot_count = 0;

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Lab 1.3 Starting");
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE);
}

void checkPresence()
{
  if (digitalRead(PIR_PIN) == HIGH)
  {
    tot_count++;
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }
}

void loop()
{
  Serial.print("Number of people detected: ");
  Serial.println(tot_count);
  delay(30 * 1e03);
}
