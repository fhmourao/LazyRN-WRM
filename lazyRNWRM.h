/*lazyRNWRM.h*/

#ifndef _lazyrnwrm_h
#define _lazyrnwrm_h

/* Inclusions */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "myIO.h"

using namespace std;

/* Definitions */
#define INVALID_CLASS -1000

typedef pair<int, float> myRelationClass;
typedef unordered_map<string, unordered_map<int, int> > HashOfHash;

/* Prototypes */
int main(int argc,char ** argv);
int buildTermNetwork(FILE *file, unordered_map<int, int> &testTerms, HashOfHash &termNetwork, vector<myRelationClass> &classAdjustment);
void calculateAdjustmentFactors(unordered_map<int, set<int> > &termsPerClass, vector<myRelationClass> &classAdjustment);
inline myRelationClass calculateClassDominance(string relation, HashOfHash &termNetwork, float minimumDominance);
void classifyTestDocuments(FILE * outputFile, unordered_map<int, int> &testTerms, VectorOfTerms &termsPerDocument, HashOfHash &termNetwork, vector<myRelationClass> &classAdjustment, int trainingSize, float minimumDominance);
bool comparisonFunction(const myRelationClass& a, const myRelationClass& b);

#endif
