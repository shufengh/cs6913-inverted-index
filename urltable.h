#ifndef __URLTABLE_H__
#define __URLTABLE_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "config.h"
#include "gzstream.h"
using namespace std;

/*
  A singleton class to provide urlTable operations
 */

class UrlTable{
 private:
 static UrlTable* pTable;
 unsigned int Default_Save_Size;
 unsigned int beginDocID; // give the saving file a name containing starting and ending docID
 unsigned int curDocCnt; // for a new doc ID
 stringstream docInfo; // url info
 UrlTable();
 public:
 ~UrlTable();
 static UrlTable* getInstance();
 int insert(string url, string filepath, int offset, unsigned lexCnt); //return docID
 bool saveTable();
 unsigned int getDocid();
};

#endif //__URLTABLE_H__
