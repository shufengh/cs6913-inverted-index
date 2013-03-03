#include<iostream>
#include<sstream>
#include<zlib.h>
#include<errno.h>
#include "parser.h"
#include "urltable.h"
using namespace std;

#define __DEBUG__
//ls *_index
#define INDEX_CHUNK 409600 //400KiB


char *memAlloc(gzFile *, int, bool readAll); 

int main (int argc, char * argv[])
{
  gzFile *cData, *cIndex;
  char *indexBuffer;
  char indexFName[] = "nz2_merged/0_index";
  char dataFName[] = "nz2_merged/0_data";
  cIndex = (void **)gzopen(indexFName, "r");
  if (!cIndex) {
    fprintf (stderr, "gzopen of '%s' failed: %s.\n", indexFName,
             strerror (errno));
        exit (EXIT_FAILURE);
  }
  cData =  (void **)gzopen(dataFName,"r");
  if (!cData) {
    fprintf (stderr, "gzopen of '%s' failed: %s.\n", dataFName,
             strerror (errno));
        exit (EXIT_FAILURE);
  }

  indexBuffer=memAlloc(cIndex, INDEX_CHUNK, true);

  char *pLine = strtok(indexBuffer, "\n");
  string url;
  int offset = 0;
  string trash;
  int tmp = 0;

  while (pLine != NULL){
    stringstream ss(pLine);
    for(int i = 0; i < 7; ++i){
      if(i == 0)
        ss >> url;
      else if (i == 3)
        ss >> offset;
      else 
        ss >> trash;
    }
    if(tmp++ > 10) // for test only
      break;
    int docID = UrlTable::getInstance()->insert(url, dataFName, offset);
    if (docID == -1){
      cerr<<"urltable insert error"<<endl;
      break;
    }
    
    //cout<<url<<" "<<docID<<endl;
    char *pageBuf, *lexbuf;
    pageBuf = memAlloc(cData, offset, false);
    lexbuf = (char*)malloc(2*offset+1);
    int ret = parser(const_cast<char*>(url.c_str()), pageBuf, lexbuf, 2*offset+1);
    if (ret > 0)
      cout<< lexbuf <<"---------------------------------------------------"<<endl;
    else cout<<url<<" "<<offset <<" : parse error : " <<ret<< endl;

    free(pageBuf);
    free(lexbuf);
    pLine = strtok (NULL, "\n");
  }
  return 0;
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
            exit (EXIT_FAILURE);
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

