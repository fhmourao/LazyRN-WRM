/* lazyRNWRM.cc*/

/* Inclusions */
#include "lazyRNWRM.h"

int main(int argc,char ** argv){
	// Local variables
	int trainingSize;
	float minDominance;
	
	FILE *trainFile;
	FILE *testFile;
	FILE *outputFile;
	char *trainFileName= (char*) malloc(sizeof(char)*(FILE_NAME_SIZE+1));
	char *testFileName= (char*) malloc(sizeof(char)*(FILE_NAME_SIZE+1));
	char *outputFileName= (char*) malloc(sizeof(char)*(FILE_NAME_SIZE+1));

	unordered_map<int, int> testTerms;
	VectorOfTerms termsPerDocument;
	vector<myRelationClass> classAdjustment;
	HashOfHash termNetwork;

	// get input parameters 
	getParameters(trainFileName, testFileName, outputFileName, minDominance, argc, argv);

	// open input files
	trainFile = openInputFile(trainFileName);
	testFile = openInputFile(testFileName);

	//open output file
	outputFile = openOutputFile(outputFileName);

	//load test documents
	loadTestDocuments(testFile, testTerms, termsPerDocument);
	fclose(testFile);

	//build classification model
	trainingSize = buildTermNetwork(trainFile, testTerms, termNetwork, classAdjustment);
	fclose(trainFile);
	
	//classify Test Documents
	classifyTestDocuments(outputFile, testTerms, termsPerDocument, termNetwork, classAdjustment, trainingSize, minDominance);
	fclose(outputFile);

	return 0;
}

int buildTermNetwork(FILE *file, unordered_map<int, int> &testTerms, HashOfHash &termNetwork, vector<myRelationClass> &classAdjustment){
	char* item = NULL;
	int docClass;
	string relation;
	char *line = (char *) malloc(sizeof(char)*LINE_SIZE);
	int numSelectedItems;
	int trainingSize = 0;
	
	vector<int> selectedTerms;
	unordered_map<int, set<int> > termsPerClass;

	unordered_map<int, int>::iterator itr;

	//for each line in the train file
	while( (fgets(line, LINE_SIZE, file)!= NULL) && (!feof(file)) ) {
		// ignore document id
		item = strtok(line, DELIMITER); 

		// ignore temporal info 
		item = strtok(NULL, DELIMITER);

		// get the document's class 
		item = strtok(NULL, DELIMITER);
		docClass = atoi(item);

		// read all terms belonging to document 'docId'
		item = strtok(NULL, DELIMITER);
		while(item!=NULL) {
			//check if item represents a test item
			itr = testTerms.find(atoi(item));
			if( itr != testTerms.end() ){
				selectedTerms.push_back(atoi(item));
				
				termsPerClass[docClass].insert(atoi(item));
				
				//update test term occurrence in the training set
				testTerms[atoi(item)] += 1;
			}
			
			// ignore TF information
			item = strtok(NULL, DELIMITER); 
			// get next item
			item = strtok(NULL, DELIMITER);
		}
		
		numSelectedItems = (int) selectedTerms.size();
		for(int i=1; i<numSelectedItems; i++){
			for(int j=0; j<i; j++){
				if( selectedTerms[i] > selectedTerms[j])
					relation = to_string(selectedTerms[j]) + "," + to_string(selectedTerms[i]);
				else
					relation = to_string(selectedTerms[i]) + "," + to_string(selectedTerms[j]);

				// Instead of checking, it just ensures that relation is into the hash
				termNetwork[relation][INVALID_CLASS] = 0;
					
				//check if this relation has been observed within docClass
				itr = termNetwork[relation].find(docClass);
				if(itr == termNetwork[relation].end() )
					termNetwork[relation][docClass] = 0;
				
				termNetwork[relation][docClass] += 1;
				
			}
		}
		
		selectedTerms.clear();
		trainingSize++;
	}
	free(line);
	
	calculateAdjustmentFactors(termsPerClass, classAdjustment);
	
	termsPerClass.clear();
	
	return trainingSize;
	
}

void calculateAdjustmentFactors(unordered_map<int, set<int> > &termsPerClass, vector<myRelationClass> &classAdjustment){
	int totalRelations;
	int numTerms;
	int numRelations;
	int secondMax = 0;
	int maxOccurrence = 0;
	int numClasses;
	
	float adjustmentFactor = 1.0;
	float firstPercentage;
	float secondPercentage;
	float occurrence;
	
	
	set<int> termSet;
	unordered_map<int, set<int> >::iterator itr1;

	//calculate class adjustment factors
	totalRelations = 0;
	for(itr1=termsPerClass.begin(); itr1!=termsPerClass.end(); itr1++){
		termSet = itr1->second;
		numTerms = (int) termSet.size();
		numRelations = numTerms*(numTerms-1)/2;
		totalRelations += numRelations;
		
		if( numRelations > maxOccurrence ){
			secondMax = maxOccurrence;
			maxOccurrence =  numRelations;
		}
		else{
			if(numRelations > secondMax)
				secondMax = numRelations;
				
		}
		
		classAdjustment.push_back( make_pair(itr1->first, (float) numRelations) );
	}
	
	if( maxOccurrence > secondMax ){
		firstPercentage = ((float) maxOccurrence)/((float) totalRelations);
		secondPercentage = ((float) secondMax)/((float) totalRelations);
		
		adjustmentFactor = (firstPercentage-secondPercentage)/(firstPercentage*firstPercentage - secondPercentage*secondPercentage);
	}
	
	numClasses = (int) classAdjustment.size();
	for(int i=0; i<numClasses; i++){
		occurrence = ((float) classAdjustment[i].second)/((float) totalRelations);
		classAdjustment[i].second = 1.0 - (adjustmentFactor*occurrence);
	}
	
	sort(classAdjustment.begin(), classAdjustment.end(), comparisonFunction);
}

