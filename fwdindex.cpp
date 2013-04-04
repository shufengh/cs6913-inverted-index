#include "fwdindex.h"

FwdIndex::FwdIndex(){
  curBarrelSize = 0;
  BarrelSize = BARREL_SIZE;
  strcpy(savePrefix,"fwdbarrel/temp");
  nameCnt = START_TEMP_NUM;
}
FwdIndex::~FwdIndex(){
  // save what's still in the barrel and say bye
  saveIntoDisk();
}
unsigned FwdIndex::insertParsingRes(int docid, char *lexbuf){
  string word;
  char cch[7]; 
  int pos = 0;
  unsigned int lexCnt = 0;

  curBarrelSize += strlen(lexbuf);
  stringstream ss(lexbuf);
  while(ss>>word>>cch>>pos){

    if(word.size() > 24){
      word = word.substr(0, 24);
    }
    map<string, Record>::iterator itr = FwdIndexBarrel.find(word);
    if (itr == FwdIndexBarrel.end()){
      // currently deal with single context only
      //Record rd(docid, cch[0], pos-lastpos);
      FwdIndexBarrel.insert(std::pair<string, Record>(word, Record(docid, cch[0], pos)));
    }
    else{
      itr->second.insert(docid, cch[0], pos); //append
    }
    ++lexCnt;
  }

  if (curBarrelSize > BarrelSize)
    saveIntoDisk();
  
  return lexCnt;
}

void FwdIndex::saveIntoDisk(){

  if(FwdIndexBarrel.size() == 0){
    cout<<endl<<"FwdIndexBarrel empty"<<endl;
    return;
  }
  char fname[20];
  sprintf(fname,"%s%d", savePrefix, nameCnt++);
  cout<<endl<<"save into "<<fname<<endl;
  map<string, Record>::iterator itr;
  for(itr = FwdIndexBarrel.begin(); itr!=FwdIndexBarrel.end();++itr){
    string tmp = itr->second.recordToString();
    if( tmp.length() != 0)
      strchunk.append(itr->first + " " + tmp + "\n");
  }
  

  clock_t s = clock();
  gzFile *fi = (void **)gzopen(fname, "wb");
  if (!fi) {
    cerr<<"fwdindex.cpp: gzopen of "<<fname<<" failed: "<<strerror(errno);
    return;
  }
  int r = gzwrite(fi, strchunk.c_str(), strchunk.size());
  if(r < (signed)strchunk.size()){
    cerr<<"fwdindex.cpp: gzwrite of "<<fname<<" failed:"<<strerror(errno)<<endl;
  }
  gzclose(fi);
 
  // ogzstream outfile(fname);
  // if(!outfile.good()){
  //   cerr<<" fwdindex: savetodisk error open "<<fname<<endl;
  // }
  // outfile<<strchunk;
  // outfile.close();
  cout<<"saving time: "<<(double)(clock() - s)/CLOCKS_PER_SEC<<endl;
  FwdIndexBarrel.clear();
  strchunk.clear();
  curBarrelSize = 0;
  
  // keep saving urltable and temp the same step to recover easily
  UrlTable::getInstance()->saveTable(); 
  
}
