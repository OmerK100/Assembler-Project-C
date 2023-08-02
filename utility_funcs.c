#include "data.h"

char* createAS(char* argv) /* Adds the ".as" suffix to each file in order to open it. */
{
	int j = strlen(argv);
	char *fileName1 = (char *)malloc(sizeof(char)*(j+4));
	
	if (fileName1 == NULL)
	{
		printf("\nMemory allocation failure.\n\n");
		return NULL;	
	}
	strcpy(fileName1 , argv);
	*(fileName1 + j) = '.';
	*(fileName1 + j + 1) = 'a';
	*(fileName1 + j + 2) = 's';
	*(fileName1 + j + 3) = '\0';
	
	return fileName1;	
} 

/***************************************************************************/

char* createAM(char* argv) /* Adds the ".am" suffix to each file in order to make the pre_assembled version. */
{
	int j = strlen(argv);
	char *fileName2 = (char *)malloc(sizeof(char)*(j+4));
	
	if (fileName2 == NULL)
	{
		printf("\nMemory allocation failure.\n\n");
		return NULL;	
	}
	strcpy(fileName2 , argv);
	*(fileName2 + j) = '.';
	*(fileName2 + j + 1) = 'a';
	*(fileName2 + j + 2) = 'm';
	*(fileName2 + j + 3) = '\0';
	
	return fileName2;	
} 

/***************************************************************************/

void checkLegalMacroName(char *token , FILE *past)
/* checkLegalMacroName has an array with all saved names and makes sure that a macro name isn't the same as each of the saved names. */
{
	char savedNames[SAVED_NAMES][SAVED_NAMES] = {"mov" , "cmp" , "add" , "sub" , "lea" , "not" , "clr" , "inc" , "dec" , "jmp" , "bne" , "red" , "prn" , "jsr" , "rts" , "stop" , ".data" , ".string" , ".entry" , ".extern" , "r0" , "r1" , "r2" , "r3" , "r4" , "r5" , "r6" , "r7" , "mcr" , "endmcr"};
	int i = 0;

	while (i <= SAVED_NAMES)
	{
		if(strcmp(token , savedNames[i]) == STR_CMP_TRUTH)
		{
			printf("\nMacro name is illegal, saved name.\n\n"); /* Found illegal macro name. */
			past = NULL;	
			exit(0);	
		}
		i++;
	}
}

/***************************************************************************/

int twosComplement(int num , int which) /* twosComplement operates upon negative numebrs and by using a mask it creates a two's complement representation of the number. */
{
	unsigned int mask;
	int i;
	
	i = 1;
	num = ~num; /* Switching all int bits. */
	mask = 1<<(sizeof(int)*BYTE-1);
	num++;
	if (which == 1) /* Switching all unused bits to 0 (number with 12 bits).  */
	{
		while (i <= sizeof(int)*BYTE - CELL_SIZE + 2)
		{
			num = num^mask;
			mask = mask>>1;
			i++;
		} 
		return num;	
	}
	if (which == 2) /* Switching all unused bits to 0 (number with 14 bits). */
	{
		while (i <= sizeof(int)*BYTE - CELL_SIZE)
		{
			num = num^mask;
			mask = mask>>1;
			i++;
		} 
		return num;	
	}

	return FALSE;
}

/***************************************************************************/

int checkInteger(char *token) /* Checking if a string after '#' char represnets an integer. */
{
	int i = 0;
	
	if (token == NULL || token[0] == '\0')
	{
		return FALSE;		
	}
	if (token[LINE_BEGINNING] == '-' || token[LINE_BEGINNING] == '+') /* Sign before number. */
	{
		i++;
		if (isdigit(token[LINE_BEGINNING+1]) == FALSE)
		{
			return FALSE;
		}
	}
	while (*(token + i) != '\0')
	{
		if(isdigit(token[i]) == FALSE)
		{
			return FALSE;	
		}
		i++;
	}


	return TRUE;
}

/***************************************************************************/

