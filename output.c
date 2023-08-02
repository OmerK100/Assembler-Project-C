#include "data.h"

int createOutputs(int *IC , int *DC , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , advancedExPtr *advancedExHead , assemblyType *assembly , char *nameOfFile)
/* Receives parameters from the second run, and activates functions that create .ob , .ent and .ext output files. */
{
	ExPtr p1 = *ExHead;
	EnPtr p2 = *EnHead;
	
	if (createObject(IC , DC , assembly , nameOfFile) == FALSE)
	{
		printf("\nCreating .ob file failed.\n\n"); /* Creating .ob file using the assembly we made. */  
		return FALSE;
	}
	else
	{
		printf("\nCreating .ob file was successful.\n\n");
	}
	if (p1 != NULL) /* Checking if external list is empty. */ 
	{
		if (createExternal(symbolHead , advancedExHead , nameOfFile) == FALSE) /* Creating .ext file using the second external table we created. */
		{
			printf("\nCreating .ext file failed.\n\n");
			return FALSE;
		}
		else
		{
			printf("\nCreating .ext file was successful.\n\n");
		}
	}
	else
	{	
		printf("\nNo extern symbols were used, .ext file isn't created.\n\n");
	}
	if (p2 != NULL) /* Checking if enternal list is empty. */ 
	{
		if (createEnternal(symbolHead , EnHead , nameOfFile , IC) == FALSE) /* Creating .ent file using the enternal table we created. */ 
		{
			printf("\nCreating .ent file failed.\n\n");
			return FALSE;
		}
		else
		{
			printf("\nCreating .ent file was successful.\n\n");	
		}
	}
	else
	{
		printf("\nNo entry symbols were used, .ent file isn't created.\n\n");	
	}

	return TRUE;
}

/***************************************************************************/

int createObject(int *IC , int *DC , assemblyType *assembly , char *nameOfFile)
/* createObject uses the assembly data structure and passes to conversion each one of assembly's numbers to special binary representation. */
{
	int i = 0;
	int ad = MEMORY_BEGIN;
	int *adress = &ad;
	char *name;
	FILE *object; /* .ob output file. */	

	name = convertToOb(nameOfFile);
	object = fopen(name , "w");
	/*object = fopen("finaltest.txt" , "w");*/ /* Opening the .ob file for writing. */
	free(name);
	if (object == NULL)
	{
		printf("\nCreation of .ob file failure.\n\n");
		return FALSE;	
	}
	insertAmount(object , IC , DC); /* Inserting how many data commands and instructions are within the assembly using IC and DC. */
	while (i < *IC + *DC) /* Running through each assembly cell and converting the number into binary. */
	{
		insertBinary(object , assembly , adress);
		i++;	
	}	
	
	return TRUE;
}

/***************************************************************************/

int createExternal(symbolPtr *symbolHead , advancedExPtr *advancedExHead , char *nameOfFile)
/* createExternal uses the second external data structure and passes to conversion each extern symbol to special binary representation. */
{
	char *name;
	FILE *external; /* .ext output file. */
	advancedExPtr p1 = *advancedExHead;

	name = convertToExt(nameOfFile); 
	external = fopen(name , "w");
	/*external = fopen("extern.txt" , "w");*/ /* Opening the .ext file for writing. */
	free(name);	
	if (external == NULL)
	{
		printf("\nCreation of .ext file failure.\n\n");
		return FALSE;
	}
	while (p1 != NULL)
	{
		insertExtern(external , (p1->advancedExTable_entry).advancedSymbolEx , (p1->advancedExTable_entry).whereUsedInFile); 
		/* Passing to conversion and insertion to .ext file each external symbol. */
		p1 = p1->advancedExTable_next;
	}

	return TRUE;	
}

/***************************************************************************/

void insertExtern(FILE *external , char *symbolEx , int num)
{
	char line[LINE_SIZE]; /* Will store the symbol name. */
	int length;
	char *binary = (char *)malloc(LINE_SIZE*sizeof(char)); /* Will store the binary representation. */

	strcpy(line , symbolEx);
	length = strlen(line);
	line[length] = '\t';
	line[length+1] = '\0';
	toBinary(num , binary); /* Conerting the adress into bianry. */
	strcpy(line+length+1 , binary);
	length = strlen(line);
	line[length] = '\n';
	line[length+1] = '\0';
	fprintf(external , "%s" , line); /* Printing the converted representation into the .ext file. */
	free(binary);		
}

/***************************************************************************/

