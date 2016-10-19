//#include <DallasTemperature.h>

#include <SD.h>
#include <LiquidCrystal.h>
#include "fileStream.h"
#include "recipe.h"
#include "rotaryEncoder.h"
#include "jsonsp.h"

//-------define pins ------------------------------------
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

//-------define Key names for json file -----------------
#define RECIPE_NAME "name"
#define RECIPE_PHASES "Phases"
#define RECIPE_PHASE_NAME "Name"
#define RECIPE_PHASE_INCREASE_TEMPERATURE "IncT"
#define RECIPE_PHASE_MAINTAIN_TEMPERATURE "MaintainT"
#define RECIPE_PHASE_KEY_EVENT "KeyEvent"

Recipe<6> recipe;
RotaryEncoder myEnc(ENCODER_CLK_PIN, ENCODER_DT_PIN);
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_EN, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);
int currentPhase = 0;
float currentTemperature = 20.8;
int timePassed = 11;

void setup() {
  lcd.begin(20,4);
  //lcd.print("hello");
  
  pinMode (ENCODER_SW_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println(freeRam());

  String fileName = loadRecipe();
  File parseFile = SD.open(fileName);
  FileStream fstream(&parseFile);
  Serial.println(freeRam());
  parseJsonObj(&fstream);


  for (int i = 0; i < recipe.size_; ++i){
    currentPhase = i;
    printCurrentStatus();
    delay(3000);
  }
  
  
  Serial.println(freeRam());
}


void printCurrentStatus(){
  lcd.setCursor(0,0);
  lcd.print(recipe.name_);
  Serial.println(recipe.name_);
  
  lcd.setCursor(0,1);
  lcd.print(recipe.phases_[currentPhase].name_);
  
  lcd.setCursor(0,2);
  lcd.print("T=");
  lcd.print(currentTemperature);
  lcd.print("C, Aim=");
  lcd.print(recipe.phases_[currentPhase].incT_);
  lcd.print("C");
  
  lcd.setCursor(0,3);
  lcd.print(timePassed);
  lcd.print(" min. (");
  lcd.print(recipe.phases_[currentPhase].maintainT_);
  lcd.print(" min.)");
  
}

int freeRam (){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

bool parseJsonObj(inputStream * str){
  Jsonsp par(*str);
  //Serial.println("start parsing");
  Serial.println(freeRam());

  int numberOfPhases;

  recipe.size_ = 0;
  if ( par[RECIPE_NAME].objType() == 3 )
    recipe.name_ =  par[RECIPE_NAME].asString();
  else 
      return false;

  //Serial.println("Name = ");
  Serial.println(recipe.name_);

  if ( par[RECIPE_PHASES].objType() == 2 ){
    numberOfPhases = par[RECIPE_PHASES].size();
    
    if (numberOfPhases < 1 || numberOfPhases > 20)
      return false;
  }

  //Serial.println("Number of phases ");
  Serial.println(numberOfPhases);

  for (int i = 0; i < numberOfPhases; ++i){
    //Serial.print("i=");
    //Serial.println(i);
    int y = par[RECIPE_PHASES][i].objType();
    //Serial.print("ObjectType=");
    //Serial.println(y);
    //continue;
    if ( par[RECIPE_PHASES][i].objType() == 1 ){
      if ( par[RECIPE_PHASES][i][RECIPE_PHASE_NAME].objType() == 3 )
        recipe.phases_[i].name_ =  par[RECIPE_PHASES][i][RECIPE_PHASE_NAME].asString();
      else 
          return false;
      
    Serial.println(freeRam());
    Serial.print("-------------------------- ");
    Serial.println(par[RECIPE_PHASES][i][RECIPE_PHASE_INCREASE_TEMPERATURE].objType());
    
      if ( par[RECIPE_PHASES][i][RECIPE_PHASE_INCREASE_TEMPERATURE].objType() == 3 )
        recipe.phases_[i].incT_ =  par[RECIPE_PHASES][i][RECIPE_PHASE_INCREASE_TEMPERATURE].asInt();
      else 
          return false;
      
    Serial.print("recipe.phases_[i].incT_");
    Serial.println(recipe.phases_[i].incT_);
      if ( par[RECIPE_PHASES][i][RECIPE_PHASE_MAINTAIN_TEMPERATURE].objType() == 3 )
        recipe.phases_[i].maintainT_ =  par[RECIPE_PHASES][i][RECIPE_PHASE_MAINTAIN_TEMPERATURE].asInt();
      else 
          return false;
      
    Serial.print("recipe.phases_[i].maintainT_");
    Serial.println(recipe.phases_[i].maintainT_);
      if ( par[RECIPE_PHASES][i][RECIPE_PHASE_KEY_EVENT].objType() == 3 )
        recipe.phases_[i].keyEvent_ =  par[RECIPE_PHASES][i][RECIPE_PHASE_KEY_EVENT].asBool();
      else 
          return false;
      
    Serial.print("recipe.phases_[i].keyEvent_");
    Serial.println(recipe.phases_[i].keyEvent_);
    }

    ++recipe.size_;
    
  }

  Serial.println(recipe.size_);
  Serial.println(recipe.name_);
  for (int i = 0; i < recipe.size_; ++i){
    Serial.print(recipe.phases_[i].name_);
    Serial.print("  ");
    Serial.print(recipe.phases_[i].incT_);
    Serial.print("  ");
    Serial.print(recipe.phases_[i].maintainT_);
    Serial.print("  ");
    Serial.println(recipe.phases_[i].keyEvent_);
  }
  Serial.println(freeRam());
}


String loadRecipe(){
#define ARRAY_SIZE 20
  String files[ARRAY_SIZE];
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return "";
  }
  Serial.println("initialization done.");  
  File parseFile = SD.open("/");
  int pos = 0;
  for (int i = 0; i < ARRAY_SIZE; ++i){
    parseFile = parseFile.openNextFile();
    Serial.println(parseFile.name());
    if ( parseFile.isDirectory() )
      continue;

    files[pos] = parseFile.name();
  }
  
  return "recept1.txt";
  
  while (true){
    int res = myEnc.getEncoderTurn();
    int res2 = digitalRead( ENCODER_SW_PIN );
    if (res) 
      Serial.println(res);
    if (res2 == 0) 
      Serial.println(res2);
  }
  

}


void loop() {
}
