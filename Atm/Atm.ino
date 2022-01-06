#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

#define RST_PIN     49
#define SS_PIN      53

void(* resetFunc) (void) = 0;

// Constants
const byte ROWS = 4;
const byte COLS = 3;

// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
 
MFRC522 mfrc522(SS_PIN, RST_PIN); // create MFRC522 instance
LiquidCrystal lcd(37, 38, 39, 40, 41, 42);
byte rowPins[ROWS] = {44, 45, 32, 33};
byte colPins[COLS] = {34, 35, 36};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
byte language[3] = {0, 0, 0};

String atmPIN = "";
byte noMistakes = 0;
String updateVal = "";

// LCD variables:
int contrast = 0;
volatile byte screenNo = 0;

// RFID variables:
byte firstName[18];
byte firstNameLength = 18;
byte block1;
String strFirstName;

byte pinRFID[18];
byte pinSize = 18;
byte block2;
int intPin;

byte balance[18];
byte balanceSize = 18;
byte block3;
volatile int intValue;

// DC Motor variables:
// A
int enA = 5;
int in1 = 22;
int in2 = 23;
// B
int enB = 6;
int in3 = 24;
int in4 = 25;
// C
int enC = 7;
int in5 = 27;
int in6 = 26;
// D
int enD = 8;
int in7 = 31;
int in8 = 30;


// values
volatile int amm10x;
volatile int amm20x;
volatile int amm50x;
volatile int amm100x;

// buttons & leds
byte btn1 = 2;
byte btn2 = 3;
byte btn3 = 18;
byte btn4 = 19;
byte btn5 = A3;
byte btn5State = 0;

volatile boolean btn1On = false;
volatile boolean btn2On = false;
volatile boolean btn3On = false;
volatile boolean btn4On = false;



void setup() {
  Serial.begin(9600);     // Initialize serial communication with the PC
  SPI.begin();            // Init SPI bus
  mfrc522.PCD_Init();     // Init MFRC522 card

  pinMode(13, OUTPUT);    // contrast for LCD
  analogWrite(13, contrast);
  lcd.begin(20, 4);

  strFirstName = "";
  intPin = 0;
  intValue = 0;

  amm10x = 5;
  amm20x = 5;
  amm50x = 5;
  amm100x = 5;

  // PULL-UP resistor for btns
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  //pinMode(btn5, INPUT_PULLUP);
  pinMode(btn5, INPUT);

  // for DC motors:
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(enC, OUTPUT);
  pinMode(enD, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);
  pinMode(in7, OUTPUT);
  pinMode(in8, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(btn1), interruptBtn1, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn2), interruptBtn2, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn3), interruptBtn3, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn4), interruptBtn4, FALLING);
 // attachInterrupt(digitalPinToInterrupt(btn5), interruptBtn5, FALLING);
}

