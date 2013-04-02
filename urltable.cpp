#include "urltable.h"
#include <cstdio>
#include <cstdlib>
#include <cerrno>
UrlTable* UrlTable::pTable = NULL;
UrlTable::UrlTable(){
  beginDocID = START_DOCID;
  curDocCnt = START_DOCID;
  Default_Save_Size = DEFAULT_SAVE_SIZE; 
}
UrlTable::~UrlTable(){
  this->saveTable();
}
bool UrlTable::saveTable(){
  char filename[66]="";
  sprintf(filename,"urltable/%d-%d",beginDocID, curDocCnt-1);
 
  cout<<"saveTable"<<endl;

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
    outfile =NULL;
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
  //  bool ret = true;
  //unsigned int id = curDocCnt;
  docInfo << curDocCnt++ << " "<< url<< " "<<filepath<<" "<<offset<<"\n";
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
