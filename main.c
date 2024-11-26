#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define RESET "\033[0m"
#define ERROR_COLOR "\033[31m"
#define SUCCESS_COLOR "\033[32m"
#define BOLD "\033[1m"
#define BOLD_BLINK "\033[1;5m"
#define SUCCESS_BLINK "\033[1;5;33m"
#define LIGHT_BLUE_BG_WHITE_TEXT "\033[97;104m"

int main(int argc, char const *argv[])
{
	const int MAX_LENGTH_OF_PROMPTS = 100;

    FILE *output_stream = NULL;
	char prompt[MAX_LENGTH_OF_PROMPTS];
	int maxLengthOfSeq = 0;
	int menuOption = 0, rerunApp = 0;

	// Check if enough arguments are provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_stream>\n", argv[0]);
        fprintf(stderr, "Options for <output_stream>: stdout, stderr, or a file path.\n");
        return 1;
    }


    // Determine the output stream
    if (strcmp(argv[1], "stdout") == 0) {
        output_stream = stdout;
    } else if (strcmp(argv[1], "stderr") == 0) {
        output_stream = stderr;
    } else {
        // Treat as a file path
        output_stream = fopen(argv[1], "w");
        if (output_stream == NULL) {
            perror("Error opening file!\a");
            return 1;
        }
    }

    do {
	    fprintf(output_stream, SUCCESS_BLINK "Welcome to Sequence-Checker 2.0!\t:)\n" RESET );
	    fprintf(output_stream, BOLD "\n\n\t*************\n\tMENU\n\t*************\n" );
	    fprintf(output_stream,  "1. Verify input sequences contain prokaryotic coding sequences.\n" );
	    fprintf(output_stream,  "2. Show history.\n") ;
	    fprintf(output_stream,  "3. Exit." RESET);
	    fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");
	    scanf("%d", &menuOption);
	    fprintf(output_stream,  RESET "\n"); // Reset colors

	    while (menuOption < 1 || menuOption > 3) {
	    	fprintf(output_stream, ERROR_COLOR "\aThe number you entered doesn't correspond to any menu option.\nChoose one of the menu options (1-3)" RESET);
	    	fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");
	    	scanf("%d", &menuOption);
	    	fprintf(output_stream,  RESET "\n"); // Reset colors
	    }

	    if (menuOption == 1)
	    {
	    	fprintf(output_stream, SUCCESS_COLOR "Bring it on!" RESET);
	    	/* code */

	    } else if (menuOption == 2)
	    {
	    	fprintf(output_stream, SUCCESS_COLOR "Fine..." RESET);
	    	/* code */

	    } else {

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
	    while (rerunApp != 0 && rerunApp != 1) {
	    	fprintf(output_stream, ERROR_COLOR "\aYou should enter 1 for \"YES\" or 0 for \"NO\".\nSo, would you like to run the app again?\t(YES: 1\tNO: 0)" RESET);
    		fprintf(output_stream,  LIGHT_BLUE_BG_WHITE_TEXT "\n\nYour input:\t");	
	    	scanf("%d", &rerunApp);
	    	fprintf(output_stream,  RESET "\n"); // Reset colors
	    }
    } while (rerunApp == 1);

    fprintf(output_stream, SUCCESS_BLINK "Good... See ya around!\n" RESET);

    // Close the file stream if it's not stdout or stderr
    if (output_stream != stdout && output_stream != stderr) {
        fclose(output_stream);
    }
    return 0;
}