void loop() {
  //interruptBtn5();
  if(screenNo == 0 || screenNo == 7){
    startLCDText();
  }
  
  MFRC522::MIFARE_Key key;
  
  if(screenNo == 0){
    readFirstNamePICCC(firstName, &firstNameLength, pinRFID, &pinSize, balance, &balanceSize, &key, 4, 8, 12);
    strFirstName = transformPinNameToString(firstName, firstNameLength);
    intPin = transformPinToInt(pinRFID, pinSize);
    intValue = transformPinValueToInt(balance, balanceSize);
  }
  //readFirstNamePICC(firstName, &firstNameLength, &key, 4);
  if(firstName[0] == 32){ // == " "
    // no name yet ! X
  }
  else {
    if(screenNo < 2){
      //afterScanSuccessLCD(firstName, firstNameLength);
      afterScanSuccessLCD(strFirstName);
      screenNo = 1;
      Serial.println(Serial.available());
      while(Serial.available() == 0 && screenNo == 1){
        char customKey = customKeypad.getKey();
        if(customKey){
          Serial.print(customKey);
          atmPIN += customKey;
          displayLCD(3, 5 + atmPIN.length(), '*');
          if(atmPIN.length() == 4){
            Serial.println("haha");
            if(pinIsGood(atmPIN)) screenNo = 2; // good - light on green LED
            else { 
              /*noMistakes ++;
              if(noMistakes == 3) Serial.println("abc"); // bad - block card (uid) ligh on red LED
              else Serial.println("def");*/
              atmPIN = "";
            }
            break;
          }
        }
      }
    }
  }
  
  if(screenNo == 2){
    afterPinLCD();
    while(Serial.available() == 0 && screenNo == 2){
      char customKey = customKeypad.getKey();
      if(customKey == '1'){
        screenNo = 3;
      }
      else if(customKey == '2'){
        screenNo = 4;
      }
      else if(customKey == '3'){
        screenNo = 5;
      }
      else ;
    }
  }
  else if(screenNo == 3){
    showBalance();
  }
  else if(screenNo == 4){
    withdrawMoney();
    Serial.println("butoaneleeee::");
    Serial.println(btn1On);
    Serial.println(btn2On);
    Serial.println(btn3On);
    Serial.println(btn4On);
    if(btn1On == true){
      motorOn(in1, in2, enA, 1);
    }
    else if(btn2On == true){
      motorOn(in3, in4, enB, 1);
    }
    else if(btn3On == true){
      motorOn(in5, in6, enC, 1);
    }
    else if(btn4On == true){
      motorOn(in7, in8, enD, 1);
    }
    btn1On = false;
    btn2On = false;
    btn3On = false;
    btn4On = false;
    Serial.println(intValue);
    
  }

  else if(screenNo == 5){
    int moneyToUpdate = requestMoneyToUpdate();
    updateMoney(moneyToUpdate);
  }
  else if(screenNo == 6){ // Not enough Money msg
    notEnoughMoneyMsg();
    screenNo = 2;
  }

  /*if(screenNo == 3){
    showBalance();
  }
  else if(screenNo == 4){
    withdrawMoney();
    Serial.println("butoaneleeee::");
    Serial.println(btn1On);
    Serial.println(btn2On);
    Serial.println(btn3On);
    Serial.println(btn4On);
    if(btn1On == true){
      motorOn(in1, in2, enA, 1);
    }
    else if(btn2On == true){
      motorOn(in3, in4, enB, 1);
    }
    else if(btn3On == true){
      motorOn(in5, in6, enC, 1);
    }
    else if(btn4On == true){
      motorOn(in7, in8, enD, 1);
    }
    btn1On = false;
    btn2On = false;
    btn3On = false;
    btn4On = false;
    Serial.println(intValue);
    
  }

  else if(screenNo == 5){
    int moneyToUpdate = requestMoneyToUpdate();
    updateMoney(moneyToUpdate);
  }*/
}

void clearLCD(){
  for(byte i = 0; i < 4; i ++){
    for(byte j = 0; j < 20; j ++){
      lcd.setCursor(j, i);
      lcd.print(" ");
    }
  }
}

int requestMoneyToUpdate(){
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Insert money:");
  lcd.setCursor(15, 1);
  lcd.print("Euro");
  lcd.setCursor(10, 1);
  while(Serial.available() == 0 && screenNo == 5){
    char customKey = customKeypad.getKey();
    if(customKey){
      Serial.print(customKey);
      updateVal += customKey;
      displayLCD(1, 10 + updateVal.length() - 1, customKey);
      if(customKey == '#' || updateVal.length() == 4){
        screenNo = 2;
        intValue += updateVal.toInt();
      }
    }
  }
  updateVal = "";
  return intValue;
}

void updateMoney(int amount){
  ;
}

void startLCDText(){
  lcd.clear();
  delay(1000);
  lcd.setCursor(7, 0);
  lcd.print("Hello!");
  delay(1000);
  lcd.setCursor(2, 1);
  lcd.print("Please scan your");
  lcd.setCursor(7, 2);
  lcd.print("card!");
  //lcd.setCursor(12, 3);
  //lcd.print((char)15);
  delay(1000);
}

void afterScanSuccessLCD(String strFirstName){
  Serial.println(strFirstName);
  lcd.clear();
  delay(1000);
  lcd.setCursor(2, 0);
  lcd.print("Welcome,");
  lcd.setCursor(11, 0);
  //Serial.println(firstName);
  lcd.print(strFirstName);
  lcd.setCursor(1, 1);
  lcd.print("Please insert your");
  lcd.setCursor(6, 2); 
  lcd.print("PIN!");
  delay(1000);
}

