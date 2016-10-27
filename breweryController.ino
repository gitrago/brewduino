#include <Encoder.h>

#include <TimerOne.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#include <SD.h>
#include <LiquidCrystal.h>
#include "fileStream.h"
#include "recipe.h"
#include "jsonsp.h"


#define ENCODER_USE_INTERRUPTS
//typedef RotaryEncoder Enc;
typedef Encoder Enc;
#define RECIPE_MAX_STAGES 20
#define TIME_PERIOD_FOR_LONG_PUSH 2000
#define LOG_FILE_NAME F("/logs/log.txt")
#define RECIPE_DIR F("/recipe/")
//-------define pins ------------------------------------
#define J1_PIN A4
#define J2_PIN 8

#define ENCODER_SW_PIN A5
#define ENCODER_DT_PIN 2
#define ENCODER_CLK_PIN 3

#define LCD_PIN_RS 5
#define LCD_PIN_RW 6
#define LCD_PIN_EN 7

#define LCD_PIN_D4 A0
#define LCD_PIN_D5 A1
#define LCD_PIN_D6 A2
#define LCD_PIN_D7 A3

#define DS18B20_PIN 9
#define BEEPER_PIN 4
//-------define Key names for json file -----------------
#define RECIPE_NAME "name"
#define RECIPE_PHASES "Phases"
#define RECIPE_PHASE_NAME "Name"
#define RECIPE_PHASE_INCREASE_TEMPERATURE "IncT"
#define RECIPE_PHASE_MAINTAIN_TEMPERATURE "MaintainT"
#define RECIPE_PHASE_KEY_EVENT "KeyEvent"


#define FAIL_T_TIME 2
#define FAIL_TEMPERATURE 1

//DS18b20
OneWire oneWire(DS18B20_PIN);
DallasTemperature t_sensor(&oneWire);
DeviceAddress sensor1;
Enc myEnc(2, 3);

Recipe<RECIPE_MAX_STAGES> *recipe = 0;

//LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_EN, LCD_PIN_D0, LCD_PIN_D1, LCD_PIN_D2, LCD_PIN_D3, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_EN, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);
int currentPhase = 0;
bool isHeat = true;
bool isStart = true;
bool mustStop = false;
bool keyPressed = false;
bool isFailed = false;
float currentTemperature = 0;
int timePassed = 11;
unsigned long startTime;


#define HYSTERESIS 1
#define PRE_OFF 0

void setup() {
  
  
  
  lcd.begin(20,4);
  //lcd.print("hello");
  
  pinMode (ENCODER_SW_PIN, INPUT_PULLUP);

  pinMode (J1_PIN, OUTPUT);
  pinMode (J2_PIN, OUTPUT);
  digitalWrite (J1_PIN, HIGH);
  digitalWrite (J2_PIN, HIGH);
  
  pinMode (BEEPER_PIN, OUTPUT);

  
  //Serial.begin(9600);
  //Serial.println(freeRam());
  //for (int i = 0; i < 8; ++i)
//    Serial.println(sensor1[i]);

  t_sensor.begin();
  t_sensor.getAddress(sensor1, 0);
  t_sensor.setResolution(12);

  //for (int i = 0; i < 8; ++i)
    //Serial.println(sensor1[i]);
    
  lcd.clear();
  lcd.print(F("Initializing SD card"));
  if (!SD.begin(10)) {
    lcd.setCursor(0, 1);
    lcd.print(F("initialization fail"));
    lcd.setCursor(3, 2);
    lcd.print(F("insert SD card"));
    lcd.setCursor(2, 3);
    lcd.print(F("and reset device"));
    
    while (true){
      
    }
  }


/*
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
   
  }
  Serial.println("initialization done.");  
*/


  //Serial.println(freeRam());
  
}

