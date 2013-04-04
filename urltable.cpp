#include "urltable.h"
#include <cstdio>
#include <cstdlib>
#include <cerrno>
UrlTable* UrlTable::pTable = NULL;
UrlTable::UrlTable(){
  curDocCnt = START_DOCID;
  Default_Save_Size = DEFAULT_SAVE_SIZE; 
}
UrlTable::~UrlTable(){
  this->saveTable();
}
bool UrlTable::saveTable(){
  cout<<"saveTable"<<endl;
  clock_t s = clock();
  gzFile *outfile =  (void **)gzopen("urltable/urltable","a");
  if (!outfile) {
    cerr<<"UrlTable::saveTable: gzopen of 'urltable/urltable' failed: "<<strerror(errno);
    return false;
  }
  string tmp = docInfo.str();
  int r = gzwrite(outfile, tmp.c_str(), tmp.length());
  if(r != (signed)tmp.length()){
    cerr<<"UrlTable::saveTable: gzwrite of 'urltable/urltable' failed:";
    cerr<<strerror(errno)<<endl;
    return false;
  }
  gzclose(outfile);
  docInfo.str(string()); //delete contents currently in the stream
  cout<<"saving time: "<<(double)(clock() - s)/CLOCKS_PER_SEC<<endl;

  return true;
}

UrlTable* UrlTable::getInstance(){
  if(pTable == NULL)
    pTable = new UrlTable;
  return pTable;
}

unsigned int UrlTable::getDocid(){
  return curDocCnt;
}

int UrlTable::insert(string url, string filepath, int offset, unsigned lexCnt){
  //  bool ret = true;
  //unsigned int id = curDocCnt;
  docInfo << curDocCnt++ << " "<< url<< " "<<filepath<<" "<<offset<<" "<<lexCnt<<"\n";
  return curDocCnt-1;
  
  // modify to save the urltable when the barrel saving happens
  //  docInfo.seekg(0, docInfo.end);
  // unsigned int len = docInfo.tellg();
  // docInfo.seekg(0, docInfo.beg);
  // if (len >= Default_Save_Size){
  //   ret = this->saveTable();
  // }
  // if(ret == true)
  //   return id;
  // return -1;
}
