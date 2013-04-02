#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include "gzstream.h"
#include <cerrno>
#include <time.h>
#include "record.h"
using namespace std;

string get_fnames(char* path);
void format(string fname, ogzstream& gzlex, ofstream &outfmd, ofstream &lexchunk);
unsigned int convert(string record, vector<unsigned char> &listbuf,
                     unsigned int &listItr); // return docid
void pairTo2Bts(unsigned int num, unsigned char ch, vector<unsigned char>& listbuf,
                unsigned int &listItr);
void vb_decode(vector<unsigned char> &listbuf);
void vb_encode(unsigned int docid, vector<unsigned char> &listbuf, unsigned int &listItr);

int main(int argc, char** argv){
  if( argc != 2){
    cout<<"format: formatter indexpath"<<endl;
    exit(0);
  }
  string flex = string(argv[1]) + "/lexicon.gz";
  string flist = string(argv[1]) + "/i2list";
  string chunkname = string(argv[1]) + "i2chunk";
  ogzstream gzlex(flex.c_str());
  if ( ! gzlex.good() ) {
    cerr << "ERROR: Opening file '" << flex << "' failed.\n";
    exit(1);
  }
  ofstream outfmd(flist.c_str(), ofstream::binary);
  if ( ! outfmd.good() ) {
    cerr << "ERROR: Opening file '" << flist << "' failed.\n";
    exit(1);
  }
  ofstream lexchunk(chunkname.c_str());
  if ( ! lexchunk.good() ) {
    cerr << "ERROR: Opening file '" << chunkname << "' failed.\n";
    exit(1);
  }

  clock_t beg = clock();

  string fnames = get_fnames(argv[1]);
  stringstream ss(fnames);
  while(!ss.eof()){
    string name;
    ss>>name;
    cout<<"Enter "<<name<<endl;
    clock_t s = clock();
    if (!name.empty())
      format(name, gzlex, outfmd, lexchunk);
    cout<<"Exit "<<name<<" time usage: "<<(double)(clock()-s)/CLOCKS_PER_SEC<<endl;
  }
  
  gzlex.close();
  outfmd.close();
  lexchunk.close();
  cout<<"Total time: "<<(double)(clock()-beg)/CLOCKS_PER_SEC<<endl;
  return 0;
}
void format(string fname, ogzstream& gzlex, ofstream &outfmd, ofstream &lexchunk){
  igzstream gzin(fname.c_str());
  if ( ! gzin.good() ) {
    cerr << "First round ERROR: Opening file '" << fname << "' failed.\n";
    return;
  }

  string line, fmdbuf;
  int start = -1, end = -1;
  unsigned docCnt = 0; // how many docs in a posting
  stringstream lexbuf;
  vector<unsigned char> listbuf(10000000,0);
  unsigned int listItr = 0;
  stringstream lexchkbuf; // save the last docid in a chunk and current size

  long long proc = 0; // to record the process stage
  while(!gzin.eof()){
    getline(gzin, line);
    string word = line.substr(0, line.find_first_of(" "));
    lexbuf<<line.substr(0, line.find_first_of(" "))<<" ";
    unsigned int startOffset = listItr;
    int startChkOffset = lexchkbuf.tellp();
    unsigned int docid = 0;
    unsigned int nextStart = 0; 
    do {

      // if( proc++ % 100000 == 0)
      //   cout<<proc<<" lex size:"<<lexbuf.tellp()<<", list size:"<<listItr<<":"<<10000000<<endl;
      
      start = line.find_first_of("(", nextStart);
      end = line.find_first_of(")", start);
      if( start != -1 && end != -1){
        // start+1, end-start-1 to omit '(' and ')'
        docid = convert(line.substr(start+1, end-start-1), listbuf, listItr);

        //        line = line.substr(end+1);
        nextStart = end + 1;
        start = end = -1;
      }
      else break;
      
      if (docCnt > 0 && docCnt % 128 == 0){
        lexchkbuf<<docid<<" "<<listItr-startOffset<<" ";
        //        cout<<lexchkbuf.str()<<endl;
      }      
    }while(++docCnt);

    // the last part chunk of the posting
    if (docCnt % 128 != 0)
      lexchkbuf<<docid<<" "<<listItr-startOffset<<" ";
    if (proc == 100000)
      cout<<proc++<<" "<<word<<" "<<docCnt<<endl;
    lexbuf<<docCnt<<" "; // Ft
    docCnt = 0;
    // posting offset and chunk record offset
    lexbuf<<(listItr-startOffset)<<" ";
    lexbuf<<((int)lexchkbuf.tellp()-startChkOffset)<<"\n"; // record the size of the current chunk

    // if(lexchkbuf.tellp() > 5242880){ // save the chunk info
    //   lexchunk.write(lexchkbuf.str().c_str(), (int)lexchkbuf.tellp());
    //   lexchkbuf.str(string());

    // } 
    // if(lexbuf.tellp() > 5242880){ // 5MiB
    //   gzlex<<lexbuf.str()<<flush;
    //   lexbuf.str(string());
    // }

    if(listItr > 5242880){
      outfmd.write(reinterpret_cast<const char*>(listbuf.data()), listItr);
      listbuf.clear();
      listItr = 0; 
      // save at the same time to ensure no data loss
      lexchunk.write(lexchkbuf.str().c_str(), (int)lexchkbuf.tellp());
      lexchkbuf.str(string());

      gzlex<<lexbuf.str()<<flush;
      lexbuf.str(string());
    }
  } //while(!gzin.eof()) ends

  gzin.close();

  lexchunk.write(lexchkbuf.str().c_str(), (int)lexchkbuf.tellp());
  gzlex<<lexbuf.str()<<flush;
  //vector.data() returns the pointer to raw data
  outfmd.write(reinterpret_cast<const char*>(listbuf.data()), listItr); 
  listbuf.clear();
  listItr = 0;
}