/*
String addSpace(const String& s1){
  String s = s1;
  int l = s.length();

  if ( l <= 20 )
    l = 20 - l;
  else 
    return s.substring(0, 19);

  for (int i = 0; i < l; ++i)
    s += ' ';

  return s;
}
*/
void printCurrentStatus(){
  lcd.clear();
  //const char sp[] = "                    ";
  //lcd.setCursor(0,0);
  //lcd.print(sp);
  lcd.setCursor(0,0);
  
  lcd.print(recipe->name());
  //Serial.println(recipe->name());
  
  //lcd.setCursor(0,1);
  //lcd.print(sp);
  lcd.setCursor(0,1);
  
  lcd.print( recipe->phases_[currentPhase].name() );
  //lcd.print(freeRam());
  
  //lcd.setCursor(0,2);
  //lcd.print(sp);
  lcd.setCursor(0,2);
  lcd.print( F("T=") );
  lcd.print( currentTemperature );
  lcd.print( F("C, Aim=") );
  lcd.print( recipe->phases_[currentPhase].incT_ );
  lcd.print( F("C") );

  //lcd.setCursor(0,3);
  //lcd.print(sp);
  lcd.setCursor(0,3);
  if ( isHeat) {
    lcd.print( F("Heating ") );
    lcd.print( timePassed );
    lcd.print( F(" min.") );
  }else{
    lcd.print( timePassed );
    lcd.print( F(" min. (") );
    lcd.print( recipe->phases_[currentPhase].maintainT_ );
    lcd.print( F(" min.)") );
  }
}

int freeRam (){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}



bool parseJsonObj(inputStream& str){
  Jsonsp par(str);
  //Serial.println("start parsing");
  //Serial.println(freeRam());

  int numberOfPhases;
  //Serial.println(freeRam());

  recipe->size_ = 0;
  if ( par[RECIPE_NAME].objType() == 3 )
    recipe->setName( par[RECIPE_NAME].asString() );
  else 
      return false;

  if ( par[RECIPE_PHASES].objType() == 2 ){
    numberOfPhases = par[RECIPE_PHASES].size();
    
    if (numberOfPhases < 1 || numberOfPhases > 20)
      return false;
  }

  for (int i = 0; i < numberOfPhases; ++i){
    //int y = par[RECIPE_PHASES][i].objType();
    Jsonsp phase = par[RECIPE_PHASES][i];
    if ( phase.objType() == 1 ){
      if ( phase[RECIPE_PHASE_NAME].objType() == 3 )
        recipe->phases_[i].setName( phase[RECIPE_PHASE_NAME].asString() );
      else 
          return false;
      
    
      if ( phase[RECIPE_PHASE_INCREASE_TEMPERATURE].objType() == 3 )
        recipe->phases_[i].incT_ =  phase[RECIPE_PHASE_INCREASE_TEMPERATURE].asInt();
      else 
          recipe->phases_[i].incT_ = 0;
      
      if ( phase[RECIPE_PHASE_MAINTAIN_TEMPERATURE].objType() == 3 )
        recipe->phases_[i].maintainT_ =  phase[RECIPE_PHASE_MAINTAIN_TEMPERATURE].asInt();
      else 
          recipe->phases_[i].maintainT_ = 0;
      
      if ( phase[RECIPE_PHASE_KEY_EVENT].objType() == 3 )
        recipe->phases_[i].keyEvent_ =  phase[RECIPE_PHASE_KEY_EVENT].asBool();
      else 
          recipe->phases_[i].keyEvent_ = false;
      
      //Serial.println(freeRam());
    }

    ++recipe->size_;
    
  }
/*
  Serial.println(freeRam());
  Serial.println(recipe->size_);
  Serial.println(recipe->name());
  for (int i = 0; i < recipe->size_; ++i){
    Serial.print(recipe->phases_[i].name());
    Serial.print(F("  "));
    Serial.print(recipe->phases_[i].incT_);
    Serial.print(F("  "));
    Serial.print(recipe->phases_[i].maintainT_);
    Serial.print(F("  "));
    Serial.println(recipe->phases_[i].keyEvent_);
  }
  Serial.flush();
  Serial.println(freeRam());
*/
  

  return true;
}

