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

volatile long primes[200];                                            //All prime numbers that can be stored by slave
volatile int counter = 0;                                            //How many prime numbers are actually stored
volatile int sentPrimes = 0;
long index = 0;                                              //Current number
int totalPrimes = 0;
double percent = 1;                                           //LONG
double percentedNumber = 0;                                   //LONG

volatile int ledStrength = 0;
volatile byte ledSpeed = 8;
volatile byte ledCnt = 0;
volatile bool up = true;


void setup() {
  Wire.begin(8); // join i2c bus with address #8
  pinMode(LED, OUTPUT);
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
  Serial.println("Started slave on port 8.");

//  cli();               // Gasimo prekide
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

//  OCR1A = 10000;                // 16000000/(200Hz*8)
//  TCCR1B |= (1 << WGM12);
//  TCCR1B |= (1 << CS11);        // Prescaler na 8
//  TIMSK1 |= (1 << OCIE1A);
//
//  sei();

}

//ISR(TIMER1_COMPA_vect) {
//  Serial.println();
//  Serial.print("Led CNT: ");
//  Serial.println(ledCnt);
//  Serial.print("Led Speed: ");
//  Serial.println(ledSpeed);
//  Serial.print("Led strength: ");
//  Serial.println(ledStrength);
//  Serial.print("Up: ");
//  Serial.println(up);
//
//  if (slaveStatus == WORK) {
//    if (ledCnt > ledSpeed)
//      ledCnt = ledSpeed;
//    if (ledCnt == ledSpeed) {
//      if (ledStrength > 100) {
//        up = false;
//        ledStrength = 100;
//      }
//      if (ledStrength < 4) {
//        up = true;
//        ledStrength = 0;
//      }
//      ledCnt = 0;
//    }
//    if (up)
//      ledStrength++;
//    else
//      ledStrength--;
//    analogWrite(LED, ledStrength);
//    ledCnt++;
//  }
//}

void loop() {

  if (slaveStatus == WORK) {
    //    Serial.print("WORK ");
    percentedNumber++;
    //    Percentage handling
    percent = percentedNumber / (y - x + 1);                //Prvo pomnozi pa podeli - kada je LONG
    percent *= 100;
    if (percent < 1) {
      percent = 1;
    }
    if (percent > 100) {
      percent = 100;
    }
    if (percent > 30) {
      ledSpeed = 6;
    }
    if (percent > 50) {
      ledSpeed = 4;
    }
    if (percent > 70) {
      ledSpeed = 2;
    }
    //    Serial.println();
    //    Serial.print("Percented number: ");
    //    Serial.println(percentedNumber);
//    int temp = percent;
//    Serial.print("Percentage: ");
//    Serial.println(temp);
    
    if (percent >= 10.00 && percent <= 10.10 ||percent >= 20.00 && percent <= 20.10 || percent >= 30.00 && percent <= 30.10 
        || percent >= 40.00 && percent <= 40.10 || percent >= 50.00 && percent <= 50.10 || 
           percent >= 60.00 && percent <= 60.10 || percent >= 70.00 && percent <= 70.10 || 
           percent >= 80.00 && percent <= 80.10 || percent >= 90.00 && percent <= 90.10 || percent == 100.00) {
        Serial.print("Percentage: ");
        Serial.println((int)percent);
    }
    else {
      delay(5);
//      Serial.println((int)percent);
    }
    //    Serial.println();

    if (isPrime(index) && counter < 199 && index <= y) {
      primes[counter++] = index;
      totalPrimes++;
      //      Serial.println(index);
    }
    else if (counter == 99) { //199
      slaveStatus = FULL;
      //      totalPrimes = 0;
      percentedNumber = 0;
      ledSpeed = 8;
    }

    //    if (index > y + 100) {
    //      slaveStatus = EMPTY;
    //    }
    if (sentPrimes == totalPrimes && sentPrimes != 0 && index > y) {
      slaveStatus = EMPTY;
      percentedNumber = 0;
      percent = 0;
      ledSpeed = 8;
    }
    if (index == y) {
      Serial.println("FINISHED");
      //      percentedNumber = 0;
      //      percent = 0;
    }
    index++;
  }
  if (slaveStatus == FULL) {
    slaveStatus = EMPTY;
    memset(primes, 0, sizeof(primes));
  }

  //  delay(20); //20
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  String message = String(slaveStatus);
  Wire.write(message.c_str());
//  Serial.println("REQUEST");
//  Serial.print("\t");
//  Serial.println(message);

  if (slaveStatus == EMPTY) {

  }
  else if (slaveStatus == WORK) {
    byte byteArray[4];

    //    Serial.print("Counter: ");
    //    Serial.println(counter);
    byteArray[0] = (counter >> 8) & 0xFF;
    byteArray[1] = counter & 0xFF;

    Wire.write(byteArray[0]);
    Wire.write(byteArray[1]);

    //    Serial.print("sent primes");
    //    Serial.println(sentPrimes);
    if (counter <= 6) {
      for (int i = 0; i < counter; i++) {
        //        Serial.print("Prime: ");
        //        Serial.print(i);
        //        Serial.print(": ");
        //        Serial.println(primes[i]);

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
  //  Serial.print("Total primes: ");
  //  Serial.println(totalPrimes - 1);
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

bool isPrime(long i) {
  bool res = true;
  if (i == 1) {
    return !res;
  }
  if (i == 2) {
    return res;
  }
  for (long k = 2; k <= sqrt(i) + 1; k++) {
    if (i % k == 0) {
      res = false;
      break;
    }
  }
  return res;
}

