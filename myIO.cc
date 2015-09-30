/* myIO.cc*/

/* Inclusions */
#include "myIO.h"

void getParameters(char *trainFileName, char *testFileName, char *outputFileName, float &minDominance, int argc, char **argv){
	int c;
	int num_args = 0;
	while((c = getopt(argc,argv,"t:e:o:d:h"))!=-1) {
		switch(c) {
			case 't':
				strncpy(trainFileName,strdup(optarg),FILE_NAME_SIZE);
				if (trainFileName== NULL) {
					fprintf(stderr, "\n\tERROR: you must give a valid name for the training file!\n");
					usage();
					exit(1);
				}
			break;
			case 'e':
				strncpy(testFileName,strdup(optarg),FILE_NAME_SIZE);
				if (testFileName== NULL) {
					fprintf(stderr, "\n\tERROR: you must give a valid name for the test file!\n");
					usage();
					exit(1);
				}
			break;
			case 'o': 
				strncpy(outputFileName,strdup(optarg),FILE_NAME_SIZE);
				if (outputFileName == NULL) {
					fprintf(stderr, "\n\tERROR: you must give a valid name for the output file!\n");
					usage();
					exit(1);
				}
			break;
			case 'd':
				minDominance = ((float) atoi(optarg))/100.0;
				if( (minDominance < 0.0) | (minDominance > 1.0) ){
					fprintf(stderr, "\n\tERROR: Invalid minimum dominance value. It must be an integer between zero and hundred!\n");
					usage();
					exit(1);
				}
			break;
			default:
				usage();
				exit(1);
		}
		num_args++;
	}

	// verifica se todos os parâmetros obrigatórios foram passados
	if( num_args < MIN_ARGS ){
		usage();
		exit(1);
	}
}

void loadTestDocuments(FILE *file, unordered_map<int, int> &testTerms, VectorOfTerms &termsPerDocument){
	char *line = (char *) malloc(sizeof(char)*LINE_SIZE);
	char* item = NULL;
	int docClass;
	int termId;
	
	vector<myTerm> currentDocument;

	//for each line in the test file
	while( (fgets(line, LINE_SIZE, file) != NULL) && (!feof(file)) ) {
		// ignore the document's id
		item = strtok(line, DELIMITER); 

		// ignore temporal info 
		item = strtok(NULL, DELIMITER);

		// get the document's class 
		item = strtok(NULL, DELIMITER);
		docClass = atoi(item);
		
		//first position in the vector represents the document class
		currentDocument.push_back(make_pair(docClass, 0));

		// read all terms belonging to current document 
		item = strtok(NULL, DELIMITER);
		while(item!=NULL) {
			termId = atoi(item);
			testTerms[termId] = 0;
			
			// get TF information
			item = strtok(NULL, DELIMITER); 
			currentDocument.push_back(make_pair(termId, atoi(item)));
			
			// get next item
			item = strtok(NULL, DELIMITER);			
		}		
		termsPerDocument.push_back(currentDocument);
		
		//clear term vector
		currentDocument.clear();

	}
	free(line);
}

FILE *openInputFile(char *fileName){
	int fildes;
	FILE *file;

	fildes  = open(fileName, O_RDONLY|O_LARGEFILE);
	if(fildes < 0){
		printf("\n\t*** File (%s) not found!\n\n",fileName);
		exit(1);
	}
	file = fdopen (fildes, "r");

	return file;
}

FILE *openOutputFile(char *fileName){
	int fildes;
	FILE *file;

	fildes  = open(fileName, O_WRONLY|O_LARGEFILE|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(fildes < 0){
		printf("\n\t*** File (%s) not found!\n\n",fileName);
		exit(1);
	}
	file = fdopen (fildes, "w");

	return file;
}

void usage() {
	printf("Options: \n");
	printf("         -h                  (prints this help)\n");
	printf("         -t                  (Training file -- string with at most 100 characters)\n");
	printf("         -e                  (Test file -- string with at most 100 characters)\n");
	printf("         -d                  (Minimum dominance value -- an integer between zero and 100.)\n");
	printf("         -o                  (Output file -- string with at most 100 characters)\n\n");
}
