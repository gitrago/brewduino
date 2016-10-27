#ifndef __RECIPE_H_
#define __RECIPE_H_
#include <EEPROM.h>

template<int N>
class Recipe{
public:
  struct node_{
    friend Recipe;
  public:
    void setName(String& name){
      len = name.length();
      for (int i = 0; i < len; ++i)
        EEPROM[startPos+i] = name[i];
      
    }
    
    void setName(String&& name){
      len = name.length();
      for (int i = 0; i < len; ++i)
        EEPROM[startPos+i] = name[i];
      
    }
    
    String name(){
      String s = "";
      for (int i = 0; i < len; ++i)
        s += (char)EEPROM[startPos+i];
      return s;
    }
  
    byte incT_;
    byte maintainT_;
    bool keyEvent_;
  
  private:
    int startPos;
    byte len = 0;
    //String name_;
  };


  Recipe(){
    for (int i = 0; i < N; ++i)
      phases_[i].startPos = 40 + i*25;
  }
  
  
  void setName(String&& str){
      len_ = str.length();
      for (int i = 0; i < len_; ++i)
        EEPROM[i] = str[i];
      
  }
  
  void setName(String& str){
      len_ = name.length();
      for (int i = 0; i < len_; ++i)
        EEPROM[i] = str[i];
      
  }
  
  String name(){
      String s = "";
      for (int i = 0; i < len_; ++i)
        s += (char)EEPROM[i];
      return s;
  }
  
  byte size(){return size_;}
  
  
  int size_ = 0;
  node_ phases_[N];

private:
  //int currentPos;
  //int name_;
  byte len_ = 0;
  String name_;
  
};

#endif 