inline myRelationClass calculateClassDominance(string relation, HashOfHash &termNetwork, float minimumDominance){
	int totalOccurrence = 0;
	int maxOccurrence = 0;
	float dominance;
	float weight = 0.0;
	int dominantClass = INVALID_CLASS;
	
	unordered_map<int, int>::iterator itr;
		
	// determine the dominant class for the input relation
	for(itr=termNetwork[relation].begin(); itr!=termNetwork[relation].end(); itr++){
		if( itr->second > maxOccurrence ){
			maxOccurrence = itr->second;
			dominantClass = itr->first;
		}
		totalOccurrence += itr->second;
	}
	
	dominance = ((float) maxOccurrence)/((float) totalOccurrence);
	
	if(dominance >= minimumDominance)
		weight = dominance * ((float) log(totalOccurrence + 1));
	
	return make_pair(dominantClass, weight);
}

void classifyTestDocuments(FILE * outputFile, unordered_map<int, int> &testTerms, VectorOfTerms &termsPerDocument, HashOfHash &termNetwork, vector<myRelationClass> &classAdjustment, int trainingSize, float minimumDominance){
	int numTestDocs;
	int numTerms;
	int termId;
	float score;
	short hasVoteFlag;
	float termFrequency;
	float termIDF;
	string relation;
	int numDistinctClasses;
	
	myTerm term1;
	myTerm term2;
	myRelationClass relationClass;
	unordered_map<int, float> voteSummary;
	vector<myRelationClass> finalVote;
	vector<myTerm> docTerms;
	unordered_map<int, unordered_map<int, float> > termVote;
	
	HashOfHash::iterator itr1;
	unordered_map<int, float>::iterator itr2;
	vector<myRelationClass>::iterator itr3;
	
	//for each test document
	numTestDocs = (int) termsPerDocument.size();
	numDistinctClasses = (int) classAdjustment.size();
	for(int docIndex=0; docIndex<numTestDocs; docIndex++){
		docTerms = termsPerDocument[docIndex];
		numTerms = (int) docTerms.size();
		
		//initialize votes;
		for(int classIndex=0; classIndex<numDistinctClasses; classIndex++)
			voteSummary[ classAdjustment[classIndex].first ] = 0.0;

		//for each term
		for(int i=1; i<numTerms; i++){
			term1 = docTerms[i];
			termId = term1.first;
			termFrequency = term1.second;
			termIDF = 1.0;
			termVote[term1.first][INVALID_CLASS] = 0;
			
			// if test term was observed in the training set 
			if(testTerms[termId] > 0)
				termIDF = log( ((float) trainingSize)/((float) testTerms[termId]) );
			
			//for each other term in the same document
			for(int j=i+1; j<numTerms; j++){
				term2 = docTerms[j];
				termVote[term2.first][INVALID_CLASS] = 0;
				
				if( term1.first > term2.first )
					relation = to_string(term2.first) + "," + to_string(term1.first);
				else 
					relation = to_string(term1.first) + "," + to_string(term2.first);

				//check if the relation was observed in the training set
				itr1 = termNetwork.find(relation);
				if( itr1 != termNetwork.end() ){
					relationClass = calculateClassDominance(relation, termNetwork, minimumDominance);

					//check if it is a valid vote
					if(relationClass.second > 0.0 ){
						
						itr2 = termVote[term1.first].find(relationClass.first);
						if(itr2 == termVote[term1.first].end() )
							termVote[term1.first][relationClass.first] = 0.0;
						termVote[term1.first][relationClass.first] += relationClass.second;
						
						itr2 = termVote[term2.first].find(relationClass.first);
						if(itr2 == termVote[term2.first].end() )
							termVote[term2.first][relationClass.first] = 0.0;
						termVote[term2.first][relationClass.first] += relationClass.second;
					}
				}
			}
			
			// update term's vote according to its TFxIDF
			for(itr2=termVote[term1.first].begin(); itr2!=termVote[term1.first].end(); itr2++)
				voteSummary[itr2->first] += (itr2->second * termFrequency * termIDF);
			
			termVote[term1.first].clear();
		}
		termVote.clear();
		
		// update class's vote according to the adjusted class representativity
		hasVoteFlag = 0;
		for(int classIndex=0; classIndex<numDistinctClasses; classIndex++){
			score = voteSummary[classAdjustment[classIndex].first];
			if(score > 0.0){
				finalVote.push_back(make_pair( classAdjustment[classIndex].first, classAdjustment[classIndex].second * score));
				hasVoteFlag = 1;
			}
		}

		//print the resulting afinity matrix into the output file
		if( !hasVoteFlag )
			fprintf(outputFile, "%d %d %d:1.0\n", docIndex+1, docTerms[0].first, classAdjustment[numDistinctClasses-1].first);
		else{
			sort(finalVote.begin(), finalVote.end(), comparisonFunction);
			
			fprintf(outputFile, "%d %d", docIndex+1, docTerms[0].first);
			for(itr3=finalVote.begin(); itr3!=finalVote.end(); itr3++)
				fprintf(outputFile, " %d:%.3f", itr3->first, itr3->second);
			fprintf(outputFile, "\n");
		}
		
		finalVote.clear();
	}
	termsPerDocument.clear();
	termNetwork.clear();
}

bool comparisonFunction(const myRelationClass& a, const myRelationClass& b) {
    return ((a.second) > (b.second));
}