unsigned int convert(string record, vector<unsigned char> &listbuf,
                     unsigned int &listItr){
  //cout<<record<<endl;
  stringstream ss(record);
  // record format: [docid freq context pos context pos .. ..]
  string str; // get chars from stream first
  unsigned char ch; // freq or context
  unsigned int num, pos; // docid 
  
  ss>>str;
  num = atoi(str.c_str());
  if(str.size() > 1 && num == 0){
    cout<<"formatter: docid converting error "<<record<<endl;
    return -1;
  }
  vb_encode(num, listbuf, listItr);
  ss>>str;
  //ch = str[0];
  listbuf[listItr++] = atoi(str.c_str());
  while(!ss.eof()){
    ss>>str;
    ch = str[0];
    ss>>str;

    pos = atoi(str.c_str());
    if(str.size() > 1 && pos == 0){
      cout<<"formatter: pos converting error "<<str<<","<<record<<endl;
      return -1;
    }
    // a test to see if vbytes is better than the byte alignment
    pairTo2Bts(pos, ch, listbuf, listItr); 
    //vb_encode(Record::convert(ch)<<13 + pos, listbuf, listItr);
  }
  return num; //return docid
}
void vb_encode(unsigned int docid, vector<unsigned char>&listbuf, 
               unsigned int &listItr){
  unsigned char bits[6]="";
  short i = -1;
  while(docid != 0){
    bits[++i] = docid % 128;
    docid >>= 7;
  }

  for(; i > 0; --i){
    // 0x80 set the highest bit  
    listbuf[listItr++] = ((short)bits[i] | 0x80); 
  }
  listbuf[listItr++] = bits[0];

}
void vb_decode(vector<unsigned char> &listbuf){
  unsigned int num;
  for(vector<unsigned char>::iterator itr = listbuf.begin();
      itr != listbuf.end(); ++ itr){
    // cout<<"IN:"<<int(*itr)<<endl;  
    if(*itr > 127){
      num |= *itr & 0x7f; //clear the highest bit
      //cout<<num<<endl;
      num <<= 7;
    }
    else{
      num |= *itr;
      cout<<"decode:"<<num<<endl;
      num = 0;
    }
  }
}

void pairTo2Bts(unsigned int pos, unsigned char ch, vector<unsigned char>& listbuf,
                unsigned int &listItr){
  short hit = 0;
  short chID = Record::convert(ch);
  hit = ((chID<<13) | pos);
  //cout<<hit<<' ';
  listbuf[listItr++] = ((hit>>8) & 0xff); // save the high 8 bits
  listbuf[listItr++] = (hit & 0xff);
}
string get_fnames(char* path){
  string cmd("ls ");
  cmd.append(path);
  // sort numerically (-n) on the third field (-k3) using 'p' 
  // as the field separator (-tp).
  cmd.append("/temp* | sort -k3 -tp -n"); 
  cout<<cmd<<endl;
  FILE *handle = popen(cmd.c_str(), "r");
  if(handle == NULL){
    cerr<<"popen error"<<endl;
    return NULL;
  }
  
  string fnames;
  char buff[128];
  while ((fgets(buff,127,handle)) != NULL){
    fnames.append(buff);
  } 
  pclose(handle);
  return fnames;
}
