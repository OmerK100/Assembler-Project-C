#include "data.h"

FILE* preAssembly(FILE *pre , char *nameOfPast) 
/* First pre-assembly function initializes important variables and passes each file line for searching for macros. */
{	
	FILE *past = fopen(nameOfPast , "w+"); /* This will be the pre-assembled file by the end. */
	int mcrflag = FLAG_OFF;
	int *mcrFlag = &mcrflag;
	ptr Head = NULL;
	char line[LINE_SIZE]; /* Stores each line gathered from the original file. */
	
	if (past == NULL)
	{
		printf("\nOpening file: %s for writing failed.\n\n" , nameOfPast);
		return NULL;
	}
	while (fgets(line , LINE_SIZE , pre) != NULL) /* Running through the file and passing each line into sacnning. */
	{
		searchLine(pre , past , &Head , mcrFlag , line);
	}
	fclose(pre);
	fclose(past);
	free(Head);
	
	return past; /* Returning final pre-assembled file. */
}

/***************************************************************************/

void searchLine(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line)
/* searchLine scans the received file line for potential macros/macro declarartions and passes its findings to next functions. */
{
	char copyLine[LINE_SIZE]; /* Copy of the original line for scanning purposes. */
	char *token; /* strtok function and token will accompany us through the whole project and will help us to seperate sentences into words. */
	
	strcpy(copyLine , line);
	token = strtok(copyLine , " \t\n\f\r");
	if (token == NULL) /* Empty line or only white chars, automatically printed in the pre-assembly file. */
	{
		fprintf(past , "%s" , line);
		return;	
	}
	else if (line[LINE_BEGINNING] == ';')
	{
		fprintf(past , "%s" , line);
		return;
	}
	if (strcmp(token , "mcr") == STR_CMP_TRUTH) /* Potential macro declaration passed for further searching. */
	{
		maybeNewMcr(pre , past , Head , mcrFlag , line , copyLine , token);	
	}
	else
	{
		if (maybeReplace(pre , past , Head , mcrFlag , line , copyLine , token) == FALSE) /* Potential macro usuage in the file. */
		{
			
			fprintf(past , "%s" , line); /* No macro usage nor macro declaration, line can be directly printed at pre-assembly file. */
		}
	}		
}

/***************************************************************************/

void maybeNewMcr(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token)
/* maybeNewMcr decides whether there is a new macro declaration or not, and if there is makes sure to copy its data into the macro list. */
{
	char copyLine2[LINE_SIZE];
	char* token2;
	
	strcpy(copyLine2 , line);
	token = strtok(NULL , " \t\n\f\r");
	token2 = strtok(copyLine2 , " \t\n\f\r");
	token2 = strtok(NULL , " \t\n\f\r");
	token2 = strtok(NULL , " \t\n\f\r");
	if (token == NULL) /* Checking for potential errors. */
	{
		printf("\nMacro name is missing after declaration attempt, ending.\n\n");
		past = NULL;
		exit(0);
	}
	if (token2 != NULL)
	{
		printf("\nIllegal miscellaneous text after macro declaration, ending.\n\n");
		past = NULL;
		exit(0);		
	}
	*mcrFlag = FLAG_ON;
	addToTable(pre , past , Head , mcrFlag , line , copyLine , token); /* Adding macro to the list. */	
}

/***************************************************************************/

void addToTable(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token)
/* addToTable receives the input file in a state where a macro has been declared so it's adding the macro name and its data do the macro list. */
{
	char copyLine2[LINE_SIZE];
	ptr t = (ptr)malloc(sizeof(mcrItem)); /* Creating new node for the macro list. */
	ptr p1 = *Head;
	ptr p2;
	char *buff;

	if (t == NULL)
	{
		printf("\nAddition to list failure due to memory allocation failure, ending.\n\n");
		past = NULL;	
		exit(0);
	}
	checkLegalMacroName(token , past); /* Making sure macro name isn't a saved name. */
	strcpy((t->macro_entry).mcrID , token); /* Adding macro name to the node. */
	buff = copyToMcr(pre , past , Head , mcrFlag , line , copyLine , token , copyLine2); /* Copying macro's data to a special char array. */
	(t->macro_entry).mcrInside = buff; /* Adding macro data to the node. */
	while (p1 != NULL) /* Adding the new macro node to the list. */
	{
		p2 = p1;
		p1 = p1->macro_next;
	}
	if (p1 == *Head)	
	{
		*Head = t;
		t->macro_next = p1;
	}
	else
	{
		p2->macro_next = t;
		t->macro_next = p1;
	}
}

/***************************************************************************/

char* copyToMcr(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token , char *copyLine2)
/*copyToMcr runs through the file after we stumbled upon a macro declaration and saves its data until "endmcr" command. */
{
	int i = 0;
	int j = 0;
	char *buffer = (char *)malloc((i+1)*sizeof(char)); /* Will store the macro's data. */
	char *token2;

	while (!feof(pre))
	{
		fgets(copyLine , LINE_SIZE , pre);
		strcpy(line , copyLine);
		strcpy(copyLine2 , line);
		token = strtok(copyLine , " \t\n\f\r");
		token2 = strtok(copyLine2 , " \t\n\f\r");
		if (token != NULL)
		{
			if (strcmp(token , "endmcr") == STR_CMP_TRUTH) /* Line includes "endmcr", we check for potential errors and stop filling the data array. */
			{
				token2 = strtok(copyLine2 ," \t\n\f\r");
				token2 = strtok(NULL ," \t\n\f\r");
				if (token2 != NULL)
				{
					printf("\nIllegal miscellaneous text after macro declaration.\n\n");
					past = NULL;
					exit(0);
				}
				*mcrFlag = FLAG_OFF;
				break;
			}
			while (*(line+j) != '\0') /* "endmcr" wasn't encountered so we keep adding each of macro's data chars to the data array and reallocating memory. */
			{
				buffer[i] = line[j];
				i++;
				j++;
				buffer = (char *)realloc(buffer , (i+1)*sizeof(char));
			}
			j = 0;		
		}
		else
		{
			while (*(line+j) != '\0')
			{
				buffer[i] = line[j];
				i++;
				j++;
				buffer = (char *)realloc(buffer , (i+1)*sizeof(char));
			}
			j = 0;		
		}
	} 
	buffer[i] = '\0';
	
	return buffer; /* Returning data collected. */
} 

/***************************************************************************/

int maybeReplace(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token)
/* maybeReplace checks for potential command that calls for a macro. if there's a call, it finds the macro and places the macro's data instead of the call. */
{
	ptr p1 = *Head;
	
	if (p1 == NULL)
	{
		return FALSE;
	}
	while (p1 != NULL) /* Running through the macro list. */
	{
		if (strcmp((p1->macro_entry).mcrID , token) == STR_CMP_TRUTH) /* Checking whether the command called matches a mcro. */
		{
			token = strtok(NULL , " \t\n\f\r");
			if (token != NULL)
			{
				printf("\nIllegal miscellaneous text after macro usage.\n\n");
				past = NULL;
				exit(0);
			}
			fprintf(past , "%s" , (p1->macro_entry).mcrInside); /* If a macro is confirmed and found we print its data instead of the call for the macro in the pre-assembly file. */
			return 1;
		}
		p1 = p1->macro_next;
	}
	
	return FALSE;
}