String loadRecipe(){
  //Serial.println(freeRam());
  //RotaryEncoder myEnc(ENCODER_CLK_PIN, ENCODER_DT_PIN);
  //Serial.println((int)myEnc.clkPin_);
  //Serial.println((int)myEnc.dt_pin_);
  lcd.clear();
//  Enc myEnc(2, 3);

#define ARRAY_SIZE 30
#define startEEPROMAdress 512
  //char files[ARRAY_SIZE][13] = {0};
  char fileName[13] = {0};
  byte arraySize = 0;
  byte startPos = 0;
  byte cursorPos = 0;
  
  if ( !SD.exists(RECIPE_DIR) )
    SD.mkdir(RECIPE_DIR);

  File rootDir = SD.open(RECIPE_DIR);
  rootDir.rewindDirectory();

  for (int i = 0; i < ARRAY_SIZE; ++i){
    File parseFile = rootDir.openNextFile();
    
    if ( !parseFile ){
      parseFile.close();      
      break;
    }
      
    //Serial.println(parseFile.name());
    if ( parseFile.isDirectory() ){
      parseFile.close();
      continue;
    }

    strcpy(fileName, parseFile.name());
    EEPROM.put(startEEPROMAdress+arraySize*sizeof(fileName), fileName);
    parseFile.close();
    
    ++arraySize;
  }

  rootDir.close();

  //Serial.println(freeRam());
  //Serial.flush();
  
  lcd.setCursor(0,0);
  lcd.print(F("Change file :"));
  //lcd.print(freeRam());
  //lcd.print(arraySize);

  if (arraySize == 0) {
    lcd.setCursor(0,1);
    lcd.print(F("      no files"));
    while (true){
      
    }
    
  }

  
  byte choice = 0;
  bool firstRun = true;
  while (true){

    int res = myEnc.read();//EncoderI.getEncoderTurn();
    int res2 = getButtonEvent();

    if (res2 != 0){
      choice = startPos+cursorPos;
      break;
    }
    
    if (res or firstRun){
      
      firstRun = false;
      if ( (cursorPos + res) > 2 ){
        if ( (startPos + cursorPos + res) < arraySize ){
          startPos += res;
        }
      }else if ( (cursorPos + res) < 0 ){
        if ( (startPos + cursorPos + res) >= 0 ){
          startPos += res;
        }
      }else {
        if ( (cursorPos + res) < arraySize)
          cursorPos += res;
      }

      for (int i = 0; i < 3; ++i){
        if ( (startPos + i) >= arraySize )
          break;
          
        lcd.setCursor(0, i+1);
        lcd.print(F("                   "));
        EEPROM.get(startEEPROMAdress + sizeof(fileName) * (startPos+i), fileName);
        byte t = (20 - strlen(fileName) )/2;
        lcd.setCursor(t,i+1);
        lcd.print(fileName);
        if (cursorPos == i){
          lcd.setCursor(0, i+1);
          lcd.print(F("  ["));
          lcd.setCursor(17, i+1);
          lcd.print(F("]"));
        }
      }

          myEnc.write(0);

      
    }
    
  }
  //delay(100);
  EEPROM.get(startEEPROMAdress + sizeof(fileName) * choice, fileName);
  String s = RECIPE_DIR;
  s += fileName;
  //delete[] &files;
  return s;
}


void logger(String str, bool saveTime = true){
  if ( !SD.exists(F("/logs")) )
    SD.mkdir(F("/logs"));
    
  File logFile = SD.open(LOG_FILE_NAME, FILE_WRITE);
  
  if (saveTime){
    unsigned long cTime = (millis() - startTime) / 1000;
    logFile.print(cTime/60, 10);
    logFile.print(F(":"));
    //logFile.print(F(" min., "));
    //logFile.print(cTime%60, 10);
    //logFile.print(F(" sec. : "));
  }
  
  logFile.println(str);
  logFile.close();
  
}



