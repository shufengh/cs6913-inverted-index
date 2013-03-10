#ifndef __RECORD_H__
#define __RECORD_H__
#include "config.h"
#include <iostream>
using namespace std;
//#define __DEBUG__
class Record { //To save the freq and hits seperately, because we need to update the freq frequently in a page 
 public:
  int docid; 
  unsigned char freq;
  string pagehits; // [context:3 | pos:13]
  string hits;  // [docid:32 | freq:8 | [pagehits]]
  static char context[7]; // convert contexts into a num

  Record(int doctid, char cch, int pos);
  ~Record();
  void insert(int newDocid,unsigned char cch, int pos);
  static short convert(unsigned char cch);
  string recordToString();
};


#endif // __RECORD_H__
