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
void FwdIndex::insertParsingRes(int docid, char *lexbuf){
    // if ( docid == 277)
    //    cout<<lexbuf<<" ";
  string word;
  char cch[7]; 
  int pos = 0;
  curBarrelSize += strlen(lexbuf);
  string tmp(lexbuf);
  stringstream ss(tmp);
  while(ss>>word>>cch>>pos){
    //  ss>>word>>cch>>pos;
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
  }
  //cout<<curBarrelSize<<":"<<BarrelSize<<endl;
  if (curBarrelSize > BarrelSize)
    saveIntoDisk();
}
void FwdIndex::saveIntoDisk(){
  cout<<endl<<"saveIntoDisk"<<endl;
  if(FwdIndexBarrel.size() == 0){
    cout<<endl<<"FwdIndexBarrel empty"<<endl;
    return;
  }
  //char name[25]="";
  map<string, Record>::iterator itr;
  for(itr = FwdIndexBarrel.begin(); itr!=FwdIndexBarrel.end();++itr){
    //    sprintf(name,"%s ",itr->first.c_str());
    strchunk.append(itr->first);
    strchunk.append(itr->second.recordToString());
    strchunk.append("\n");
  }
  
  char fname[20];
  sprintf(fname,"%s%d", savePrefix, nameCnt);
  ++nameCnt;
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

  FwdIndexBarrel.clear();
  strchunk.clear();
  curBarrelSize = 0;
  
  UrlTable::getInstance()->saveTable(); // keep saving urltable and temp the same step to recover easily
    

}
