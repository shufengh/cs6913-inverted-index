#include "record.h"
char Record::context[7] = "BHIPTU";

Record::Record(int docid, char cch, int pos){
  this->docid = docid;
  this->freq = 0;
  lastpos = 0;
  insert(this->docid, cch, pos);
}
Record::~Record(){
  
}

void Record::insert(unsigned int newDocid, unsigned char cch, int pos){

  if (this->docid != newDocid){
    this->recordToString();
    this->docid = newDocid;
    insert(this->docid, cch, pos);
  }
  else{
    // if a word appears 255 times in a page, I get two conclusions: 
    // the word is not important; the author is bored. 
    if(freq == 255) 
      return; 
    freq = 1 + freq - '\0';
    int tpos = pos; // to save pos as the relative position
    pos = pos - lastpos;
    pos = pos > 8191 ? 8191:pos;  // keep pos not over 2^13-1 = 8191

    char str[8];
    sprintf(str," %c %d", cch, pos);
    pagehits.append(str);
    lastpos = tpos;
// #else
//     short hit = 0; //[chID:3 | pos:13] 
//     char chit[3]="";
//     short chID = convert(cch);
//     hit = (chID<<13) | pos;
//     chit[0] = (hit>>8) & ~(~0<<8); // save the high 8 bits
//     chit[1] = hit & ~(~0<<8);
//     chit[2] = 0;
//     pagehits.append(chit);
// #endif

  }
}
short Record::convert(unsigned char cch){
  short chID = -1; //convert the context cch into a num
  for(unsigned i = 0; i< strlen(context); ++i){
    if (context[i] == cch){
      chID = i;
      break;
    } 
  }
  if (chID == -1){
    //cout<< "bad context in FwdIndex::insertParsingRes, set it plain"<<endl;
    chID = 3;
  }
  return chID;
}
string Record::recordToString(){
  //splice the record properties into a string and merge pagehits into hits

  char num[20];
  sprintf(num," (%d %d ",this->docid,(int)freq);
  hits.append(num);
  hits.append(pagehits);
  hits.append(")");

  pagehits.clear();
  freq = 0;
  lastpos = 0;
  return hits;
}
