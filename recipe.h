#ifndef __RECIPE_H_
#define __RECIPE_H_

struct node_{
public:
  String name_;
  byte incT_;
  byte maintainT_;
  bool keyEvent_;
  //node_* next_;
};

template<int N>
class Recipe{
public:
  String name_;
  node_ phases_[N];
  int size_ = 0;
  
};

#endif 

