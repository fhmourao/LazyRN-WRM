/*myIO.h.h*/

#ifndef _myio_h
#define _myio_h

/* Inclusions */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <utility>
#include <unordered_map> 
#include <map>
#include <set>
#include <vector>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

/* Definitions */
#define LINE_SIZE 25000
#define DELIMITER " ;\t\n"
#define FILE_NAME_SIZE 100
#define MIN_ARGS 4
#define __USE_FILE_OFFSET64

typedef pair<int, int> myTerm;
typedef vector< vector<myTerm> > VectorOfTerms;

/* Prototypes */
void getParameters(char *trainFileName, char *testFileName, char *outputFileName, float &minDominance, int argc, char **argv);
void loadTestDocuments(FILE *file, unordered_map<int, int> &testTerms, VectorOfTerms &termsPerDocument);
FILE *openInputFile(char *fileName);
FILE *openOutputFile(char *fileName);
void getParameters(char *inputFileName,char *outputFileName, int argc,char **argv);
void usage();

#endif
