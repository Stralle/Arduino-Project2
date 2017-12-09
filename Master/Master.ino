#include <Wire.h>

byte slaves[] = {8, 9};                             //All slaves
byte slaveNum = sizeof(slaves) / sizeof(byte);    //Number of slaves

String inputString;       //String that users inputs
String firstNum;          //First number parsed as string
String secondNum;         //Second number parsed as sstring

long minimumNumber = 0;   //Minimal number for given range
long maximumNumber = 0;   //Maxmial number for given range
long partNum = 0;         //Partial number for the range
long segment = 0;   //Number of prime numbers that every slave should return

long primeNumbers[200];   //Total prime numbers that can be held in the moment
byte index = 0;            //Index of current prime numbers in array

bool allEmpty = true;     //Are all slaves empty?
bool stringParsed = false;//is string Parsed?

void setup() { 
  Wire.begin();           // join i2c bus (address optional for master)
  Serial.begin(9600);     // start serial for output
  Serial.println("Master is started.");
  Serial.println("Enter range to receive prime numbers: PRIME X Y");
}

//Parsing the input string
void parseString() {
  inputString = Serial.readString();
//  Serial.println("Parse String");

  inputString = inputString.substring(inputString.indexOf(' ') + 1, inputString.length());
  firstNum = inputString.substring(0, inputString.indexOf(' '));

  inputString = inputString.substring(inputString.indexOf(' ') + 1, inputString.length());
  secondNum = inputString;

  minimumNumber = firstNum.toInt();
  maximumNumber = secondNum.toInt();
  if (maximumNumber < minimumNumber) {
    Serial.println("Y must be greater than X");
  }
  partNum = (maximumNumber - minimumNumber) / slaveNum;
  stringParsed = true;
}

// Count the range for numbers
void countRange() {
//  Serial.println("Count range");
  long i = minimumNumber;
  byte range = 180;
  if (i > 20000) {
    range = 150;
  }
  if (i > 50000) {
    range = 100;
  }
  i++;
  while (pi(i) - pi(minimumNumber) < range && i < maximumNumber) {
    i++;
  }
  partNum = i;
  segment = (partNum - minimumNumber) / slaveNum;
  while (segment + minimumNumber > maximumNumber) {
    segment--;
  }
}

// Pi funkcija za aproksimaciju kolicine prostih brojeva od 1 do n.
long pi(long n) {
  if (n == 1) return 0;
  else return n / (log(n) - 1);
}

// Swap
void swap(long *xp, long *yp)
{
  long temp = *xp;
  *xp = *yp;
  *yp = temp;
}

// Bubble sort
void sortArray()
{
  int i, j;
  for (i = 0; i < index; i++)

    // Last i elements are already in place
    for (j = 0; j < index - i - 1; j++)
      if (primeNumbers[j] > primeNumbers[j + 1])
        swap(&primeNumbers[j], &primeNumbers[j + 1]);
}

void printArray() {
  int n = sizeof(primeNumbers) / sizeof(long);
  //  Serial.println("PRINT ARRAY");
  for (int i = 0; i < index; i++) {
    Serial.print(primeNumbers[i]);
    if (i == index - 1) {
      Serial.println();
    }
    else {
      Serial.print(", ");
    }
    if (i % 10 == 0 && i != 0) {
      Serial.println();
    }
  }
}