void shortBeep(){
  digitalWrite(BEEPER_PIN, HIGH);
  delay(100);
  digitalWrite(BEEPER_PIN, LOW);
}

void longBeep(){
  digitalWrite(BEEPER_PIN, HIGH);
  delay(1000);
  digitalWrite(BEEPER_PIN, LOW);
  
}

String readline(File & file){
  char c;
  char readChar;
  String buf = "";
  
  while ( (c = file.read(&readChar, 1)) ){
    if ( readChar == '\r' ){
      file.read(readChar, 1);
      break;
    }

    buf += readChar;
  }

  return buf;
}

void showLog(){
  //lcd.print(freeRam() );
  //delay(1000);
  File logFile2 = SD.open(LOG_FILE_NAME, FILE_READ);

  /*
  if (! logFile2){
    logFile2.close();
    return;
  }
    */
  int fPos[30] = {0};
  int lineNumber = 0;

  for (int i = 0; i < 30; ++i){
    int p = logFile2.position();
    String s = readline(logFile2);
    //Serial.println(s);
    
    if ( s != "" ){
      fPos[i] = p;
      ++lineNumber;
    }
    else 
      break;
    
  }

  //Serial.println(lineNumber);
  bool isFirst = true;
  int startPos = 0;
  //lcd.print((bool)logFile);
  //delay(1000);
  while (true){
    int res = myEnc.read();
    int key = getButtonEvent();
    
    if ( key != 0 )
      break;
    
  
    if ( res || isFirst ){
      isFirst = false;
      if (res > 1)
        res = 1;
  
      if (res < -1)
        res = -1;
      myEnc.write(0);
  
      if ( ( (startPos+res) >= 0 ) && ( (startPos+res) <= (lineNumber-4) ) )
        startPos += res;
    
    
      lcd.clear();
      for (int i = 0; i < 4; ++i){
        if ( (startPos+i) >= lineNumber )
          break;
        logFile2.seek(fPos[startPos+i]);
        String s = readline(logFile2);
        lcd.setCursor(0,i);
        lcd.print(s);
      }
      
      
    }
  
  }

 logFile2.close();
}

void stopRegulator(){
  Timer1.stop();
  digitalWrite(J1_PIN, HIGH);
  digitalWrite(J2_PIN, HIGH);
  mustStop = true;
  
}


void regulatorMenu(){
  char items[3][13] = {"Cancel","Show log", "Stop brewing"};

  byte pos = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Menu:"));

  for (int i = 0; i < 3; ++i){
    byte len = strlen(items[i]);
    lcd.setCursor((20-len)/2, i+1);
    lcd.print(items[i]);
  }
  
  lcd.setCursor(2,1+pos);
  lcd.print("[");
  lcd.setCursor(18,1+pos);
  lcd.print("]");

  
  while (true){
    int res = myEnc.read();
    int key = getButtonEvent();

    if (res){
      //Serial.println(res);
      if (res > 1)
        res = 1;

      if (res < -1)
        res = -1;
      myEnc.write(0);

      if ( ( (pos+res) <= 2 ) and ( (pos+res) >= 0 ) )
        pos += res;


        for (int i = 0; i < 3; ++i){
          lcd.setCursor(2,i+1);
          lcd.print(" ");
          lcd.setCursor(18,i+1);
          lcd.print(" ");
        }
        
        lcd.setCursor(2,1+pos);
        lcd.print("[");
        lcd.setCursor(18,1+pos);
        lcd.print("]");
    }


    if ( key != 0 ){
      if (pos == 2){
        if ( confirmation(F("Stop brewing."), "") ){
          logger(F("stopped by user."));
          stopRegulator();
          break;
        } 
      }
          
      if (pos == 1){
        showLog();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Menu:"));
      
        for (int i = 0; i < 3; ++i){
          byte len = strlen(items[i]);
          lcd.setCursor((20-len)/2, i+1);
          lcd.print(items[i]);
        }
        
        lcd.setCursor(2,1+pos);
        lcd.print("[");
        lcd.setCursor(18,1+pos);
        lcd.print("]");
      
        
      }
       
      if (pos == 0)
        return;
    }
  }
}

