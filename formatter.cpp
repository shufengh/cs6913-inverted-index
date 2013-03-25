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
void format(string fname, ogzstream& gzlex, ofstream &outfmd);
void convert(string record, vector<unsigned char> &listbuf);
void pairTo2Bts(unsigned int num, unsigned char ch, vector<unsigned char>& listbuf);
void vb_decode(vector<unsigned char> &listbuf);
void vb_encode(unsigned int docid, vector<unsigned char> &listbuf);
int main(int argc, char** argv){
  if( argc != 2){
    cout<<"format: formatter indexpath"<<endl;
    exit(0);
  }
  string flex = string(argv[1]) + "/lexicon.gz";
  string flist = string(argv[1]) + "/i2list";
  ogzstream gzlex(flex.c_str());
  if ( ! gzlex.good() ) {
    cerr << "ERROR: Opening file '" << flex << "' failed.\n";
    exit(1);
  }
  ofstream outfmd(flist.c_str(), ofstream::binary);
  if ( ! outfmd.good() ) {
    cerr << "ERROR: Opening file '" << outfmd << "' failed.\n";
    exit(1);
  }
  clock_t beg = clock();
  string fnames = get_fnames(argv[1]);
  stringstream ss(fnames);
  while(!ss.eof()){
    string name;
    ss>>name;
    format(name, gzlex, outfmd);
  }
  gzlex.close();
  outfmd.close();
  cout<<"Total time: "<<(double)(clock()-beg)/CLOCKS_PER_SEC<<endl;
  return 0;
}
void format(string fname, ogzstream& gzlex, ofstream &outfmd){
  igzstream gzin(fname.c_str());
  if ( ! gzin.good() ) {
    cerr << "First round ERROR: Opening file '" << fname << "' failed.\n";
    return;
  }
  string line, fmdbuf;
  int start = -1, end = -1;
  unsigned docCnt = 0; // how many docs in a posting
  stringstream lexbuf;
  vector<unsigned char> listbuf;
  listbuf.resize(10000000);
  int proc = 1;
  while(!gzin.eof()){
    getline(gzin, line);
    if( proc++ % 10000 == 0)
      cout<<"lex size:"<<lexbuf.tellp()<<"/list size:"<<listbuf.size()<<":"<<10000000<<endl;
    //cout<<listbuf.size()<<":"<<20000000<<endl;
    //cout<<line.substr(0, line.find_first_of(" "));
    lexbuf<<line.substr(0, line.find_first_of(" "))<<" ";
    unsigned startOffset = listbuf.size();
    while(++docCnt){
      start = line.find_first_of("(");
      end = line.find_first_of(")");
      if( start != -1 | end != -1){
        // start+1, end-start-1 to omit '(' and ')'
        convert(line.substr(start+1, end-start-1), listbuf);
        line = line.substr(end+1);
        start = end = -1;
      }
      else break;
    } // while(++docCnt) ends
    //cout<<":"<<docCnt<<endl;
    lexbuf<<docCnt<<" "; // Ft
    docCnt = 0;
    lexbuf<<(listbuf.size()-startOffset)<<"\n"; // word offset
    if(lexbuf.tellp() > 10000000){
      gzlex<<lexbuf.str()<<flush;
    }
    if(listbuf.size() > 10000000){
      outfmd.write(reinterpret_cast<const char*>(listbuf.data()), listbuf.size());
      listbuf.clear();
    }
  } //while(!gzin.eof()) ends
  gzin.close();
  gzlex<<lexbuf.str()<<flush;
  outfmd.write(reinterpret_cast<const char*>(listbuf.data()), listbuf.size());  //vector.data() returns the pointer to raw data
  listbuf.clear();
  cout<<"Finish "<<fname<<endl;
}

void convert(string record, vector<unsigned char> &listbuf){
  //cout<<record<<endl;
  stringstream ss(record);
  // record format: [docid freq context pos context pos .. ..]
  string str; // get chars from stream first
  unsigned char ch; // freq or context
  unsigned int num; // docid or pos 
  
  ss>>str;
  num = atoi(str.c_str());
  if(str.size() > 1 && num == 0){
    cout<<"formatter docid converting error"<<endl;
    return;
  }
  vb_encode(num, listbuf);
  ss>>str;
  ch = str[0];
  listbuf.push_back(ch);
  while(!ss.eof()){
    ss>>str;
    ch = str[0];
    ss>>str;

    num = atoi(str.c_str());
    if(str.size() > 1 && num == 0){
      cout<<"formatter docid converting error"<<endl;
      return;
    }

    pairTo2Bts(num, ch, listbuf);
  }
}
void vb_encode(unsigned int docid, vector<unsigned char>&listbuf){
  unsigned char bits[6]="";
  short i = -1;
  while(docid != 0){
    bits[++i] = docid % 128;
    docid >>= 7;
  }

  for(; i > 0; --i){
    // 0x80 set the highest bit  
    listbuf.push_back( ((short)bits[i] | 0x80)); 
    //cout<<((short)bits[i])<<endl;
  }
  listbuf.push_back(bits[0]);
  //cout<<((short)bits[0])<<endl;
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
void pairTo2Bts(unsigned int pos, unsigned char ch, vector<unsigned char>& listbuf){
  short hit = 0;
  short chID = Record::convert(ch);
  hit = (chID<<13) | pos;
  listbuf.push_back((hit>>8) & 0xff); // save the high 8 bits
  listbuf.push_back(hit & 0xff);
}
string get_fnames(char* path){
  string cmd("ls ");
  cmd.append(path);
  // sort numerically (-n) on the third field (-k3) using 'p' as the field separator (-tp).
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
