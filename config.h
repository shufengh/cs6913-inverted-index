#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BARREL_SIZE 209715200; //foward  barrel size  about 100MiB
#define DEFAULT_SAVE_SIZE 20971520 // url mapper size 4MiB
#define READ_CHUNK 1048576 //409600 //400KiB

// last failure + 1
// ls urltable/* |sort -k2 -t/ -n
#define START_DOCID 0
#define START_TEMP_NUM 0
#define START_INDEX "nz2_merged/0_index"
#endif    