void startRegulator(){
  timePassed = 0;
  
  SD.remove(LOG_FILE_NAME);
  //Serial.println(freeRam());

  delay(300);
  mustStop = false;
  isStart = true;
  keyPressed = false;
  isHeat = recipe->phases_[0].incT_;
  startTime = millis();
  logger(F(" started."));
  logger(recipe->name(), false);

  Timer1.initialize(100000);
  Timer1.attachInterrupt(regulator);

  //Serial.println(freeRam());

  while (! mustStop){
    if ( isFailed) {
      faildIncreaseT();
      isFailed = false;
    }
    printCurrentStatus();
    switch ( getButtonEvent() ){
      case (0) :
        break;
      case (1) :
        keyPressed = true;
        break;
      case (2) :
        regulatorMenu();
        break;
    }
    delay(50);
  }
  
}

void faildIncreaseT(){
  logger(F("Temp. inc. faild"));
  //stopRegulator();
  lcd.clear();
  lcd.print(F("  Temp. increasing"));
  lcd.setCursor(5,1);
  lcd.print(F("is failed."));
  lcd.setCursor(5,2);
  //lcd.print(freeRam());
  longBeep();
  delay(1000);
  longBeep();
}

void regulator(){
  
  t_sensor.setWaitForConversion(false);
  t_sensor.requestTemperaturesByAddress(sensor1);
  t_sensor.setWaitForConversion(true);
  
  //Serial.println(t_sensor.isConnected(sensor1));
  currentTemperature = t_sensor.getTempC(sensor1);
  //Serial.println(currentTemperature);
  static float oldT = currentTemperature;
  static unsigned long timeOfOldT = millis();
  static unsigned long startPhaseTime = millis();
  



  unsigned  long cTime =  millis();

  if (isStart){
    startPhaseTime = cTime;
    oldT = currentTemperature;
    timeOfOldT =cTime;
    isStart = false;
  }
  
  timePassed = (cTime-startPhaseTime)/1000/60;
  unsigned long timeFI = (cTime-timeOfOldT)/1000/60;
  
  
  if ( isHeat ){
    if ( (timeFI) >= FAIL_T_TIME){
      if ( (currentTemperature - oldT) <= FAIL_TEMPERATURE ){
        // stop Regulator
        isFailed = true;;
        //return;
      }
      

      oldT = currentTemperature;
      timeOfOldT = cTime;      
    }
    
    if ( currentTemperature <= recipe->phases_[currentPhase].incT_ )
      digitalWrite(J1_PIN, LOW);
    else{
      digitalWrite(J1_PIN, HIGH);
      isHeat = false;
      timeOfOldT = cTime;
      startPhaseTime = cTime;
      shortBeep();
      logger(F("heating finished"));
      logger(recipe->phases_[currentPhase].name(), false);
      
    }
  }else{

    if ( currentTemperature >= recipe->phases_[currentPhase].incT_ )
      digitalWrite(J2_PIN, HIGH);
    
    if ( currentTemperature <= (recipe->phases_[currentPhase].incT_-HYSTERESIS) )
      digitalWrite(J2_PIN, LOW);
  
    //byte workTime = (startTime - millis()) * 1000 * 60;
    if ( (recipe->phases_[currentPhase].maintainT_ != 0) && ( timePassed >= recipe->phases_[currentPhase].maintainT_ ) ){
      logger(F("maintain finished"));
      logger(recipe->phases_[currentPhase].name(), false);
      
      ++currentPhase;
      if ( currentPhase >= recipe->size() ){
        stopRegulator();
      }
      
      isHeat = recipe->phases_[currentPhase].incT_;
      digitalWrite(J2_PIN, HIGH);
      timeOfOldT = cTime;
      startPhaseTime = cTime;
      shortBeep();
    }
    

    if ( recipe->phases_[currentPhase].keyEvent_ && keyPressed){
      keyPressed = false;
      logger(F("maintain finished"));
      logger(recipe->phases_[currentPhase].name(), false);
      
      ++currentPhase;
      if ( currentPhase >= recipe->size() ){
        stopRegulator();
      }
      
      isHeat = recipe->phases_[currentPhase].incT_;
      digitalWrite(J2_PIN, HIGH);
      timeOfOldT = cTime;
      startPhaseTime = cTime;
      shortBeep();
    }
  
  
  }

  
  //Serial.println("R");
}

