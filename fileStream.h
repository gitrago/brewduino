#ifndef __FILESTREAM_H_
#define __FILESTREAM_H_

#include <SD.h>

class FileStream{
  public:
  FileStream(File*  f):file_(f){}
  char operator[](int pos){

    file_->seek(pos);
    return file_->peek();
  }

  int size(){
    return file_->size();
  }

  String sub(int startPos, int endPos){
    file_->seek(startPos);
    int len = endPos-startPos;
    char buf[len+1] ={0};
    file_->read(buf, len);
    return buf;
  }

  int find(char c, int startPos = 0){
    int len = this->size();
    
    for (int i = startPos; i < len; ++i)
      if ( (*this)[i] == c )
        return i;

    return -1;
  }
  
  File* file_;
};

#endif 

