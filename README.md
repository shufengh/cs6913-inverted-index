WSE: Inverted Index List
=======

The program consists of 3 parts: interps, merger and formatter.

Interps creates intermediate posting from the dataset. It first gets the file list of the given directory and opens it with the zlib utility. It used the parser provided on the course website modified to output words' positions. It then assigns a docID for a page with the UrlTable class and  puts the data into a c++ map with words as keys and the rest as values. The size of the map can be set through config.h, and it saves the content to disk when the size is beyond the setting figure, which is done by the FwdIndex class. The Record class serves to save and format the rest of posting. It contains two main strings: the first one is to save the information for the current page and the format is [context pos context pos .. ..]; the second one is to save all the information the word corresponds in the current map. When a new record with a current docID is inserted, it just increments the frequency counter and appends the context and position the first string; when a new record with a new docID is inserted, it splices the word's information in current page and appends it to the second string. The format is [docID freq Con pos Con pos .. ..]. Finally it creates intermediate index lists in several files gzipped which are sorted with words alphabetically. 

Merger merges and sorts the partial sorted intermediate index lists. It uses a library that wraps zlib as c++ streams. It first gets the number of total files in the given directory and divides them into two passes to create the final unformatted index list. It uses a c++ map again with words as keys and the rest as values to implement the merge and sort: it first gets the first posting from the opened list and 'insert' it into the heap(map)(if the word's already in the map, it will append this posting to the one in the map with the word trimmed). When the map size is over the given max size, it pipes the postings in the current map to another thread to save them to disk, which is also gzipped. 

Formatter formats the final unformatted index list and create the lexicon file. The format of the lexicon is like this: [word Ft offset]. It is formatted as ascii and saved in a whole gzipped file. the final posting format is like this: 

          [docid:5Bts | freq:1Bt | con: 3bits| pos:13bits]

Since the postion is only within 13 bits and forms two bytes with the context that is already compressed, it uses var-bytes method to compress the docid, which it itself is the type of unsigned int occupying four bytes all the time. but with var-bytes its space occupation ranges from 1 byte to five bytes. 

How to run it.
======
The program is organized with a Makefile:

```bash
$$make run_interps        # create the intermediate index and need to specify the datapath

$$make run_merger        # create the final unformatted index list and need to specify three parameters: the intermediate list path, tmpath to save the the first path result and the destpath to save final list

$$make run_formatter      #create the final formatted list. Need to specify the unformatted list path
```

Limits
======
1. The code is a mixture of c and c++, so some parts of the memory management is blackbox-ed, like strings and fstreams. And I tried to keep the memory usage in a certain capacity, like saving data into disk when the size reaches a certain order.


Evaluation
======

I tested the code with the nz2_merged and the first 1500 tarballs in the NZ dataset. The data in the tarballs is about 8.6 GiB and it took about 4 hours to create intermediate index lists for them and another 10 minutes to generate 1 GiB final unformatted index lists with 2 passes by the merger.