void loop() {
  if (Serial.available()) {
    parseString();
  }
  if (allEmpty && stringParsed) {
    if (index > 0) {
      sortArray();
      printArray();
      memset(primeNumbers, 0, sizeof(primeNumbers));
      index = 0;
    }
    if (maximumNumber > minimumNumber) {
      Serial.println("Sending X and Y to slaves...");
      countRange();
      for (int i = 0; i < slaveNum; i++) {
        String temp = String(minimumNumber);
        temp += ' ';
        //        Serial.println(minimumNumber);
        minimumNumber += segment;
        //        Serial.println(minimumNumber);
        //        Serial.println(segment);
        if (minimumNumber - 1 == maximumNumber) {
          minimumNumber = maximumNumber;
        }
        temp += minimumNumber;
        minimumNumber += 1;
        Serial.print("Slave: ");
        Serial.println(slaves[i]);
        Serial.print("\t");
        Serial.println(temp);
        Serial.print("\t");
        Serial.println(segment);
        Wire.beginTransmission(slaves[i]); // transmit to device #i
        Wire.write(temp.c_str());
        Wire.endTransmission(); // stop transmitting
        Serial.println("End transmission");
//        delay(10); //100
      }
      allEmpty = false;
    }
    else {
      Serial.println("Everything is finished");
      stringParsed = false;
//      delay(10); //100
    }
  }
  else if (stringParsed) {
    allEmpty = true;
    if (index == 0) {
      Serial.println("Working...");
    }
    if (index == 199) {
      sortArray();
      printArray();
      memset(primeNumbers, 0, sizeof(primeNumbers));
      index = 0;
    }
    for (int i = 0; i < slaveNum; i++) {
      Wire.requestFrom(slaves[i], 32); // request 32 bytes from slave device
//            Serial.print("Receive from slave:");
//            Serial.println(slaves[i]);
      char c;
      if (Wire.available()) {
        c = Wire.read();
//                Serial.print("\tStatus: ");
//                Serial.println(c);
        //        Serial.println("\t-------------");
      }
      if (c == '0') {           //Ovde bi potencijalno trebalo da ide kradja poslova - ako je slejv prazan, tj zavrsio je, onda da se uzme pola preostalog posla slejva koji ima najvise
        //        int maxSlaveIndex = 0;
        //        int maxSlaveNums = 0;
        //        for (int j = 0; j < slaveNum; j++) {
        //          if (j != i) {
        //            // TODO: Ako je j!=i i ako je slaves[j].leftNums > maxSlaveNums -> maxSlaveNums = slaves[j].leftNums; maxSlaveIndex = j;
        //          }
        //        }
        //        // TODO: slaves[i] da dobije pola preostalog posla od slaves[j];
      }
      else if (c == '1') {      //Ako slejv radi, primi do sada obradjene brojeve i odstampaj ih
        unsigned int cnt = 0;
        cnt |= Wire.read();
        cnt <<= 8;
        cnt |= Wire.read();
        //        Serial.print("Counted numbers: ");
        //        Serial.println(cnt);
        if (cnt <= 6) {
          for (int i = 0; i < cnt; i++) {
            long num = 0;
            //          Serial.print("Num bef: ");
            //          Serial.println(num);
            num |= Wire.read();
            num <<= 8;
            num |= Wire.read();
            num <<= 8;
            num |= Wire.read();
            num <<= 8;
            num |= Wire.read();
            //            Serial.print("Num: ");
            //            Serial.println(num);
            primeNumbers[index] = num;
            //            Serial.print("Prime number at index ");
            //            Serial.print(index);
            //            Serial.println(" is: ");
            //            Serial.println(primeNumbers[index]);
            index = index + 1;
            //                        Serial.print("\t");
            //                        Serial.println(primeNumbers[index - 1]);
          }
        }
      }
      else if (c == '2') {      //Ako je pun - isprazni ga

      }
      bool t = c == '0' ? true : false;
      allEmpty = allEmpty && t;
//      delay(5);
      //      Serial.print("\t\t");
      //      Serial.println(allEmpty);
    }
    //    Serial.print("All empty: ");
    //    Serial.println(allEmpty);
    //    Serial.print("Index is: ");
    //    Serial.println(index);

    //    sortArray();
    //    printArray();
    //    memset(primeNumbers, 0, sizeof(primeNumbers));
    //    index = 0;
//    delay(20); //200
  }
  else {
    Serial.println("Enter 'PRIME X Y' to start finding prime numbers");
    while (!Serial.available());
  }
  //  if (index > 0) {
  //    sortArray();
  //    printArray();
  //    memset(primeNumbers, 0, sizeof(primeNumbers));
  //    index = 0;
  //  }
}