int createEnternal(symbolPtr *symbolHead , EnPtr *EnHead , char *nameOfFile , int *IC)
{
	char *name;
	FILE *enternal; /* .ent output file. */
	EnPtr p1 = *EnHead;
	int num;

	name = convertToEnt(nameOfFile);
	enternal = fopen(name , "w");
	/*enternal = fopen("entry.txt" , "w");*/ /* Opening the .ent file for writing. */
	free(name);
	if (enternal == NULL)
	{
		printf("\nCreation of .ent file failure.\n\n");
		return FALSE;
	}
	while (p1 != NULL)
	{
		num = lookForSymbolAdress((p1->EnTable_entry).symbolEn , symbolHead , IC); /* Looking for adress for each enternal symbol through the symbol table. */
		insertEntry(enternal , (p1->EnTable_entry).symbolEn , num);
		/* Passing to conversion and insertion to .ent file each enternal symbol. */	
		p1 = p1->EnTable_next;
	}

	return TRUE;	
}

/***************************************************************************/

int lookForSymbolAdress(char *symbolEn , symbolPtr *symbolHead , int *IC)
/* lookForSymbolAdress receives a symbol name from the enternal list and looks for the symbol declaration node within the symbol table and returnin teh declaration adress. */
{
	symbolPtr p1 = *symbolHead;
	
	while (p1 != NULL) /* Running through symbol table and comparing each symbol to the parameter received. If a match is found we return the corresponding adress. */
	{
		if (strcmp((p1->symbolTable_entry).symbolName , symbolEn) == STR_CMP_TRUTH)
		{
			if (strcmp((p1->symbolTable_entry).symbolKind , "string") == STR_CMP_TRUTH || strcmp((p1->symbolTable_entry).symbolKind , "data") == STR_CMP_TRUTH)
			{
				return ((p1->symbolTable_entry).symbolAdress + MEMORY_BEGIN + *IC); 
			}
			else
			{
				return ((p1->symbolTable_entry).symbolAdress + MEMORY_BEGIN); 
			}
		}
		p1 = p1->symbolTable_next;
	}
	
	return FALSE;
}

/***************************************************************************/

void insertEntry(FILE *enternal , char *symbolEn , int num)
{
	char line[LINE_SIZE]; /* Will store the symbol name. */
	int length;
	char *binary = (char *)malloc(LINE_SIZE*sizeof(char)); /* Will store the binary representation. */

	strcpy(line , symbolEn);
	length = strlen(line);
	line[length] = '\t';
	line[length+1] = '\0';
	toBinary(num , binary); /* Conerting the adress into bianry. */
	strcpy(line+length+1 , binary);
	length = strlen(line);
	line[length] = '\n';
	line[length+1] = '\0';
	fprintf(enternal , "%s" , line); /* Printing the converted representation into the .ent file. */
	free(binary);	
}

/***************************************************************************/

void insertAmount(FILE *object , int *IC , int *DC) /* Inserts into the object file the number of instructions (IC) and number of data commands (DC). */
{
	char line[LINE_SIZE];
	int length;
	
	/* Printing both numbers into a string with a space between them. */
	line[LINE_BEGINNING] = '\t';
	line[LINE_BEGINNING+1] = '\0';
	sprintf(line+1 , "%d" , *IC);
	length = strlen(line);
	line[length] = ' ';
	line[length+1] = '\0';
	sprintf(line+length+1 , "%d" , *DC);
	length = strlen(line);
	line[length] = '\n';
	line[length+1] = '\0'; 
	fprintf(object , "%s" , line); /* Printing the string into the object file. */
}

/***************************************************************************/

void insertBinary(FILE *object , assemblyType *assembly , int *adress) /* Receives an assembly cell and converts the number inside into binary representation. */
{
	char buff[LINE_SIZE];
	char line[LINE_SIZE]; /* Final object line. */
	char *binary = (char *)malloc(LINE_SIZE*sizeof(char)); /* Will store the binary representation. */
	int length;
	int num;
	int *p1 = (int *)(&assembly[*adress]); /* Converting assmbly type to int. */

	line[LINE_BEGINNING] = '0';
	sprintf(line+1 , "%d" , *adress); /* Printing the next adress into the line. */
	length = strlen(line);
	line[length] = '\t';
	line[length+1] = '\0';
	sprintf(buff , "%d" , *p1);
	num = atoi(buff); /* Turning assembly value into int. */
	toBinary(num , binary); /* Converting assembly value to binary. */
	strcpy(line+length+1 , binary);
	length = strlen(line);
	line[length] = '\n';
	line[length+1] = '\0';
	fprintf(object , "%s" , line); /* Printing the bianry conversion into the object file. */  
	*adress = *adress + 1; 
	free(binary);
}
