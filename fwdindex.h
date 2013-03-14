#ifndef __FWDINDEX_H__
#define __FWDINDEX_H__
#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>
#include <tr1/unordered_map>
#include <sstream>
#include <zlib.h>
#include <errno.h>
#include "config.h"
#include "record.h"
using namespace std;

// #define __DEBUG__
/* This class is to create the forward indexes for following merging op. 
   Since most of the ops are merging same words' contexts and position,
   I use the map with words as keys and two bytes saving the context and position. */


class FwdIndex{
 private:
  int BarrelSize;
  char savePrefix[20];
  int nameCnt;
  int curBarrelSize;
  string strchunk;
  map<string, Record> FwdIndexBarrel; 
  // [docid:32|freq:1|context:1|pos:15|context:1|pos:15...]
  
public:
  FwdIndex();
  ~FwdIndex();
  void insertParsingRes(int docid, char *lexbuf);
  void saveIntoDisk();

};

#endif //__FWDINDEX_H_
