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
    if(this->freq == 65535){ 
      cerr<<"lex reaches freq's upper bound"<<endl;
      return; 
    }
    freq += 1;
    int tpos = pos; // to save pos as the relative position
    pos = pos - lastpos;
    pos = pos > 8191 ? 8191:pos;  // keep pos not over 2^13-1 = 8191

    char str[8];
    sprintf(str," %c %d", cch, pos);
    pagehits.append(str);
    lastpos = tpos;

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
  stringstream ss;
  ss<<pagehits;
  string trash;
  unsigned counter=0;
  while(ss>>trash) ++counter;
  if(counter != ((unsigned int)this->freq)*2){
    cerr<<"C|P splice error "<<freq<<":"<<counter<<endl;
    return string();
  }
  
  char num[20];
  sprintf(num,"%d %d",this->docid, this->freq);
  hits.append("(");
  hits.append(num);
  hits.append(pagehits);
  hits.append(")");
  
  pagehits.clear();
  freq = 0;
  lastpos = 0;
  return hits;
  
}
