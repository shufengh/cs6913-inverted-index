#ifndef __URLTABLE_H__
#define __URLTABLE_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/*
  A singleton class to provide urlTable operations
 */

class UrlTable{
 private:
 static UrlTable* pTable;
 static unsigned int DEFAULT_SAVE_SIZE;
 unsigned int beginDocID; // give the saving file a name containing starting and ending docID
 unsigned int curDocCnt; // for a new doc ID
 stringstream docInfo; // url info
 UrlTable();
 bool saveTable();
 public:
 ~UrlTable();
 static UrlTable* getInstance();
 int insert(string url, string filepath, int offset); //return docID
 
};

#endif //__URLTABLE_H__
