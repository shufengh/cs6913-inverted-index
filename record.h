#ifndef __RECORD_H__
#define __RECORD_H__
#include "config.h"
#include <iostream>
#include <sstream>
using namespace std;
class Record { 
  //To save the freq and hits seperately, because we need to update the freq frequently in a page 
 public:
  static char context[7]; // convert the context into a num
  unsigned int docid; 
  unsigned short freq;
  string pagehits; // [context:3 | pos:13] in current page
  string hits;  // [docid:32 | freq:16 | [pagehits]]
  int lastpos;
  
  Record(int doctid, char cch, int pos);
  ~Record();
  void insert(unsigned int newDocid,unsigned char cch, int pos);
  static short convert(unsigned char cch);
  string recordToString();
};


#endif // __RECORD_H__
