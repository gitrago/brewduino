#ifndef __JSONSP_H_
#define __JSONSP_H_
#include "fileStream.h"
typedef FileStream inputStream;

/*
  objType_:
    -1 : unknow type
     1 : object
     2 : array
     3 : string
     4 : primitive
 
*/


class Jsonsp{
public:
  Jsonsp(inputStream& instream):instream_(instream){startBlockPos_ = 0;}
  Jsonsp operator[](const char* filed);
  Jsonsp operator[](const int idx);
  String asString();
  int asInt();
  bool asBool();
  float asFloat();
  long asLong();
  int size();
  int objType(){return objType_;}
  
//private:

  int startBlockPos_;
  int objType_;
  String value_;
  int arrSize = 0;
  inputStream& instream_;
  bool isWhiteChar(char c);
  int getType(int start = -1);
  
};

#endif 
