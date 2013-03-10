#include<cstdio>
#include<iostream>
#include<sstream>
#include<map>
#include<unistd.h>
#include "gzstream.h"
#include <cerrno>
#include <pthread.h>
#include <ctime>
using namespace std;
/*
  This merger procedure is divided into two steps: it'll first splice the same posting
  and then merge lists within 2 passes.
*/
class Node{
public:
  int sid;// sid =igzstream id
  string posting;
  Node(int sid, string p){
    this->sid = sid;
    posting = p;
  }
  void append(string leftps){
    posting.append(leftps);
  }
};
bool map_append(map<string, Node*> &frontier, string posting, int sid); 
void merge_and_sort(string fnames, unsigned int filecnt);
string get_fnames(char* path);
unsigned int filecount(string fnames);
void *saveThread(void *t);
string tmpath;
int pipefd[2];
long default_content_size =  100000000; // about 100 MiB
int main(int argc, char **argv){
  if(argc != 4){
    cout<<"format: merger srcpath tmpath destpath"<<endl;
    return 1;
  }
  
  if (pipe (pipefd) < 0){
    cout<<"pipe: "<<strerror(errno)<<endl;
    exit(1);
  }
  pthread_t tid;
  string fnames;
  clock_t beg = clock();
  tmpath = string(argv[2]);
  pthread_create(&tid, NULL, saveThread, NULL);
  fnames = get_fnames(argv[1]);
  default_content_size = 300000000;
  merge_and_sort(fnames, 
                 filecount(fnames) > 10 ? 10:filecount(fnames));
  pthread_join(tid, NULL);

  tmpath = string(argv[3]);
  pthread_create(&tid, NULL, saveThread, NULL);
  fnames = get_fnames(argv[2]);
  default_content_size = 300000000;
  merge_and_sort(fnames, filecount(fnames));
  pthread_join(tid, NULL);

  clock_t end = clock();
  cout<<"Total time: "<<(double)(end-beg)/CLOCKS_PER_SEC<<endl;
  return 0;
}
unsigned int filecount(string fnames){
  string found;
  unsigned int cnt = 0;
  stringstream ss(fnames);
  while(!ss.eof()){
    ss>>found;
    ++cnt;
  }
  return cnt;
}
string get_fnames(char* path){
  string cmd("ls ");
  cmd.append(path);
  // sort numerically (-n) on the third field (-k3) using 'p' as the field separator (-tp).
  cmd.append("/* | sort -k3 -tp -n"); 
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
void merge_and_sort(string fnames, unsigned int bufnum){  

  igzstream gzin[bufnum];
  stringstream ns(fnames);
  map<string, Node*> frontier; 
  string pipeStr; // save merged first 20MiB postings
  string fname, posting;
  for(unsigned i = 0; i< bufnum; ++i){
    ns>>fname;
    gzin[i].open(fname.c_str());
    if ( ! gzin[i].good()) {
      std::cerr << "ERROR: Opening file '" << fname << "' failed.\n";
      break;
    }   
  }
  
  for(unsigned i = 0; i< bufnum; ++i){ // insert the first unique posting into the map 
    bool r  = false;
    do{
      if(gzin[i].eof()){
        gzin[i].close();
        if(ns.eof()){
          cout<<" have read all files in srcpath"<<endl;
          break;
        }
        ns>>fname;
        gzin[i].open(fname.c_str());
        if ( ! gzin[i].good() ) {
          std::cerr << "First round ERROR: Opening file '" << fname << "' failed.\n";
          break;
        }        
      }
      getline(gzin[i], posting);
      //cout<<posting<<endl;
      r = map_append(frontier, posting, i);
    }while(!r);
  }
  
  bool nsendtag = false; 
  while(1){
    if(frontier.size() == 0){
      cout<<"finish merging, exit"<<endl;
      write(pipefd[1],"99999999999999",14);
      break;
    }
    // remove the smallest posting from the map
    map<string,Node*>::iterator itr = frontier.begin();
    pipeStr += itr->first + " " + itr->second->posting + "\n";
    int sid = itr->second->sid;
    delete itr->second;
    frontier.erase(itr);
    //cout<<"pipeStr:"<<pipeStr<<endl;
    // insert the first unique posting into the map from the stream where the removed one's from
    bool r = false;
    do{
      if(gzin[sid].eof()){
        gzin[sid].close();
        gzin[sid].clear();    
        if(ns.eof()){
          if (!nsendtag){
            cout<<"all files in srcpath have been read"<<endl;
            nsendtag = true;
          }
          break;
        }
    
        string fname;
        ns>>fname;
        if (fname.empty())
          continue;

        gzin[sid].open(fname.c_str());
        if (!gzin[sid].good()) {
          std::cerr << "Second ERROR: Opening file '" << fname << "' failed. "<<strerror(errno)<<"\n";
          gzin[sid].clear();
          continue;
        }
      } 
      
      string curPs;
      getline(gzin[sid], curPs);
      r = map_append(frontier, curPs, sid);
    }while(!r);

    if (pipeStr.size() > 30000000 || frontier.size() == 0){
      char size[16];
      sprintf(size, "%14d",(unsigned)pipeStr.size()); 
      if(write(pipefd[1], size, 14) < 14){
        cout<<"write size: "<<strerror(errno)<<endl;
        exit(3);
      }
      if( write(pipefd[1], pipeStr.c_str(), pipeStr.size()) < pipeStr.size() ){
        cout<<"write pipeStr: "<<strerror(errno)<<endl;
        exit(3);
      }
      //cout<<"write size: "<<pipeStr.size()<<" "<<endl;
      pipeStr.clear();
    }
  }
  
}


void *saveThread(void *t){
  char readlen[14];
  char *content;
  char fnprefix[32];
  int nameCnt = 0;
  long curSize = 0;
  
  sprintf(fnprefix,"%s/temp", tmpath.c_str());
  char *filename = (char*) malloc(32);
  sprintf(filename,"%s%d",fnprefix,nameCnt++);
  
  ogzstream gzout;
  gzout.open(filename);
  if ( ! gzout.good()) {
    std::cerr << "saveThread: Opening file `" << filename<<"' failed "<<" "<<strerror(errno) <<endl;
    exit(3);
  }
  
  while(1){
    int r = read (pipefd[0], readlen, 14);
    if (r == -1) {
      perror("read");
      exit(3);
    }
    readlen[r] = 0;
    
    if(!strcmp(readlen,"99999999999999")){
      cout<<"pipe received close signal, thread exits"<<endl;
      gzout.close();
      break;
    }
    
    cout<<"told read size: "<<atol(readlen)<<endl;   
    content = (char*)malloc(atol(readlen)+1);
    long readsize = 0;
    r = 0;
    while(readsize < atol(readlen)){
      r = read(pipefd[0], content + readsize, atol(readlen));
      if (r == -1) {
        perror("read");
        delete content;
        free(content);
        exit(3);
      }
      readsize += r;
    }
    content[readsize] = 0;
    cout<<"actual read size: "<<strlen(content)<<endl;
    
    gzout<<content<<flush;
    cout<<"saved "<<strlen(content)+curSize<<" into "<<filename<<endl;
    //cout<<content<<endl;
    free(content);  
    curSize += atol(readlen); 

    if(curSize > default_content_size){
      gzout.close();
      free(filename);
      filename = (char*)malloc(32);
      sprintf(filename,"%s%d",fnprefix, nameCnt++);
      gzout.open(filename);
      if ( ! gzout.good()) {
        std::cerr << "saveThread: Opening file `" << filename << "' failed.\n";
        exit(3);
      }
      curSize = 0;
    }    
  }
  
}

bool map_append(map<string, Node*> &frontier, string posting, int sid){
  // split the new posting and if the posting exists, merge them, or insert it into the map.
  int split = posting.find_first_of(" ");
  //cout<<"map_append:"<<posting<<endl;
  if(split == -1){
    cerr<<"map_append: no word found"<<endl;
    return false;
  }
  string word  = posting.substr(0, split);
  string leftps = posting.substr(split+1);
  map<string,Node*>::iterator itr = frontier.find(word);
  if(itr == frontier.end()){
    Node *rd = new Node(sid, leftps);
    frontier[word] = rd;
  }
  else{
    frontier[word]->append(leftps);
    return false; // merge action and we should get next line of the stream.
  }  
  return true; // insertion action
}
