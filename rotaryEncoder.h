#ifndef __ROTARYENCODER_H_
#define __ROTARYENCODER_H_
class RotaryEncoder{
public:
  RotaryEncoder(int clkPin, int dt_pin){
    pinMode (clkPin, INPUT);
    pinMode (dt_pin, INPUT);
    dt_pin_ = dt_pin;
    clkPin_ = clkPin;
    oldA = digitalRead (clkPin_);
    oldB = digitalRead (dt_pin_);

  }
  int getEncoderTurn ()
  {
    // Return -1, 0, or +1
    int result = 0;
    int newA = digitalRead (clkPin_);
    int newB = digitalRead (dt_pin_);
/*    Serial.print(newA);
    Serial.print("  ");
    Serial.print(newB);
    Serial.print("  ");
    Serial.print(oldA);
    Serial.print("  ");
    Serial.print(oldB);
    Serial.print("  ");
*/   
    if (newA != oldA || newB != oldB)
    {
      //Something has changed
      if (oldA == LOW && newA == HIGH)
      {
        result = - (oldB * 2 - 1);
      }
    }
    oldA = newA;
    oldB = newB;
    return result;
  }
  
  
private:
  int clkPin_;
  int dt_pin_;
  int oldA;
  int oldB;
    
};
#endif 