int isLegalSymbol(char *token , int length , int *numOfErrors , int *fileLine)
/* isLegalSymbol runs various checks upon the declared symbol and makes sure the syntax of the symbol is correct. */
{
	int i;

	if (isLetter(token[LINE_BEGINNING]) == FALSE) /* First letter must be a letter. */
	{
		printf("\nError: attempt to declare a symbol in which first char isn't a letter | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE; 
	}
	for (i = 0 ; i < length ; i++) /* Each char after the first must be a letter or a digit. */
	{
		if (isdigit(token[i]) == FALSE && isLetter(token[i]) == FALSE)
		{
			printf("\nError: in attempt to declare symbol, one of the chars isn't a digit or a letter | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return FALSE; 	
		}		
	} 
	if (length > SYMBOL_SIZE) /* Length of symbol cannot exceed 30. */
	{
		printf("\nError: attempt to declare an oversized symbol | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE; 
	}
	
	return TRUE;
}

/***************************************************************************/

int isLegalSymbol2(char *token , int length , int *numOfErrors , int *fileLine)
/* isLegalSymbol runs various checks upon the declared symbol and makes sure the syntax of the symbol is correct. */
{
	int i;

	if (isLetter(token[LINE_BEGINNING]) == FALSE) /* First letter must be a letter. */
	{
		printf("\nError: using a symbol in which first char isn't a letter | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE; 
	}
	for (i = 0 ; i < length ; i++) /* Each char after the first must be a letter or a digit. */
	{
		if (isdigit(token[i]) == FALSE && isLetter(token[i]) == FALSE)
		{
			printf("\nError: using a symbol in which one of the chars isn't a digit or a letter | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return FALSE; 	
		}		
	} 
	if (length > SYMBOL_SIZE) /* Length of symbol cannot exceed 30. */
	{
		printf("\nError: using an oversized symbol | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE; 
	}
	
	return TRUE;
}

/***************************************************************************/

int* checkDataCorrectness(char *str , int *num_count)
/* checkDataCorrectness checks a string of numbers seperated by the commas for errors and puts the numbers in an array. */
{
	int len = strlen(str);
    	int i = 0;
    	int num_start = 1; 
    	int sign_allowed = 1; 
    	int comma_allowed = 0; 
    	int num;
    	int *arr = NULL;
	char c;
	char temp = '0';

    	*num_count = 0;
    	while (i < len) /* Loop that scans through the line. */ 	
	{
		c = str[i];
		if (isspace(c)) 
		{
			i++;
		} 
		else if (isdigit(c)) /* Digit of numebr is found, adding it to the next number we build. */ 
		{
			if (num_start == TRUE) 
			{
				num_start = FALSE;
				sign_allowed = FALSE;
				comma_allowed = TRUE;
				num =( c - '0');
			} 
			else 
			{
				num = num * 10 + (c - '0');
			}
			i++;
		} 
		else if (c == '+' || c == '-') /* Sign notifies beginning of number. */ 
		{
			if (sign_allowed == TRUE) 
			{
				num_start = TRUE;
				sign_allowed = FALSE;
				comma_allowed = FALSE;
				if (c == '-') 
				{	
					num = -1;
					temp = '-';
				} 
				else 
				{
                   			 num = 0;
               			}
                	i++;
            		} 
			else 
			{
				*num_count = -1; /* Type 1 error, illegal + or - sign. */
				free(arr);
                		return NULL;
            		}
        	} 
		else if (c == ',') /* Comma seperating between two numbers amd notifies of number ending so we store the number we found within the array. */
		{
			if (comma_allowed == TRUE) 
			{
				num_start = TRUE;
                		sign_allowed = TRUE;
                		comma_allowed = FALSE;
				if (checkRestOfLine(str , i) == FALSE) /* Making sure that there is more data after the comma because the string cannot end with a comma. */
				{	
					*num_count = -2; /* Type 2 error, data ending with a comma. */
					free(arr);
            				return NULL;	
				}
               			if (*num_count == 0) 
				{
                    			arr = (int *)malloc(sizeof(int));
                		} 
				else 
				{
                    			arr = (int *)realloc(arr, (*num_count + 1) * sizeof(int));
                		}
                		if (arr == NULL) 
				{
					return NULL;
                		}
				if (temp == '-')
				{
					arr[*num_count] = -1*num;
                			(*num_count)++;
                			i++;
					temp = '0';
				}
				else
				{
					arr[*num_count] = num;
                			(*num_count)++;
                			i++;
				}
                		
            		} 
			else 
			{
				*num_count = -3; /* Type 3 error, illegal comma. */
				free(arr);
                		return NULL;
            		}
        	} 
		else 
		{
			*num_count = -4; /* Type 4 error, miscellaneous text between numbers. */
			free(arr);
            		return NULL;
       		}
	}
	if (num_start == FALSE) /* Last number. */ 
	{
        	if (*num_count == 0) 
		{
            		arr = (int *)malloc(sizeof(int));
       		} 	
		else 
		{
            		arr = (int *)realloc(arr, (*num_count + 1) * sizeof(int));
       		}
        	if (arr == NULL) 
		{
			return NULL;
        	}
		if (temp == '-')
		{
			arr[*num_count] = (-1)*num;
			temp = '0';
			(*num_count)++;	
		}
		else if(temp != '-')
		{
			arr[*num_count] = num;
			(*num_count)++;
		}
        	
    	}

	return arr; 
}

/***************************************************************************/

int checkRestOfLine(char *str , int i) /* A function that receives a string and index and checks whether all the chars from the index to the end are white chars. */
{	
	i = i + 1;
	while(*(str+i) != '\0')
	{
		
		if(!isspace(str[i]))
		{
			return TRUE;
		}
		i++;
	}

	return FALSE;
}

/***************************************************************************/

int reachFirstDigit(char *line) /* A function that receives a string and finds the firt index in which a number or a sign begins, will be used for the .data functions. */
{
	int i = 0;

	while(*(line + i) != '\0')
	{
		if (isdigit(line[i]) || line[i] == '+' || line[i] == '-')
		{
			return i;
		}
		i++;
	}

	return FALSE;
}

/***************************************************************************/

void dataErrorCases(int *x , int *fileLine) /* A function that receives a type of error found in the .data command arguments and prints the correct error. */
{
	if (*x == -1)
	{
		printf("\nError: + or - sign in a not allowed space in between the numbers of the .data command | line: %d\n\n" , *fileLine);
	}
	else if (*x == -2)
	{
		printf("\nError: no more data after a comma, data cannot end with a comma | line: %d\n\n" , *fileLine);
	}
	else if (*x == -3)
	{
		printf("\nError: comma in a not allowed space in between the numbers of the .data command | line: %d\n\n" , *fileLine);
	}
	else if (*x == -4)
	{
		printf("\nError: miscellaneous text in between numebrs that come after .data command | line: %d\n\n" , *fileLine);
	}
}

/***************************************************************************/

int isLetter(char c) /* Function checks whether a char is a letter (small or capital). */
{
	if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z'))
	{
		return FALSE; 
	}

	return TRUE;	
}

/***************************************************************************/

int missingCommas(char *line) /* When checking operations with 2 opernads we want to make sure there's only a single comma, this function checks whether there are none. */
{
	int length = strlen(line);
	int i;

	for (i = 0 ; i < length ; i++)
	{
		if (line[i] == ',')
		{
			return FALSE; /* A comma has been found. */
		}	
	}
	
	return TRUE; /* No commas. */	
}

/***************************************************************************/

int consecutiveCommas(char *line) /* When checking operations with 2 opernads we want to make sure there's only a single comma, this function checks whether there are more than one. */
{
	int length = strlen(line);
	int i;
	int commasFound = 0;

	for (i = 0 ; i < length ; i++)
	{
		if (line[i] == ',')
		{
			commasFound++;
			if (commasFound >= 2) /* At least 2 commas, not allowed. */
			{
				return TRUE;	
			}
		}	
	}
	
	return FALSE;	
}

/***************************************************************************/

int checkSavedSymbolName(char *token) /* Checks whether a word is same as a saved name (operation/register/etc). */
{
	char savedNames[SAVED_NAMES][SAVED_NAMES] = {"mov" , "cmp" , "add" , "sub" , "lea" , "not" , "clr" , "inc" , "dec" , "jmp" , "bne" , "red" , "prn" , "jsr" , "rts" , "stop" , ".data" , ".string" , ".entry" , ".extern" , "r0" , "r1" , "r2" , "r3" , "r4" , "r5" , "r6" , "r7" , "mcr" , "endmcr"};
	int i = 0;

	while (i <= SAVED_NAMES)
	{
		if(strcmp(token , savedNames[i]) == STR_CMP_TRUTH)
		{	
			return TRUE;
		}
		i++;
	}

	return FALSE;
}

/***************************************************************************/

int checkSavedOperationName(char *token) /* Checks whether a word is same as a saved legal beginning name (operation/data/etc). */
{
	char savedNames[SAVED_NAMES_TWO][SAVED_NAMES_TWO] = {"mov" , "cmp" , "add" , "sub" , "lea" , "not" , "clr" , "inc" , "dec" , "jmp" , "bne" , "red" , "prn" , "jsr" , "rts" , "stop" , ".data" , ".string" , ".entry" , ".extern"};
	int i = 0;

	while (i <= SAVED_NAMES_TWO)
	{
		if(strcmp(token , savedNames[i]) == STR_CMP_TRUTH)
		{	
			return TRUE;
		}
		i++;
	}

	return FALSE;
}

/***************************************************************************/

char* convertToExt(char *nameOfFile) /* Receives a file's name and adds a suffix of ".ext" for external file. */
{
	int i = strlen(nameOfFile) - 2;
	char *name = (char *)malloc(sizeof(char)*i + 3); /* Updated name. */

	if (name == NULL)
	{
		printf("\nMemory allocation failure.\n\n");
		return NULL;		
	}
	strcpy(name , nameOfFile);
	*(name + i) = 'e';
	*(name + i + 1) = 'x';
	*(name + i + 2) = 't';
	*(name + i + 3) = '\0';

	return name;
}

/***************************************************************************/

char* convertToEnt(char *nameOfFile) /* Receives a file's name and adds a suffix of ".ent" for enternal file. */
{
	int i = strlen(nameOfFile) - 2;
	char *name = (char *)malloc(sizeof(char)*i + 3); /* Updated name. */

	if (name == NULL)
	{
		printf("\nMemory allocation failure.\n\n");
		return NULL;		
	}
	strcpy(name , nameOfFile);
	*(name + i) = 'e';
	*(name + i + 1) = 'n';
	*(name + i + 2) = 't';
	*(name + i + 3) = '\0';

	return name;	
}

/***************************************************************************/

char* convertToOb(char *nameOfFile) /* Receives a file's name and adds a suffix of ".ob" for object file. */
{
	int i = strlen(nameOfFile) - 2;
	char *name = (char *)malloc(sizeof(char)*i + 3); /* Updated name. */

	if (name == NULL)
	{
		printf("\nMemory allocation failure.\n\n");
		return NULL;		
	}
	strcpy(name , nameOfFile);
	*(name + i) = 'o';
	*(name + i + 1) = 'b';
	*(name + i + 2) = '\0';

	return name;
}

/***************************************************************************/	

void toBinary(int num , char *binary) /* Converts an int into the special binary representation ('/' and '.'). */
{
	int i = 1;
	unsigned int mask = 1<<(sizeof(int)*BYTE-1); /* mask for converting each bit. */
	
	while (i <= (BYTE*sizeof(int))-CELL_SIZE) /* Ignoring int's bits until first 14. */
	{
		mask = mask>>1;
		i++;
	} 
	i = 0;
	while (mask != 0)
	{
		if(num&mask) /* '.' is for 0, '/' is for 1. */
		{
			binary[i] = '/';
		}
		else
		{
			binary[i] = '.';
		}
		mask = mask>>1;	
		i++;
	}
	binary[i] = '\0';
}

/***************************************************************************/

int isEmpty(FILE *file)
{
	long savedOffset = ftell(file);
    	
	fseek(file, 0, SEEK_END);
	if (ftell(file) == 0)
	{
        	return TRUE;
    	}
	fseek(file, savedOffset, SEEK_SET);
   	return FALSE;
}
