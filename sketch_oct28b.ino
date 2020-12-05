    #define NOTE_C1 33
    #include <SoftwareSerial.h>
    #include <LowPower.h>
      
    SoftwareSerial sim800l(11, 12);
    const int analogInPin = A0;
    int Trig = 9;   // Trigger pin
    int Echo = 10;   // Response pin
    
    long EchoTime;  // Echo time duration
    int  Distance;  // Distance in cm
    int  MaximumRange = 500; // Max distance
    int  MinimumRange = 20;   // Min distance

    int BatteryValue = 0;
    float Voltage = 0;
    long Delay = 24 * 60 * 60L * 1000; // 24 hours in miliseconds

    void setup() {
      Serial.begin(9600);
      while(!Serial);
      Serial.println("Arduino with SIM800L is ready");
      pinMode(Trig, OUTPUT);
      pinMode(Echo, INPUT);

      sim800l.begin(9600);
      delay(1000);
    }

    void SendSMS(int distance, float voltage) {
      Serial.println("Sending SMS...");
      sim800l.print("AT+CMGF=1\r"); //Sets the module to SMS mode
      delay(100);
      sim800l.print("AT+CMGS=\"+123123123\"\r");
      delay(500);
      sim800l.print("Odczyt gowna: " + String(distance) + "cm"); // Msg
      delay(500);
      sim800l.print((char)26);// (required according to the doc)
      delay(500);
      sim800l.println();
      Serial.println("Text Sent.");
      delay(10000);
    }
     
    void loop() {
      if (sim800l.available() ){ 
        Serial.write(sim800l.read()); 
      } else {
//        Serial.println("unavailable");
      }
      
      digitalWrite(Trig, LOW);
      delayMicroseconds(2);

      digitalWrite(Trig, HIGH);
      delayMicroseconds(10);

      digitalWrite(Trig, LOW);
      EchoTime = pulseIn(Echo, HIGH);

      Distance = EchoTime / 58;
      
      const int CLOSE = 21;
      const int AVG = 41;
      const int DISTANT = 300;
      
      if (Distance >= MaximumRange || Distance <= MinimumRange || Distance == 0) {
        Serial.println("Poza zakresem");
      } else  {
        Serial.println(String(Distance) + " cm");
        if (Distance <= CLOSE) {
            BatteryValue = analogRead(analogInPin);
            Voltage = (float(BatteryValue)*5)/1023*2;
          SendSMS(Distance, Voltage);
        }
      }

      delay(Delay);
    }
