#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/select.h>
#include<ctype.h> 
#include<locale.h>
#include <time.h>

#define RESET "\033[0m"
#define ERROR_COLOR "\033[31m"
#define SUCCESS_COLOR "\033[32m"
#define BOLD "\033[1m"
#define BOLD_BLINK "\033[1;5m"
#define SUCCESS_BLINK "\033[1;5;33m"
#define FAIL_BLINK "\033[1;5;35m"
#define LIGHT_BLUE_BG_WHITE_TEXT "\033[97;104m"
#define GREEN_BLOCK_OF_TEXT "\033[30;42m"
#define MAGENTA_BLOCK_OF_TEXT "\033[30;45m"
#define DIM "\033[2;;m"

#define TRUE 1
#define FALSE 0
#define UNDEFINED -1
typedef int bool;

#define input_stream stdin // TODO: get this from terminal arguments at runtime <--------------------------------------------
#define CODONS_LENGTH 3

//  ***************************************************  Definitions of constants *******************************************

//const int MAX_LENGTH_OF_PROMPTS = 100;
const int HISTORY_RECORDS_SIZE = 500;
char* START_CODONS[] = {"AUG", "GUG", "UUG"}; // Source: https://bio.libretexts.org/Bookshelves/Introductory_and_General_Biology/General_Biology_(Boundless)/15%3A_Genes_and_Proteins/15.03%3A_Prokaryotic_Transcription_-_Transcription_in_Prokaryotes
const int NUM_OF_START_CODONS = 3;
char* STOP_CODONS[] = {"UAA", "UAG", "UGA"}; // Source: https://microbenotes.com/codon-chart-table-amino-acids/
const int NUM_OF_STOP_CODONS = 3;
char* VALID_CHARS[] = {"A", "U", "C", "G"};
const int NUM_OF_VALID_CHARS = 4;

// Structs and enums definitions

typedef enum {
    START,       // 0
    STOP,      	 // 1
    PLAIN        // 2
} specialCodonType;

typedef enum {
    FORWARD,    // 0
    REVERSE     // 1
} direction;

typedef struct {   
  specialCodonType type;           
  char *codonSequence;
  int positionInSequence;       
} SpecialSubsequence;

typedef struct {   
  int length;
  direction seqDirection;
  int positionInSupersequence;
  bool isCodingSequence;
  SpecialSubsequence specialCodons[];
} Sequence;

