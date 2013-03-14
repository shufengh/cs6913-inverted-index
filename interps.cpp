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

//#define __DEBUG__

#define INDEX_CHUNK 409600 //400KiB


char *memAlloc(gzFile *, int, bool readAll); 
void event(char *indexFName, char * dataFName,FwdIndex &findex, string& errlink);
void nz_op(string path);
void nz2_op();
int main (int argc, char * argv[])
{
// #ifdef __DEBUG__
//   cout<<"DEBUG MODE"<<endl;
// #else
//   cout<<"RELEASE_MODE"<<endl;
// #endif
  if(argc != 2){
    cout<<"format: interps datapath"<<endl;
    exit(1);
  }
  nz_op(argv[1]);
  return 0;

}
void nz_op(string path){
  FwdIndex findex;
  string errlink;
  //sort third field with '/' as the seperator
  string tmp = "ls "+ path + "/*_* | sort -k4 -t/ -n";
  FILE *handle = popen(tmp.c_str(), "r");
  if( handle == NULL){
    cerr<<"popen error"<<endl;
    return ;
  }  
  string fnames2;
  char buff[128];
  while ((fread(buff, 127,1, handle)) != 0){
    fnames2.append(buff);
  }
  pclose(handle);
  stringstream ns(fnames2);
  double totaltime = 0;
  string data, index;
  //event("nz2_merged/65_index","nz2_merged/65_data", findex, errlink);
  while(!ns.eof()){
    ns>>data>>index;
    cout<<"visiting "<<index;
    clock_t beg = clock();
    event(const_cast<char*>(index.c_str()), const_cast<char*>(data.c_str()), findex, errlink);
    clock_t end = clock();
    double time = (double)(end-beg)/CLOCKS_PER_SEC;
    cout<<" "<<time<<endl ;
    totaltime += time;
  }
  cout<<"total time: "<<totaltime<<endl;
  UrlTable::getInstance()->saveTable();

}
void nz2_op(){
  FwdIndex findex;
  string errlink;
  //M2 for nz2_merged
  FILE *handle = fopen("nz2_merged/indexfile_list.txt","r");
  if(!handle){
    cerr<<"main fopen error:"<<strerror(errno)<<endl;
    return ;
  }
  string fnames;
  char buf[128];
  string fprefix;

  while (fgets(buf, 127,  handle) != NULL){
    buf[127] = '\0';
    fnames.append(buf);
  }
  fclose(handle);
  stringstream ns(fnames);
  double totaltime = 0;
  //event("nz2_merged/65_index","nz2_merged/65_data", findex, errlink);
  while(!ns.eof()){
    ns>>fprefix;
    char indexFName[48];// = "nz2_merged/1_index";
    char dataFName[48];// = "nz2_merged/1_data";
    sprintf(indexFName, "%s_index", fprefix.c_str());
    sprintf(dataFName, "%s_data", fprefix.c_str());
    cout<<"visiting "<<indexFName<<" "<<dataFName;
    clock_t beg = clock();
    event(indexFName, dataFName, findex, errlink);
    clock_t end = clock();
    double time = (double)(end-beg)/CLOCKS_PER_SEC;
    cout<<" "<<time<<endl ;
    totaltime += time;
  }
  cout<<"total time: "<<totaltime<<endl;
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

  indexBuffer=memAlloc(cIndex, INDEX_CHUNK, true);
  if (indexBuffer == NULL){
    errlink.append("indexbuffer allocate error\n");
    gzclose(cIndex);
    gzclose(cData);
    return;
  }

  string url;
  int offset = 0;
  string trash;
  char *pLine = strtok(indexBuffer, "\n");
  while (pLine != NULL){
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

    char *pageBuf=NULL, *lexbuf=NULL;
    pageBuf = memAlloc(cData, offset, false);
    //    pageBuf = (char*)realloc(pageBuf, offset+1);
    //pageBuf[offset] = '\0';
    if (pageBuf != NULL){
        lexbuf = (char*)malloc(2*offset + 1);
        int ret = parser(const_cast<char*>(url.c_str()), pageBuf, lexbuf, 2*offset + 1, offset);
        if (ret > 0){
          findex.insertParsingRes(docID, lexbuf);
        }
        else errlink.append(url+"parse error\n");
        
        free(pageBuf);
        free(lexbuf);
    }
    else errlink.append(url+" memAlloc error\n");

    pLine = strtok (NULL, "\n");
  }

  free(indexBuffer);
  gzclose(cData);
  gzclose(cIndex);
}

char *memAlloc(gzFile *fileName, int size, bool readAll = true){
    char *buffer=(char *)malloc(size);
    int oldSize=size;
    int count=0;             //The number of bytes that already read

    while (!gzeof(fileName)){ 
      int cur = gzread(fileName,buffer+count,oldSize);        
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