bool confirmation(String str1, String str2){
  //Enc myEnc(2, 3);

  const char yes_no[2][4] = {"No", "Yes"};
  //RotaryEncoder myEnc(ENCODER_CLK_PIN, ENCODER_DT_PIN);
  byte pos = 1;

  lcd.clear();
  
  int startPos = (20-str1.length())/2;
  lcd.setCursor(startPos, 0);
  lcd.print(str1);
  
  startPos = (20-str2.length())/2;
  lcd.setCursor(startPos, 1);
  lcd.print(str2);
  
  lcd.setCursor(9,2);
  lcd.print(yes_no[0]);
  lcd.setCursor(9,3);
  lcd.print(yes_no[1]);

  lcd.setCursor(7,2+pos);
  lcd.print("[");
  lcd.setCursor(14,2+pos);
  lcd.print("]");

  
  while (true){
    int res = myEnc.read();//EncoderI.getEncoderTurn();
    int res2 = getButtonEvent();

    if (res2 != 0){
      return pos;
    }
    
    if (res){
      if ( ( (pos+res) <= 1 ) and ( (pos+res) >= 0 ) )
        pos += res;

        lcd.setCursor(7,2);
        lcd.print(" ");
        lcd.setCursor(14,2);
        lcd.print(" ");
        
        lcd.setCursor(7,3);
        lcd.print(" ");
        lcd.setCursor(14,3);
        lcd.print(" ");
        
        lcd.setCursor(7,2+pos);
        lcd.print("[");
        lcd.setCursor(14,2+pos);
        lcd.print("]");
        myEnc.write(0);
    }
    
  }
}


void loop() {
  
  if (recipe == 0)
    recipe = new Recipe<RECIPE_MAX_STAGES>();
  else{
    delete recipe;
    recipe = new Recipe<RECIPE_MAX_STAGES>();
  }
  //Serial.println(sizeof(myEnc));
  //Serial.print("before load");
  //Serial.println(freeRam());
  
  File f;
      
  String fileName = loadRecipe();
  //Serial.print("after load");
  //Serial.println(freeRam());

  //Serial.print("before parse");
  //Serial.println(sizeof(lcd));
  //Serial.flush();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F(" Loading recipe ..."));

  
  f = SD.open(fileName, FILE_READ);
      
  //Serial.println(sizeof(String));
  FileStream fstream(&f);
  //Serial.println(freeRam());
  bool result = parseJsonObj(fstream);
  f.close();
  if (not  result){
    lcd.setCursor(0,1);
    lcd.print(F(" Uncorrect format."));
    longBeep();
    delay(3000);
    return;
  }
  
  
  
    
  //Serial.print("after parse");
  //Serial.println(freeRam());

  if (! confirmation(F("Start recipe"), recipe->name()) )
    return;
  
  

  startRegulator();
  showLog();
  delay(3000);
}

byte getButtonEvent(){
  static byte oldStatus = digitalRead(ENCODER_SW_PIN);
  byte newStatus = digitalRead(ENCODER_SW_PIN);
  static unsigned long pushedTime = 0;
  byte res = 0;
  
  if ( oldStatus == HIGH && newStatus == LOW ){
    pushedTime = millis();
  }
  
  if ( oldStatus == LOW && newStatus == HIGH ){
    if ( (millis() - pushedTime) > TIME_PERIOD_FOR_LONG_PUSH )
      res = 2;
    else
      res = 1;
  }
  
  oldStatus = newStatus;

  return res;
}