// Node in the doubly linked list
typedef struct ListNode {
    Sequence *data;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

// Doubly linked list structure
typedef struct {
    ListNode *head;
    ListNode *tail;
    int size;
} DoublyLinkedList;

const char* codonTypeToString(specialCodonType type) {
    switch (type) {
        case START: return "START";
        case STOP: return "STOP";
        case PLAIN: return "PLAIN";
        default: return "UNKNOWN";
    }
}

const char* readDirectionToString(direction readDirection) {
    switch (readDirection) {
        case FORWARD: return "FORWARD";
        case REVERSE: return "REVERSE";
        default: return "UNKNOWN";
    }
}

// Sequence "constructor"

Sequence* createSequence(int length, direction seqDirection, int positionInSupersequence, bool isCodingSequence, int numOfCodons) {

    Sequence* sequence = (Sequence*) malloc(sizeof(Sequence) + sizeof(SpecialSubsequence) * numOfCodons);
    sequence->length = length;
    sequence->seqDirection = seqDirection;
    sequence->positionInSupersequence = positionInSupersequence;
    sequence->isCodingSequence = isCodingSequence;

    for (int i = 0; i < numOfCodons; ++i) { // Initialize with dummy values
        sequence->specialCodons[i].type = PLAIN;
        sequence->specialCodons[i].codonSequence = malloc( (CODONS_LENGTH+1) * sizeof(char));
        strcpy(sequence->specialCodons[i].codonSequence, "AUG");
        sequence->specialCodons[i].positionInSequence = i * CODONS_LENGTH;
    }

    return sequence;
}

Sequence* createSequence2(int length, direction seqDirection, int positionInSupersequence, bool isCodingSequence, SpecialSubsequence* codons, int codonsArraySize) {

    Sequence* sequence = (Sequence*) malloc(sizeof(Sequence) + sizeof(SpecialSubsequence) * codonsArraySize);
    sequence->length = length;
    sequence->seqDirection = seqDirection;
    sequence->positionInSupersequence = positionInSupersequence;
    sequence->isCodingSequence = isCodingSequence;
    memcpy(sequence->specialCodons, codons, sizeof(SpecialSubsequence) * codonsArraySize);

    return sequence;
}

// ******************************************   Stream handling functions  ************************************************

bool stream_is_empty(FILE* input_stream)
{
	#ifdef __unix__ 	// Check for Linux, macOS, or any POSIX compliant OS

		fd_set readfds;
	    struct timeval tv;
	    int result;
	    int fileDescriptor = fileno(input_stream);

	    // Initialize timeout to 0 (non-blocking)
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;

	    // Set up the file descriptor set to monitor input_stream
	    FD_ZERO(&readfds);
	    FD_SET(fileDescriptor, &readfds); // Monitor input_stream

	    // Use select to check if input_stream has data available
	    result = select(fileDescriptor + 1, &readfds, NULL, NULL, &tv);

	    if (result > 0 && FD_ISSET(fileDescriptor, &readfds)) {
	        return FALSE;
	    } else {
	        return TRUE;
	    }

	#elif defined(_WIN32) || defined(_WIN64)

        if (_kbhit()) {
            return FALSE; // Input available
        } else {
            return TRUE; // No input available
        }

    #else // If the OS is not recognized, return 0

        return FALSE; // Default case: No input available

    #endif
}

void clear_input_buffer(FILE* input_stream, FILE* output_stream, int maxLength)
{
    int ch, numOfFlushed = 0;

    if (!stream_is_empty(input_stream))
    {	
    	if ( (ch = fgetc(input_stream)) != '\n')
    	{
    		fprintf(output_stream, ERROR_COLOR "\nSequence must be at most %d characters long. Flushing extra characters...\nFlushed characters:\t", maxLength);
    		fprintf(output_stream, "%c", ch);
    		numOfFlushed++;

	    	while ((ch = fgetc(input_stream)) != '\n' && ch != EOF) // Discard characters until newline or end of file
	    	{		        
		        fprintf(output_stream, ", ");
		    	fprintf(output_stream, "%c", ch);
		    	numOfFlushed++;
	    	}
    		fprintf(output_stream, "\nNum of flushed chars: %d\n" RESET, numOfFlushed);
    	}    	
    }
}

// **************************************  Doubly-linked list DS functions  **********************************************************

DoublyLinkedList* createList() {
    DoublyLinkedList *list = (DoublyLinkedList *)malloc(sizeof(DoublyLinkedList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

// Append a Sequence to the doubly linked list
void appendToList(DoublyLinkedList* list, Sequence* sequence) {
    ListNode* newNode = (ListNode*) malloc(sizeof(ListNode));
    newNode->data = sequence;
    newNode->next = NULL;
    newNode->prev = list->tail;

    if (list->tail) {
        list->tail->next = newNode;
    } else {
        list->head = newNode;
    }

    list->tail = newNode;
    list->size++;
}

// Remove a node from the list
void removeFromList(DoublyLinkedList* list, ListNode* node) {

    if (!node) return;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    free(node->data); // Free the Sequence data
    free(node);       // Free the node
    list->size--;
}

// Print the contents of the list
void printList(FILE* output_stream, DoublyLinkedList* list) {

    ListNode* current = list->head;

    while (current) {
        Sequence *seq = current->data;

        fprintf(output_stream, "\nSequence (Length: %d, Direction: %s, Position: %d, IsCodingSequence: %s)\n\n",
               seq->length,
               readDirectionToString(seq->seqDirection),
               seq->positionInSupersequence,
               seq->isCodingSequence ? "YES" : "NO");

        for (int i = 0; i < (seq->length / CODONS_LENGTH); ++i) {
            fprintf(output_stream, "\t%d)\tType: ", i);

            // Color coding for special codons
            if (seq->specialCodons[i].type == START)
            {
            	fprintf(output_stream, SUCCESS_COLOR "%s" RESET, codonTypeToString(seq->specialCodons[i].type));
            	fprintf(output_stream, "\tPosition: %d,\tCodon: ", seq->specialCodons[i].positionInSequence);
            	fprintf(output_stream, SUCCESS_COLOR "%s\n" RESET, seq->specialCodons[i].codonSequence);

            } else if (seq->specialCodons[i].type == STOP)
            {
            	fprintf(output_stream, ERROR_COLOR "%s" RESET, codonTypeToString(seq->specialCodons[i].type));
            	fprintf(output_stream, "\tPosition: %d,\tCodon: ", seq->specialCodons[i].positionInSequence);
            	fprintf(output_stream, ERROR_COLOR "%s\n" RESET, seq->specialCodons[i].codonSequence);

            } else {
            	fprintf(output_stream, "%s", codonTypeToString(seq->specialCodons[i].type));
            	fprintf(output_stream, "\tPosition: %d,\tCodon: ", seq->specialCodons[i].positionInSequence);
            	fprintf(output_stream, "%s\n", seq->specialCodons[i].codonSequence);
            }

        }

        current = current->next;
    }
}

// Free the entire list
void freeList(DoublyLinkedList* list) {

    ListNode* current = list->head;

    while (current) {
        ListNode* next = current->next;
        free(current->data); // Free Sequence
        free(current);       // Free node
        current = next;
    }

    free(list);
}

// **************************************  Generic, utility functions  *****************************************************************

void toUpperCase(char *str)
{
	setlocale(LC_ALL, "C");

    while (*str) {
        *str = toupper(*str);  // Convert each character to uppercase
        str++;
    }
}

bool has_valid_chars(char* sequence, int seqLength, char** validChars, int numOfValidChars) {

	bool isValid = FALSE;

	for (int i = 0; i < seqLength; i++) {
		isValid = FALSE;
		for (int j = 0; j < numOfValidChars; j++) {
			if ( ((char) toupper(sequence[i])) == ((char) validChars[j][0]) ) {
				isValid = TRUE;	
				break;		
			}
		}

		if (isValid == FALSE)
		{
			return isValid;
		}
	}

	return TRUE;
}

char* reverseString(FILE* output_stream, char* str, int strLength) {

    if (str == NULL) {
        return NULL;
    }

    char* reversed = (char*) malloc((strLength + 1) * sizeof(char));

    if (reversed == NULL) {
        fprintf(output_stream, ERROR_COLOR "Couldn't allocate space in memory to check the reverse sequence (required for analysis purposes)! %s\n\a" RESET, strerror(errno));
		return NULL;
    }

    for (int i = 0; i < strLength; i++) {
        reversed[i] = str[strLength - 1 - i]; // Copy characters in reverse order
    }
    reversed[strLength] = '\0'; // Null-terminate the new string

    return reversed;
}

// ****************************************************  Sequencing-related functions  ***************************************************

int is_start_codon(char* codon) {

	for (int i = 0; i < NUM_OF_START_CODONS; ++i) {
		if ( strncmp(codon, START_CODONS[i], CODONS_LENGTH) == 0 ) {
			return i;
		}
	}
	return -1;
}

int is_stop_codon(char* codon) {

	for (int i = 0; i < NUM_OF_STOP_CODONS; ++i) {
		if ( strncmp(codon, STOP_CODONS[i], CODONS_LENGTH) == 0 ) {
			return i;
		}
	}
	return -1;
}

Sequence** tokenize_seq(FILE* output_stream, int numOfRuns, char* sequence, int sequenceLength) { 
	
	toUpperCase(sequence); // convert to upper case for uniformity and easier processing

	Sequence** givenAndReversedSeq = malloc(2 * sizeof(Sequence*));
	if (givenAndReversedSeq == NULL) {
	    fprintf(output_stream, ERROR_COLOR "Memory allocation failed. Couldn't store sequence.\n%s\a\n" RESET, strerror(errno));
	    exit(EXIT_FAILURE);
	}

	int numOfCodons = sequenceLength/CODONS_LENGTH;
	Sequence* forwardSequence = createSequence(sequenceLength, FORWARD, 0, UNDEFINED, numOfCodons);
    Sequence* backwardSequence = createSequence(sequenceLength, REVERSE, 0, UNDEFINED, numOfCodons);

	for (int seqIndex = 0; seqIndex < sequenceLength; /*seqIndex + CODONS_LENGTH, but it's done at the last step of each iteration */ )
	{	
		int codonsForwardIndex = seqIndex/CODONS_LENGTH; // We scan sequence in frames of length CODONS_LENGTH, but array with sequence's codons have an index that's inceremnted by one
		int codonsBackwardIndex = (sequenceLength - seqIndex)/CODONS_LENGTH - 1; // Follow the logic as above, but start from the end of the array and move backwards in reverse order
		
		char* forwardCodon = (char*)malloc(sizeof(char) * CODONS_LENGTH + 1); // +1 is for null termination
		strncpy(forwardCodon, &sequence[seqIndex], CODONS_LENGTH);
		forwardCodon[CODONS_LENGTH] = '\0';

		int startCodonPosition = is_start_codon(forwardCodon);
		if (startCodonPosition > -1) {
			forwardSequence->specialCodons[codonsForwardIndex].type = START;
			forwardSequence->specialCodons[codonsForwardIndex].positionInSequence = seqIndex+1; // human-readable ordering
			strcpy(forwardSequence->specialCodons[codonsForwardIndex].codonSequence, START_CODONS[startCodonPosition]);
		}		

		int stopCodonPosition = is_stop_codon(forwardCodon);
		if (stopCodonPosition > -1) {
			forwardSequence->specialCodons[codonsForwardIndex].type = STOP;
			forwardSequence->specialCodons[codonsForwardIndex].positionInSequence = seqIndex+1;
			strcpy(forwardSequence->specialCodons[codonsForwardIndex].codonSequence, STOP_CODONS[stopCodonPosition]);
		}

		if ( (startCodonPosition == -1) && (stopCodonPosition == -1) ) {
    		forwardSequence->specialCodons[codonsForwardIndex].type = PLAIN;
        	forwardSequence->specialCodons[codonsForwardIndex].positionInSequence = seqIndex+1;
    		strcpy(forwardSequence->specialCodons[codonsForwardIndex].codonSequence, forwardCodon);
    	}


    	//  ****************************  check reverse codon  **********************************************

    	char* reverseCodon = reverseString(output_stream, &sequence[seqIndex], CODONS_LENGTH);

		startCodonPosition = -1;
		startCodonPosition = is_start_codon(reverseCodon);
		if (startCodonPosition > -1) {
			backwardSequence->specialCodons[codonsBackwardIndex].type = START;
			backwardSequence->specialCodons[codonsBackwardIndex].positionInSequence = seqIndex + CODONS_LENGTH; 
			strcpy(backwardSequence->specialCodons[codonsBackwardIndex].codonSequence, START_CODONS[startCodonPosition]);
		}

		stopCodonPosition = -1;
		stopCodonPosition = is_stop_codon(reverseCodon);
		if (stopCodonPosition > -1) {
			backwardSequence->specialCodons[codonsBackwardIndex].type = STOP;
			backwardSequence->specialCodons[codonsBackwardIndex].positionInSequence = seqIndex + CODONS_LENGTH; 
			strcpy(backwardSequence->specialCodons[codonsBackwardIndex].codonSequence, STOP_CODONS[stopCodonPosition]);
		}

    	if ( (startCodonPosition == -1) && (stopCodonPosition == -1) ) {
    		backwardSequence->specialCodons[codonsBackwardIndex].type = PLAIN;
        	backwardSequence->specialCodons[codonsBackwardIndex].positionInSequence = seqIndex + CODONS_LENGTH; 
    		strcpy(backwardSequence->specialCodons[codonsBackwardIndex].codonSequence, reverseCodon);
    	}

        seqIndex += CODONS_LENGTH;
	}

	givenAndReversedSeq[0] = forwardSequence;
	givenAndReversedSeq[1] = backwardSequence;
	// TODO: decide when it's a good time to free the allocated memory
  	// free(codons);
  	// Free allocated memory later in the code when no longer needed
	// for (int i = 0; i < 2; i++) {
	//     free(givenAndReversedSeq[i]); // Free each Sequence struct
	// }
	// free(givenAndReversedSeq); // Free the array of pointers
	
	return givenAndReversedSeq;
}

DoublyLinkedList* find_all_sequences(FILE* output_stream, int numOfRuns, char* sequence, int sequenceLength) {

	Sequence** givenAndReversedSeq = tokenize_seq(output_stream, numOfRuns, sequence, sequenceLength);
	DoublyLinkedList* validSequencesArray = createList();

	Sequence* newValidSequence;

	SpecialSubsequence* codons = (SpecialSubsequence *) malloc( sizeof(SpecialSubsequence) );
  	int codonsArraySize = 0;  // Keep track of the current size of the array. Array is dynamically extended in each iteration of the following for-loop

	bool foundStartCodon = FALSE;
	int positionInSupersequence;

	// *********************     FORWARD SEQUENCE :    ***********************************************************

	for (int i = 0; i < givenAndReversedSeq[0]->length/CODONS_LENGTH; i++) {

		if ( givenAndReversedSeq[0]->specialCodons[i].type == START ) {
			foundStartCodon = TRUE;
			positionInSupersequence = givenAndReversedSeq[0]->specialCodons[i].positionInSequence; // TODO: doesn't give the super-sequency's posiyion, but newValidSeq's. Change that!
		}

		if (foundStartCodon) {
			
			// Reallocate memory for the codons' array to hold one more element
	        codons = realloc(codons, (codonsArraySize + 1) * sizeof(SpecialSubsequence));
	        if (codons == NULL) {
	            fprintf(output_stream, ERROR_COLOR "Memory allocation failed. Couldn't store codon to the list of sequence's codons.\n%s\a\n" RESET, strerror(errno));
	            return NULL; // TODO: do not exit, but return to main menu, after flushing all allocated memory
	        }
	        codons[codonsArraySize] = givenAndReversedSeq[0]->specialCodons[i];
			codonsArraySize++;

			if ( givenAndReversedSeq[0]->specialCodons[i].type == STOP ) {

				// Found a valid sequence, so we create the struct that will kepp its info and we store it to the array with all valid sequences
				newValidSequence = createSequence2(codonsArraySize*CODONS_LENGTH, givenAndReversedSeq[0]->seqDirection, positionInSupersequence, TRUE, codons, codonsArraySize);
				appendToList(validSequencesArray, newValidSequence);
				foundStartCodon = FALSE;
				codonsArraySize = 0;
				free(codons);
				codons = (SpecialSubsequence *) malloc( sizeof(SpecialSubsequence) );

			}
		}
	}

		
	// *********************     BACKWARD SEQUENCE :    ***********************************************************

	free(codons);
	codons = (SpecialSubsequence *) malloc( sizeof(SpecialSubsequence) );
  	codonsArraySize = 0; 
	foundStartCodon = FALSE;

	for (int i = 0; i < givenAndReversedSeq[1]->length/CODONS_LENGTH; i++) {

		if ( givenAndReversedSeq[1]->specialCodons[i].type == START ) {
			foundStartCodon = TRUE;
			positionInSupersequence = givenAndReversedSeq[1]->specialCodons[i].positionInSequence;
		}

		if (foundStartCodon) {
			
			// Reallocate memory for the codons' array to hold one more element
	        codons = realloc(codons, (codonsArraySize + 1) * sizeof(SpecialSubsequence));
	        if (codons == NULL) {
	            fprintf(output_stream, ERROR_COLOR "Memory allocation failed. Couldn't store codon to the list of sequence's codons.\n%s\a\n" RESET, strerror(errno));
	            return NULL; // TODO: do not exit, but return to main menu, after flushing all allocated memory
	        }
	        codons[codonsArraySize] = givenAndReversedSeq[1]->specialCodons[i];
			codonsArraySize++;

			if ( givenAndReversedSeq[1]->specialCodons[i].type == STOP ) {

				// Found a valid sequence, so we create the struct that will kepp its info and we store it to the array with all valid sequences
				newValidSequence = createSequence2(codonsArraySize*CODONS_LENGTH, givenAndReversedSeq[1]->seqDirection, positionInSupersequence, TRUE, codons, codonsArraySize);
				appendToList(validSequencesArray, newValidSequence);
				foundStartCodon = FALSE;
				codonsArraySize = 0;
				free(codons);
				codons = (SpecialSubsequence *) malloc( sizeof(SpecialSubsequence) );
			}
		}		
	}

	return validSequencesArray;
}














// ********************************************* Main function  ******************************************************************




int main(int argc, char const *argv[])
{

    FILE *output_stream = NULL, *archiveFile = NULL;
    char* historyOfSeqs[HISTORY_RECORDS_SIZE];
    char* sequence;
	int maxLengthOfSeq = 0;
	int menuOption = 0, rerunApp = 0;

	// Check if enough arguments are provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_stream> <archive_file>\n", argv[0]);
        fprintf(stderr, "Options for <output_stream>: stdout, stderr, or a file path.\n");
        fprintf(stderr, "Argument <archive_file> is optional. It can be any file path. If file doesn't exist, it is created.\n");
        fprintf(stderr, "If the optional argument, <archive_file> isn't provided, a new archive file is generated in current directory (%s).\n", argv[0]);
        return 1;
    }


    // Determine the output stream
    if (strcmp(argv[1], "stdout") == 0)
    {
        output_stream = stdout;
    } else if (strcmp(argv[1], "stderr") == 0) 
    {
        output_stream = stderr;
    } else 
    {
        // Treat as a file path
        output_stream = fopen(argv[1], "w");
        if (output_stream == NULL)
        {
            perror("Error opening file!\a");
            return 1;
        }
    }

    // Check if archive file is provided
    if (argc > 2)
    {
    	archiveFile = fopen(argv[2], "r"); // We first try to open it in read mode just to check if it exists, so that we can inform user accordingly
    	if (archiveFile == NULL)
    	{
	        // File does not exist, create it in append-read mode
	        fprintf(output_stream, ERROR_COLOR "File '%s' doesn't exist\a\n" RESET, argv[2]);
	        fprintf(output_stream, "Creating file '%s'...\n", argv[2]);

	        archiveFile = fopen(argv[2], "a+");
	        if (archiveFile == NULL) {
	        	fprintf(output_stream, ERROR_COLOR "Error creating archive file:\t%s\a\n" RESET, strerror(errno));
	            return 1;
	        }
	    } else
	    {
	    	fclose(archiveFile);
	    	archiveFile = fopen(argv[2], "a+");
	        if (archiveFile == NULL) {
	        	fprintf(output_stream, ERROR_COLOR "Error creating archive file:\t%s\a\n" RESET, strerror(errno));
	            return 1;
	        }
	    } 
    } else
    {
    	archiveFile = fopen("./ARCHIVE_FILE.txt", "a+");
        if (archiveFile == NULL) {
        	fprintf(output_stream, ERROR_COLOR "Error creating archive file './ARCHIVE_FILE.txt' :\t%s\a\n" RESET, strerror(errno));
            return 1;
        }
    }

    do
    {
	    fprintf(output_stream, SUCCESS_BLINK "Welcome to Sequence-Checker 2.0!\t:)\n" RESET );
	    fprintf(output_stream, BOLD "\n\n\t\t\t*********************\n\t\t\t\tMENU\n\t\t\t*********************\n\n" );
	    fprintf(output_stream,  "1. Verify input sequences contain prokaryotic coding sequences.\n" );
	    fprintf(output_stream,  "2. Show history.\n") ;
	    fprintf(output_stream,  "3. Exit.\n");
	    fprintf(output_stream, "----------------------------------------------------------------------" RESET);
	    fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");
	    scanf("%d", &menuOption);
	    fprintf(output_stream,  RESET "\n"); // Reset colors

	    while (menuOption < 1 || menuOption > 3)
	    {
	    	fprintf(output_stream, ERROR_COLOR "\aThe number you entered doesn't correspond to any menu option.\nChoose one of the menu options (1-3)" RESET);
	    	fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");
	    	scanf("%d", &menuOption);
	    	fprintf(output_stream,  RESET "\n"); // Reset colors
	    }

	    if (menuOption == 1)
	    {
	    	fprintf(output_stream, DIM "Before we begin, please enter the maximum length that a sequence can have:\t" RESET);
	    	scanf("%d", &maxLengthOfSeq);
	    	while (maxLengthOfSeq < 2)
	    	{
	    		fprintf(output_stream, DIM "Maximum length must be at least nine chars long and it should be a positive integer. So, what is the maximum sequence length?\t" RESET);
	    		scanf("%d", &maxLengthOfSeq);
	    		scanf("%*c"); // Clear newline left in buffer
	    	}
	    	sequence = (char*) malloc(1+maxLengthOfSeq*sizeof(char));
	    	if (sequence == NULL) {
		        fprintf(output_stream, ERROR_COLOR "Couldn't allocate space in memory for storing the sequence! %s\n\a" RESET, strerror(errno));
		        return 1;
		    }
	    	
	    	fprintf(output_stream, BOLD "\n\n\t****************************\n\t\tINSTRUCTIONS\n\t****************************\n\n" );
	    	fprintf(output_stream, "Enter the sequence that you want to be analyzed\n");
	    	fprintf(output_stream, "If anytime you're done with the input, enter q\n");
	    	fprintf(output_stream, "----------------------------------------------------------\n\n" RESET);
	        // clear_input_buffer(input_stream); // If the user enters more than the buffer size, clear the remaining input

	    	bool inputOfSeqsCompleted = FALSE;
	    	int numOfRuns = 0;
	        do {

	        	int sequenceLength = 0;
	        	bool hasValidChars = FALSE;

	        	memset(sequence, '\0', maxLengthOfSeq);
	        	if (numOfRuns != 0) { // For some peculiar reason, input cannot be flushed so it always receives an empty sequence on the first run, the analysis of which we do not store to results
	        		fprintf(output_stream, "\n%d) Enter a new sequence:\t", numOfRuns);
	        	}
		    	fgets(sequence, maxLengthOfSeq+1, input_stream);
		    	
        		sequenceLength = strlen(sequence); // Check how many characters were actually read (ignoring the newline if present)

        		 if (sequence[sequenceLength - 1] == '\n')
		        {
		        	// newLine char is ignored
		        	sequence[sequenceLength - 1] = '\0';
		        	sequenceLength -= 1; 

		        } else if ( sequenceLength >= maxLengthOfSeq )
        		{
		        	clear_input_buffer(input_stream, output_stream, maxLengthOfSeq); // If the user enters more than the buffer size, clear the remaining input

		        }
		        inputOfSeqsCompleted = (strcmp(sequence, "q") == 0) || (strcmp(sequence, "Q") == 0);
				hasValidChars = has_valid_chars(sequence, sequenceLength, VALID_CHARS, NUM_OF_VALID_CHARS) || (strcmp(sequence, "q") == 0) || (strcmp(sequence, "Q") == 0);

	        	while ( !hasValidChars ) {
	        		fprintf(output_stream, ERROR_COLOR "You entered invalid character(s)!\nAcceptable chars:\t\a");
		        	for (int i = 0; i < NUM_OF_VALID_CHARS; ++i) {
		        		fprintf(output_stream, "%c\t", VALID_CHARS[i][0]);
		        	}
		        	memset(sequence, '\0', maxLengthOfSeq);
		        	fprintf(output_stream, "\n\nYou must enter the sequence again. Please enter the correct sequence:\t" RESET);
			    	fgets(sequence, maxLengthOfSeq+1, input_stream);
			    	
	        		sequenceLength = strlen(sequence); // Check how many characters were actually read (ignoring the newline if present)

	        		if (sequence[sequenceLength - 1] == '\n') {
			        	// newLine char is ignored
			        	sequence[sequenceLength - 1] = '\0';
			        	sequenceLength -= 1; 

			        } else if ( sequenceLength >= maxLengthOfSeq ) {
			        	clear_input_buffer(input_stream, output_stream, maxLengthOfSeq); // If the user enters more than the buffer size, clear the remaining input

			        }
			        inputOfSeqsCompleted = (strcmp(sequence, "q") == 0) || (strcmp(sequence, "Q") == 0);
					hasValidChars = has_valid_chars(sequence, sequenceLength, VALID_CHARS, NUM_OF_VALID_CHARS) || (strcmp(sequence, "q") == 0) || (strcmp(sequence, "Q") == 0);
	        	}

	        	if (numOfRuns != 0) { // For some peculiar reason, input cannot be flushed so it always receives an empty sequence on the first run, the analysis of which we do not store to results
					fprintf(output_stream, "\nYou entered the sequence:\n");
				}
				fprintf(output_stream, MAGENTA_BLOCK_OF_TEXT "%s" RESET, sequence);
				fprintf(output_stream, "\n");

	        	bool isCodingSequence = FALSE;

		        if (!inputOfSeqsCompleted)
		        {	
		        	DoublyLinkedList* validSequencesArray;
		        	validSequencesArray = find_all_sequences( output_stream, numOfRuns, sequence, sequenceLength);
		        	printList(output_stream, validSequencesArray);					
			    }

		        numOfRuns++;
	    	} while( !inputOfSeqsCompleted );
			
	    	free(sequence);
	    	sequence = NULL; // Is this correct here, since we freed memory allocated for 'sequence'?

	    } else if (menuOption == 2)
	    {
	    	fprintf(output_stream, SUCCESS_COLOR "Fine..." RESET);
	    	/* code */

	    } else
	    {
	    	fprintf(output_stream, SUCCESS_BLINK "Good... See ya around!\n" RESET);

		    // Close the file stream if it's not stdout or stderr
		    if (output_stream != stdout && output_stream != stderr) {
		        fclose(output_stream);
		    }
		    return 0;
	    }


		fprintf(output_stream, "\nWould you like to start over again?\n(YES: 1\tNO: 0");
    	fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");	
	    scanf("%d", &rerunApp);
	    fprintf(output_stream,  RESET "\n"); // Reset colors
	    while (rerunApp != 0 && rerunApp != 1)
	    {
	    	fprintf(output_stream, ERROR_COLOR "\aYou should enter 1 for \"YES\" or 0 for \"NO\".\nSo, would you like to run the app again?\t(YES: 1\tNO: 0)" RESET);
    		fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");	
	    	scanf("%d", &rerunApp);
	    	fprintf(output_stream,  RESET "\n"); // Reset colors
	    }
    } while (rerunApp == 1);

    fprintf(output_stream, SUCCESS_BLINK "Good... See ya around!\n" RESET);

    // Close the file stream if it's not stdout or stderr
    if (output_stream != stdout && output_stream != stderr)
    {
        fclose(output_stream);
    }
    return 0;
}

// aauuccauguccuaauccauguccuaauccgua : auguccuaa auguccuaa augccuaauccuguaccuaauccuguaccuuaa



