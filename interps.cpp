#include<iostream>
#include<sstream>
#include<zlib.h>
#include<errno.h>
#include<ctime>
#include "config.h"
#include "parserra.h"
#include "urltable.h"
#include "fwdindex.h"
using namespace std;

#define INDEX_CHUNK 1048576 //409600 //400KiB

char *memAlloc(gzFile *, int, bool readAll); 
void event(char *indexFName, char * dataFName,FwdIndex &findex, string& errlink);
void nz_op(char* path);
void nz2_op();
string get_fnames(char* path);
int main (int argc, char * argv[]){
  if(argc != 2){
    cout<<"format: interps datapath"<<endl;
    exit(1);
  }
  clock_t beg = clock();
  nz_op(argv[1]);
  cout<<"total time: "<<(double)(clock() - beg)/CLOCKS_PER_SEC<<endl;
  return 0;
}
string get_fnames(char* path){
  string cmd("ls ");
  cmd.append(path);

  // sort numerically (-n) on the third field (-k3) using 'p' 
  // as the field separator (-tp).
  cmd.append("/*_* | sort -k2 -t/ -n");
  FILE *handle = popen(cmd.c_str(), "r");
  if(handle == NULL){
    cerr<<"popen error"<<endl;
    return NULL;
  }
  string fnames;
  char buff[1024];
  while ((fgets(buff, 1023, handle)) != NULL){
    fnames.append(buff);
  } 
  pclose(handle);
  return fnames;
}

void nz_op(char *path){
  FwdIndex findex;
  string errlink;
  stringstream ns(get_fnames(path));
  string data, index;

  while(!ns.eof()){
    ns>>data;
    if(data.find("data") == string::npos || ns.eof()) continue;
    ns>>index;
    if(index.find("index") == string::npos) continue;
    cout<<"visiting "<<index;
    clock_t beg = clock();
    event(const_cast<char*>(index.c_str()), const_cast<char*>(data.c_str()), 
          findex, errlink);
    cout<<" "<<(double)(clock() - beg)/CLOCKS_PER_SEC<<endl;
  }

  ofstream errlk("errlink");
  errlk<<errlink;
  errlk.close();
  UrlTable::getInstance()->saveTable();


}

void event(char *indexFName, char * dataFName,FwdIndex &findex, string& errlink){
  gzFile *cData, *cIndex;
  char *indexBuffer = NULL;
  
  cIndex = (void **)gzopen(indexFName, "r");
  if (!cIndex) {
    fprintf (stderr, "gzopen of '%s' failed: %s.\n", indexFName,
             strerror (errno));
    return;
  }
  cData =  (void **)gzopen(dataFName,"r");
  if (!cData) {
    fprintf (stderr, "gzopen of '%s' failed: %s.\n", dataFName,
             strerror (errno));
    return;
  }

  indexBuffer = memAlloc(cIndex, INDEX_CHUNK, true);
  if (indexBuffer == NULL){
    
    errlink.append("indexbuffer allocate error\n");
    gzclose(cIndex);
    gzclose(cData);
    return;
  }

  string url;
  int offset = 0;
  string trash;
  stringstream ssindex(indexBuffer);
  string pLine; 
  
  while (!ssindex.eof()){

    getline(ssindex, pLine);
    stringstream ss(pLine);
    for(int i = 0; i < 4; ++i){
      if(i == 0)
        ss >> url;
      else if (i == 3)
        ss >> offset;
      else 
        ss >> trash;
    }

    int docID = UrlTable::getInstance()->insert(url, dataFName, offset);
    if (docID == -1){
      cerr<<"urltable insert error"<<endl;
      break;
    }

    char *pageBuf = memAlloc(cData, offset, false);

    if (pageBuf != NULL){
        char *lexbuf = (char*)malloc(2*offset + 1);
        int ret = parser(const_cast<char*>(url.c_str()),
                         pageBuf, lexbuf, 2*offset + 1, offset);
        if (ret > 0) findex.insertParsingRes(docID, lexbuf);
        else {
          if (ret == 0)
            errlink.append(url+" return code != 200\n");
          else
            errlink.append(url+" parse error\n");
        }
        
        free(pageBuf);
        free(lexbuf);
        pageBuf = lexbuf = NULL;
    }
    else{ 
      cout<<endl<<url+" memAlloc error\n";
      errlink.append(url+" memAlloc error\n");
    }

  }

  free(indexBuffer);
  gzclose(cData);
  gzclose(cIndex);
}

char *memAlloc(gzFile *fileName, int size, bool readAll = true){
  char *buffer= (char *)malloc(size);
  int oldSize=size;
  int count=0; //The number of bytes that already read
  
  while (!gzeof(fileName)){ 
    int cur = gzread(fileName,buffer+count, oldSize);        
    if (cur < oldSize){
      if (!gzeof (fileName)){
        const char * error_string;
        int err;
        error_string = gzerror (fileName, & err);
        if (err) {
          fprintf (stderr, "Error: %s.\n", error_string);
          free(buffer);
          buffer = NULL;
          return buffer;
        }
      }
    }
    count += cur;
    if(readAll == false)
      break;
    if (count == size){                    // Reallocate when buffer is full
      oldSize=size;
      size*=2;
      buffer=(char *)realloc(buffer,size);
      }
  }
  return buffer;
}

