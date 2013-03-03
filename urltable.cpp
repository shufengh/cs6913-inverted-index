#include "urltable.h"
#include <cstdio>
#include <cstdlib>
#include <cerrno>
UrlTable* UrlTable::pTable = NULL;
unsigned int UrlTable::DEFAULT_SAVE_SIZE =500; // 8388608; // 8 MiB  
UrlTable::UrlTable(){
  beginDocID = 0;
  curDocCnt = 0;
}
UrlTable::~UrlTable(){
  this->saveTable();
}
bool UrlTable::saveTable(){
  char filename[66];
  sprintf(filename,"urltable//%d-%d",beginDocID, curDocCnt-1);

  FILE *outfile = fopen(filename,"w");
  if(!outfile){
    cout<<"fopen :"<<strerror(errno)<<endl;
    return false;
  }
  int ret = fwrite(docInfo.str().c_str(), 1, 
                   docInfo.str().length(), outfile);
  if (ret < (signed)docInfo.str().length()){
    cout<<"fwrite:"<<strerror(errno)<<endl;
    fclose(outfile);
    return false;
  }
  fclose(outfile);
  docInfo.str(string()); //delete contents currently in the stream
  beginDocID = curDocCnt;
  return true;
}
UrlTable* UrlTable::getInstance(){
  if(pTable == NULL)
    pTable = new UrlTable;
  return pTable;
}
int UrlTable::insert(string url, string filepath, int offset){
  bool ret = true;
  int id = curDocCnt;
  docInfo << curDocCnt++ << " "<< url<< " "<<filepath<<" "<<offset<<"\n";
  
  if (docInfo.str().size() >= DEFAULT_SAVE_SIZE){
    ret = this->saveTable();
  }
  if(ret == true)
    return id;
  return -1;
}
