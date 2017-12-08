#include <Wire.h>
#include <stdint.h>

#define EMPTY 0                                             //Slave is empty
#define WORK 1                                              //Slave is working
#define FULL 2                                              //Slave is full
#define FINISHED 3                                          //Slave is finished

#define LED 11                                              //Led diode

long x = -1;                                                //Start of the range
long y = -1;                                                //End of the range

int slaveStatus = EMPTY;                                    //Current slave status

volatile int primes[100];                                            //All prime numbers that can be stored by slave
volatile int counter = 0;                                            //How many prime numbers are actually stored
volatile int sentPrimes = 0;
int index = 0;                                              //Current number
int totalPrimes = 0;

volatile int ledStrength = 0;
//
//bool up = true;


void setup() {
  Wire.begin(9); // join i2c bus with address #9
//  pinMode(LED, OUTPUT);
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
  Serial.println("Started slave on port 9.");
  
//  noInterrupts();               // Gasimo prekide
//
//  TCCR1A = 0;                   // Stavljamo TCCR1A na 0
//  TCCR1B = 0;                   // Stavljamo TCCR1B na 0
//  TCNT1  = 0;                   // Counter na 0


//  8 sekundi ciklus - paljenje + gasenje (0%)
//  6 sekundi ciklus - paljenje + gasenje (30%)
//  4 sekunde ciklus - paljenje + gasenje (50%)
//  2 sekunde ciklus - paljenje + gasenje (70%)
//  2Hz - 500ms (2 puta u sekundi)
//  4Hz - 1000ms(1 put u sekundi)
//  20Hz - 50ms (20 puta u sekundi)
//  200Hz - 5ms (200 puta u sekundi)
//  MaxLedStrength = 100 -> za 200Hz je diodi potrebna jedna sekunda da se upali i ugasi
  
//  OCR1A = 10000;                // 16000000/(200Hz*8) //-1999?
//  TCCR1B |= (1 << WGM12);
//  TCCR1B |= (1 << CS11);        // Prescaler na 8
//  TIMSK1 |= (1 << OCIE1A);
//
//  interrupts();

}

//ISR(TIMER1_COMPA_vect) {
//  if (led_counter > led_granica)
//    led_counter = led_granica;
//  if (led_counter == led_granica) {
//    if (brightness > 101)
//      dir = -1;
//    if (brightness < 1)
//      dir = 1;
//    brightness += dir;
//    analogWrite(LED_PIN, brightness);
//    led_counter = 0;
//  }
//  led_counter ++;
//}

void loop() {

//  if(up) {
//    ledStrength++;
//  }
//  else {
//    ledStrength--;
//  }
//  analogWrite(LED, ledStrength);
//  if(ledStrength == 100) {
//    up = false;
//  }
//  else if(ledStrength == 0) {
//    up = true;
//  }
  
  if (slaveStatus == WORK) {
    //    Serial.print("WORK ");
    if (isPrime(index) && counter < 199 && index <= y) {
      primes[counter++] = index;
      totalPrimes++;
      //      Serial.println(index);
    }
    else if (counter == 199) {
      slaveStatus = FULL;
    }
//    if (index > y + 100) {
//      slaveStatus = EMPTY;
//    }
    if (sentPrimes == totalPrimes && sentPrimes != 0 && index > y) {
      slaveStatus = EMPTY;
    }
    if (index == y) {
      Serial.println("FINISHED");
    }
    index++;
  }
  if (slaveStatus == FULL) {
    slaveStatus = EMPTY;
    memset(primes, 0, sizeof(primes));
  }
  delay(20); //20
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  String message = String(slaveStatus);
  Wire.write(message.c_str());
  Serial.println("REQUEST");
  Serial.print("\t");
  Serial.println(message);

  if (slaveStatus == EMPTY) {

  }
  else if (slaveStatus == WORK) {
    byte byteArray[4];

    Serial.print("Counter: ");
    Serial.println(counter);
    byteArray[0] = (counter >> 8) & 0xFF;
    byteArray[1] = counter & 0xFF;

    Wire.write(byteArray[0]);
    Wire.write(byteArray[1]);

    Serial.print("sent primes");
    Serial.println(sentPrimes);
    if (counter <= 6) {
      for (int i = 0; i < counter; i++) {
        Serial.print("Prime: ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(primes[i]);

        memset(byteArray, 0, sizeof(byteArray));
        byteArray[0] = (primes[i] >> 24) & 0xFF;
        byteArray[1] = (primes[i] >> 16) & 0xFF;
        byteArray[2] = (primes[i] >> 8) & 0XFF;
        byteArray[3] = (primes[i] & 0XFF);

        Wire.write(byteArray[0]);
        Wire.write(byteArray[1]);
        Wire.write(byteArray[2]);
        Wire.write(byteArray[3]);

      }
      sentPrimes += counter;
      counter = 0;
    }
  }
  else if (slaveStatus == FULL) {

  }
  Serial.print("Total primes: ");
  Serial.println(totalPrimes - 1);
}

void receiveEvent(int howMany) {
  Serial.println("RECEIVE");
  if (slaveStatus == EMPTY) {

    String str;
    while (Wire.available()) {
      char c = Wire.read();
      str += c;
    }

    x = str.substring(0, str.indexOf(' ')).toInt();
    y = str.substring(str.indexOf(' ') + 1, str.length()).toInt();

    Serial.print(x);
    Serial.print(" ");
    Serial.println(y);

    slaveStatus = WORK;
    index = x;
  }
  else if (slaveStatus == WORK) {

  }
  Serial.print("Slave status in receiveEvent: ");
  Serial.println(slaveStatus);

}

bool isPrime(int i) {
  bool res = true;
  if(i == 1) {
    return !res;
  }
  if(i == 2) {
    return res;
  }
  for (int x = 2; x <= sqrt(i) + 1; x++) {
    if (i % x == 0) {
      res = false;
      break;
    }
  }
  return res;
}