void afterScanSuccessLCD(byte *firstName, byte firstNameLength){
  String myName = String((char *)firstName);
  //String myName = transformPinNameToString(firstName, firstNameLength);
  //Serial.println(myName);
  lcd.clear();
  delay(1000);
  lcd.setCursor(2, 0);
  lcd.print("Welcome,");
  lcd.setCursor(10, 0);
  //Serial.println(firstName);
  lcd.print(myName);
  lcd.setCursor(1, 1);
  lcd.print("Please insert your");
  lcd.setCursor(6, 2); 
  lcd.print("PIN!");
  delay(1000);
}

void displayLCD(int row, int col, char c){
  lcd.setCursor(col, row);
  lcd.print(c);
}

void afterPinLCD(){
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Select operation: ");
  lcd.setCursor(0, 1);
  lcd.print("1. View balance");
  lcd.setCursor(0, 2);
  lcd.print("2. Withdraw money");
  lcd.setCursor(0, 3);
  lcd.print("3. Update balance");
  delay(1000);
}

void readFirstNamePICCC(byte *firstName, byte *firstNameSize, byte *pinRFID, byte *pinSize, byte *balance, byte *balanceSize, MFRC522::MIFARE_Key *key, byte block1, byte block2, byte block3){
  
  // erase all data
  for(byte i = 0; i < *firstNameSize; i++){
    firstName[i] = 32; // == " " (space)
  }
  for(byte i = 0; i < *pinSize; i++){
    pinRFID[i] = 32; // == " " (space)
  }
  for(byte i = 0; i < *balanceSize; i++){
    balance[i] = 32; // == " " (space)
  }
  
  // prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i ++) key->keyByte[i] = 0xFF;

  // End when no new card present on sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }
  
  MFRC522::StatusCode status;
  Serial.setTimeout(20000L); // wait until 20 seconds for input from serial

  // read firstName info
  // Authenticate with key A?
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block1, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Read block
  status = mfrc522.MIFARE_Read(block1, firstName, firstNameSize); // ?
  if ( status != MFRC522::STATUS_OK){
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  else{
    Serial.println(F("MIFARE_Read() success: "));
    for(byte i = 0; i < *firstNameSize; i++){
      if(firstName[i] == 32){
        break;
      }
      Serial.print(char(firstName[i]));
    }
  }

  // read PIN info
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block2, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Read block
  status = mfrc522.MIFARE_Read(block2, pinRFID, pinSize); // ?
  if ( status != MFRC522::STATUS_OK){
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  else{
    Serial.println(F("MIFARE_Read() success: "));
    for(byte i = 0; i < *pinSize; i++){
      if(pinRFID[i] == 32){
        break;
      }
      Serial.print(char(pinRFID[i]));
    }
  }

  // read balance info
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block3, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Read block
  status = mfrc522.MIFARE_Read(block3, balance, balanceSize); // ?
  if ( status != MFRC522::STATUS_OK){
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  else{
    Serial.println(F("MIFARE_Read() success: "));
    for(byte i = 0; i < *balanceSize; i++){
      if(balance[i] == 32){
        break;
      }
      Serial.print(char(balance[i]));
    }
  }
}

boolean pinIsGood(String givenPin){
  String pinString = "";
  for(byte i = 0; i < pinSize; i++){
    if(isDigit((char)pinRFID[i]) && pinString.length() < 4)
      pinString += (char)pinRFID[i];
  }
  Serial.println(pinString);
  Serial.println(givenPin);
  if(givenPin == pinString)
    return true;
  return false;
}

void showBalance(){
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("You have:");
  lcd.setCursor(7, 1);
  Serial.println();
  lcd.print(intValue);
  Serial.println();
  //lcd.setCursor(7,2);
  lcd.setCursor(11, 1);
  lcd.print(" Euro");
  lcd.setCursor(3, 3);
  //lcd.print(" ");
  delay(1000);
}

boolean isValidChar(char c){
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
    return true;
  else return false; 
}

String transformPinNameToString(byte *firstname, byte nameSize){
  String retName = "";
  for(int i=0; i<nameSize; i++){
    if(isValidChar((char)firstName[i])){
      retName += (char)firstName[i];
    }
  }
  return retName;
}

int transformPinToInt(byte *pin, byte pinSize){
  String strPin = "";
  for(int i=0; i<pinSize; i++){
    if(isDigit((char)pin[i])){
      strPin += (char)pin[i];
    }
  }
  return strPin.toInt();
}

int transformPinValueToInt(byte *value, byte valSize){
  String strVal = "";
  for(int i=0; i<valSize; i++){
    if(isDigit((char)value[i])){
      strVal += (char)value[i];
    }
  }
  Serial.println("aici valoare:");
  Serial.println(strVal);
  
  return strVal.toInt();
}

void notEnoughMoneyMsg(){
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("You do not have");
  lcd.setCursor(4, 2);
  lcd.print("Enough money!");
  delay(1500);
}

void displayAmounts(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Other");
  lcd.setCursor(0, 1);
  lcd.print("amount");
  lcd.setCursor(17, 0);
  lcd.print(10);
  lcd.setCursor(17, 1);
  lcd.print(20);
  lcd.setCursor(17, 2);
  lcd.print(50);
  lcd.setCursor(16, 3);
  lcd.print(100);
  delay(1000);
}

void withdrawMoney(){
  displayAmounts();
  /*attachInterrupt(digitalPinToInterrupt(btn1), interruptBtn1, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn2), interruptBtn2, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn3), interruptBtn3, FALLING);
  attachInterrupt(digitalPinToInterrupt(btn4), interruptBtn4, FALLING);*/
  //attachInterrupt(digitalPinToInterrupt(btn5), interruptBtn5, FALLING);
}

void interruptBtn1(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 2000){ // debounce system - if(stabilized) then..
    if(screenNo == 4){
    Serial.println("Am apasat btn1!");
    if(intValue >= 10){
      intValue -= 10;
      amm10x --;
      btn1On = true;
    }
    else screenNo = 6;
    delay(250);
    screenNo = 2;
    }
  }
  last_interrupt_time = interrupt_time;
}

void interruptBtn2(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 2000){ // debounce system - if(stabilized) then..
    if(screenNo == 4){
    Serial.println("Am apasat btn2!");
    if(intValue >= 20){
      intValue -= 20;
      amm20x --;
      btn2On = true;
    }
    else screenNo = 6;
    delay(250);
    screenNo = 2;
    }
  }
  last_interrupt_time = interrupt_time;
}

void interruptBtn3(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 2000){ // debounce system - if(stabilized) then..
    if(screenNo == 4){
    Serial.println("Am apasat btn3!");
    if(intValue >= 50){
      intValue -= 50;
      amm50x --;
      btn3On = true;
    }
    else screenNo = 6;
    delay(250);
    screenNo = 2;
    }
  }
  last_interrupt_time = interrupt_time;
}

void interruptBtn4(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 2000){ // debounce system - if(stabilized) then..
    if(screenNo == 2){
      resetFunc();
    }
    else if(screenNo == 3){
      screenNo = 2;
    }
    else if(screenNo == 4){
      Serial.println("Am apasat btn4!");
      if(intValue >= 100){
        intValue -= 100;
        amm100x --;
        btn4On = true;
      }
      else screenNo = 6;
      delay(250);
      screenNo = 2;
    }
    else if(screenNo == 6){
      screenNo = 2;
    }
    else ;
  }
  Serial.println(screenNo);
  last_interrupt_time = interrupt_time;
}

void motorOn(byte dir1, byte dir2, byte turationPin, int noCycles){
  Serial.println("am bani destui!");
  for(byte cycle = 0; cycle < noCycles; cycle++){
    analogWrite(turationPin, 200);
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, HIGH);
    delay(700);
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, LOW);
    delay(200);
    digitalWrite(dir1, HIGH);
    digitalWrite(dir2, LOW);
    delay(600);
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, HIGH);
    delay(200);
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, LOW);
    delay(200);
    digitalWrite(dir1, HIGH);
    digitalWrite(dir2, LOW);
    delay(2000);
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, LOW);
    delay(250);
    
  }
}
