#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <errno.h>


/**
Name: Sungwon Kang
LoginID: skang03
**/


int hits = 0;
int misses = 0;
int evictions = 0; 


int h; //optional help flag that prints usage info
int s; //number of set index bits
int E; //number of lines per set
int b; //number of block bits
int t; //name of the valgrind trace to replay


struct cache_line{

    unsigned long tag;
    char validbit;
    int lruCounter; 
};


char tracefile[1000]; 
struct cache_line *cache;


/** allocate a 2d memory layout **/
void initialize(){

    int S = E * (1 << s);
    cache = (struct cache_line*)malloc(sizeof(struct cache_line) * S);
}


/** update tag, valid bit and LRU counter **/
void update(int setindex, int empty, unsigned long tag){

    cache[empty + setindex].tag = tag;    
    cache[empty + setindex].validbit = 1;
    cache[empty + setindex].lruCounter = t;
}


/** load cache **/
void load(unsigned long address){

    int lru = ++t; //least recently used
    int j = 0;
    int empty = -1;
    unsigned long tag = address >> b;
    int temp = (1 << s) - 1;
    temp = tag & temp;
    int setindex = E * temp; 

    while (j < E){  

	//if line is empty
	if (!cache[setindex + j].validbit){
	empty = j; 
	lru = t; 
	break;
	}

	//if line is not empty
        else{ 

            if (cache[setindex + j].tag == tag){ //if tags match
		hits++;
                cache[setindex + j].lruCounter = t;
                return;
            }

            else if (cache[setindex + j].lruCounter < lru){ //if tags don't match
                empty = j;
                lru = cache[setindex + j].lruCounter; //look for LRU
            }
        }       
        j++;
    }
    misses++;

    if (!(lru == t)){
        evictions ++;
    }

    /** update **/
    update(setindex, empty, tag);
}


/** read command line **/
void readc(int argc, char *argv[]){

    for (int option = 0; option != -1; option = getopt(argc, argv, "s:E:b:t:h")){

        if (optarg != NULL){

	    if(option == 's'){
		s = atoi(optarg);
	    }

	    else if(option =='E'){
		E = atoi(optarg);
	    }

	    else if (option =='b'){
		b = atoi(optarg);
	    }

	    else if (option =='t'){
		strncpy(tracefile, optarg, 1000);
	    }

	    else if (option =='h'){
		h = 1;
	    }
        }
    }
}


/** scan and determine the type of memory access **/
void readFile(){

    FILE *fp;
    char buf[1000];
    unsigned long address;
    initialize(); 

    for (fp = fopen(tracefile, "r"); fgets(buf, 1000, fp) != NULL;){
        sscanf(buf + 2, "%lx", &address);

        if (!fp){
            fprintf(stderr, "%s: %s\n", tracefile, strerror(errno));
            exit(1);
	}

        else if(buf[1] == 'I'){ //instruction load
	    continue;
	}

	else if(buf[1] == 'L'){ //data load
	    load(address);
	}

	else if (buf[1] == 'S'){ //data store
	    load(address);
	}

	else if (buf[1] == 'M'){ //data motify
	    load(address); //load
	    load(address); //store       
	}
    }
    fclose(fp); //close the file
    free(cache); //free the memory
    }


/** main **/
int main(int argc, char *argv[]){

    readc(argc, argv);
    readFile();
    printSummary(hits, misses, evictions); //print
    return 0;
}


