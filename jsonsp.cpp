#include "jsonsp.h"
#include <avr/pgmspace.h>

bool Jsonsp::isWhiteChar(char c){
  if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
    return true;
  else 
    return false;
}



int Jsonsp::getType(int start){
  if ( start == -1 )
    start = startBlockPos_;
    
  for (int i = start; i < instream_.size(); ++i){
    if ( isWhiteChar(instream_[i]) )
      continue;

    if ( instream_[i] == '{' )
      return 1;
    
    if ( instream_[i] == '[' )
      return 2;
    
    if ( instream_[i] == '\"' )
      return 3;
    
  }

}

int Jsonsp::size(){
  int tmpSize = 0;
  if ( objType_ == 2 ){
    
    int endPos = instream_.find(']', startBlockPos_);
    int i = startBlockPos_+1;
    ++tmpSize;
    while ( i < endPos ){
      int t = getType( i );
      if ( t == 1 ){
        int endBlockPos = instream_.find('}', i);
        
        if (endBlockPos == -1) {
          objType_ = -1;
          return 0;
        }

        endBlockPos = instream_.find(',', endBlockPos);
        
        if ( endBlockPos == -1)
          break; 

        i = endBlockPos+1;
        ++tmpSize;
      }

      ++i;
      
    }
    
    
  }
  return tmpSize;
}


Jsonsp Jsonsp::operator[](const int idx){
  //Serial.println("\n\n operator [int]\n");
  Jsonsp res(instream_);

  if ( getType() != 2 ){
    res.objType_ = -1;
    return res;
  }
  
  int streamLength = instream_.find(']');

  if ( streamLength == -1 ){
    res.objType_ = -1;
    return res;
  }


  int j = startBlockPos_+1;
  //int startP = j;
  //Serial.print("StartP = ");
  //Serial.println(instream_[j]);
  for (int i = 0; i <= idx; ++i){
    int t = getType( j );
    //Serial.print("objecttype = "); Serial.println(t);
    if ( t == 1 ){
      res.objType_ = 1;
      int endBlockPos = instream_.find('}', j);
      
      if (endBlockPos == -1) {
        res.objType_ = -1;
        return res;
      }

      res.startBlockPos_ = j;
      endBlockPos = instream_.find(',', endBlockPos);
      
      if ( endBlockPos == -1)
        break; 

      j = endBlockPos+1;
      //Serial.println(instream_[j]);
      //startP = j;
    }
    
  }

  
  //Serial.print("Start pos"); Serial.println(res.startBlockPos_);
  return res;
}

Jsonsp Jsonsp::operator[](const char* inKey){
  //Serial.println("\n\n operator [char]\n");
  Jsonsp res(instream_);
  char endOfBlock = '}';
  int streamLength = instream_.find('}', startBlockPos_);

  //Serial.println(streamLength);

  if ( streamLength == -1 ){
    res.objType_ = -1;
    return res;
  }
  
  //Serial.println( getType() );
  if ( getType() != 1 ){
    res.objType_ = -1;
    return res;
  }
  
  int i = startBlockPos_;
  //Serial.println( i );
  
  while (i < streamLength){
    int pos1 = 0, pos2 = 0;
    int splitterPos = 0;
    int pos3 = 0, pos4 = 0;
  
    for (; i < streamLength; ++i){
      //Serial.print(instream_[i]);
      if ( isWhiteChar(instream_[i]) )
        continue;
  
      if ( instream_[i] == '\"' && pos1 == 0){
        pos1 = i;
        continue;
      }

      if ( instream_[i] == '\"' && pos1 != 0){
        pos2 = i;
        break;
      }

    }

    

    if ( pos1 == 0  || pos2 == 0 )
      break;

    
    for ( i = pos2; i < streamLength; ++i){
      if ( isWhiteChar(instream_[i]) )
        continue;

      if ( instream_[i] == ':' ){
        splitterPos = i;
        break;
      }
    }

    
    if ( splitterPos == 0 )
      break;
      
            
    const char key[pos2-pos1+2] = {0};
    strcpy( key, instream_.sub(pos1+1, pos2).c_str() );
    res.objType_ = 0;
    //Serial.print("key=");
    //Serial.println(key);
    //Serial.println(inKey);
    
    for (i = splitterPos; i < streamLength; ++i){
      //Serial.print(instream_[i]);
      if ( isWhiteChar(instream_[i]) )
        continue;
  
      if ( instream_[i] == '\"' && pos3 == 0){
        pos3 = i;
        continue;
      }

      if ( instream_[i] == '\"' && pos3 != 0){
        //Serial.println(key == inKey);
        if ( strcmp(key, inKey) == 0){
          res.objType_ = 3;
          pos4 = i;
          res.value_ = instream_.sub(pos3+1, pos4);
          break;
        }else{
          int k = instream_.find(',', i);
          if ( k != -1){
            i = k+1;
            break;
          }else{
            res.objType_ = -1;
            break;
          }
          
        }
        
      }

      if ( instream_[i] == '[' && pos3 == 0 ){
        if ( strcmp(key, inKey) == 0){
          res.objType_ = 2;
          res.startBlockPos_ = i;
          break;
        }
        else{
          int endB = instream_.find(']', i);
          if ( endB != -1 ){
            int k = instream_.find(',', i);
            if ( k != -1){
              i = k+1;
              break;
            }
          }
          
          res.objType_ = -1;
          break;
        }
          
        
      }

      if ( (instream_[i] == '{') && (pos3 == 0) ){
        if ( strcmp(key, inKey) == 0){
          res.objType_ = 1;
          res.startBlockPos_ = i;
          break;
        }
        else{
          int endB = instream_.find('}', i);
          if ( endB != -1 ){
            int k = instream_.find(',', i);
            if ( k != -1){
              i = k+1;
              break;
            }
          }
          
          res.objType_ = -1;
          break;
        }
    
      }

    

    
    }
    
    if ( res.objType_ == 0 )
      continue;

    if ( res.objType_ == 3 ){
      return res;
  }
    
    if ( res.objType_ == 1 || res.objType_ == 2 ){
      res.arrSize = res.size();
      return res;
    }
    

    break;
    
  }
  
  res.objType_ = -1;
  return res;
   
}

String Jsonsp::asString(){
  if ( objType_ != 3 )
    return "";

  return value_;
}

int Jsonsp::asInt(){
  if ( objType_ != 3 )
    return 0;

  return value_.toInt();
}

float Jsonsp::asFloat(){
  if ( objType_ != 3 )
    return 0;

  return value_.toFloat();
}

bool Jsonsp::asBool(){
  if ( objType_ != 3 )
    return false;

  if (value_ == "true")
    return  true;

  if (value_ == "false")
    return false;

  return value_.toInt();
}


