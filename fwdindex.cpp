#include "fwdindex.h"

FwdIndex::FwdIndex(){
  curBarrelSize = 0;
  BarrelSize = BARREL_SIZE; 
  strcpy(savePrefix,"fwdbarrel/temp");
  nameCnt = 0;
}
FwdIndex::~FwdIndex(){
  // save what's still in the barrel and say bye
  saveIntoDisk();
}
void FwdIndex::insertParsingRes(int docid, char *lexbuf){
  string word;
  char cch[7]; 
  int pos = 0;
  int lastpos = 0;
  curBarrelSize += strlen(lexbuf);
  string tmp(lexbuf);
  stringstream ss(tmp);
  while(!ss.eof()){
    ss>>word>>cch>>pos;
    if (word.size()>24){
      word = word.substr(0,24);
    }
    map<string, Record*>::iterator itr = FwdIndexBarrel.find(word);
    if (itr == FwdIndexBarrel.end()){
      // currently deal with single context only
      Record *rd = new Record(docid, cch[0], pos-lastpos);
      FwdIndexBarrel[word] = rd; // insert a new node
    }
    else{
      FwdIndexBarrel[word]->insert(docid, cch[0], pos-lastpos); //append
    }
  }
  lastpos = pos;
  // cout<<curBarrelSize<<":"<<BarrelSize<<endl;
  if (curBarrelSize > BarrelSize)
    saveIntoDisk();
}
void FwdIndex::saveIntoDisk(){
  map<string, Record*>::iterator itr;
  for(itr = FwdIndexBarrel.begin(); itr!=FwdIndexBarrel.end();++itr){
    string tmp;
    if((itr->second) != NULL)
      tmp.append(itr->second->recordToString());
    else continue;
    if(tmp.size() == 0){
      cout<<"tmp.size() == 0 ";
      continue;
    }
    char name[25]="";
#ifdef __DEBUG__
    sprintf(name,"%s ",itr->first.c_str());
#else
    sprintf(name,"%24s",itr->first.c_str());
#endif
    cout<<name<<":"<<itr->second->docid<<endl;
    strchunk.append(name);
    strchunk.append(tmp);
    strchunk.append("\n");

    delete itr->second;
    itr->second = NULL;
    FwdIndexBarrel.erase(itr);
  }
  if (strchunk.size() == 0) {
    cout<< "fwdindex.cpp: strchunk.size() = 0"<<endl;
    return; 
  }

  char fname[20];
  sprintf(fname,"%s%d", savePrefix, nameCnt);
  ++nameCnt;
  gzFile *fi = (void **)gzopen(fname, "wb");
  if (!fi) {
    fprintf (stderr, "gzopen of '%s' failed: %s.\n", fname,
             strerror (errno));
    return;
  }
  
  // cout<<"strchunk:"<<strchunk<<endl;
  int r = gzwrite(fi, strchunk.c_str(), strchunk.size());
  if(r < (signed)strchunk.size()){
    cerr<<"fwdindex.cpp: gzwrite of "<<fname<<" failed:"<<strerror(errno)<<endl;
  }
  gzclose(fi);
  strchunk.clear();
  curBarrelSize = 0;
}