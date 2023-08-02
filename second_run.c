#include "data.h"

int secondRun(int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine , assemblyType *Instructions , assemblyType *Data , char *nameOfFile)
/* secondRun function initializes important variables, and passes each symbol from the skip table we skipped in the first run for searching its declaration adress. */
{
	assemblyType *assembly = (assemblyType *)calloc(MEMORY_SIZE , sizeof(assemblyType)); /* Final assembly data structure, will be made from the Instructions and Data structures. */
	advancedExPtr advancedExHead = NULL; /* Structure to include both external symbols and their adresses. */
	skipPtr p1 = *skipHead;
	char symbol[LINE_SIZE];
	int adress; /* Hold the adress of loacted symbols. */
	int i = 0;
	int actualadress = MEMORY_BEGIN; /* Data in the assembly structre will begin from the 100th memory cell. */
	int *actualAdress = &actualadress;

	while (p1 != NULL) /* Running through the skip table and passing each symbol to look for its adress. */
	{
		strcpy(symbol , (p1->skipTable_entry).skipName);
		adress = (p1->skipTable_entry).skipAdress;
		*fileLine = (p1->skipTable_entry).skipFileLine;
		run2(symbol , adress , IC , DC , numOfErrors , symbolHead , ExHead , EnHead , &advancedExHead , fileLine , Instructions , Data);
		p1 = p1->skipTable_next;
	}
	while (i < *IC) /* Creating assembly using Instructions and Data. */
	{
		assembly[*actualAdress] = Instructions[i];
		*actualAdress = *actualAdress + 1;
		i++;
	}
	i = 0;
	while (i < *DC)
	{
		assembly[*actualAdress] = Data[i];
		*actualAdress = *actualAdress + 1;
		i++;	
	} 
	if (*numOfErrors == NO_ERRORS) /* If no errors have been found by the end of the encoding, we pass the assembly and other data to file output creation. */
	{
		if (createOutputs(IC , DC , symbolHead , ExHead , EnHead , &advancedExHead , assembly , nameOfFile) == TRUE)
		{
			printf("\nOutput creation success.\n\n");
			free(assembly);
			free(advancedExHead);
			return TRUE;
		}
		printf("\nOutput creation failure.\n\n");
		return FALSE;
	}

	return FALSE;
}

/***************************************************************************/

void run2(char *symbol , int adress , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , advancedExPtr *advancedExHead , int *fileLine , assemblyType *Instructions , assemblyType *Data)
/* run2 receives a symbol we skipped, checks whether the symbol is legal/not saved and looks for the symbol declaration in the symbol table or for the symbol declaration as an extern. If none found there's an error. */
{
	int wheredeclared;
	int *whereDeclared = &wheredeclared;
	char whatSymbol[LINE_SIZE];

	if (checkForProblems2(symbol , numOfErrors , fileLine) == FALSE) /* Problems within the symbol such as syntax legallity. */
	{
		return;
	}
	if (lookForSymbol(symbol , whatSymbol , adress , whereDeclared , symbolHead , ExHead , EnHead) == FALSE) /* Looking for the symbol within the symbol table or external table. */
	{
		printf("\nError: in the second run, potential symbol %s we skipped in the first run that belongs to an instruction in line %d wasn't declared or isn't an extern.\n\n" , symbol , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	else
	{
		if (strcmp(whatSymbol , "extern") == STR_CMP_TRUTH && *numOfErrors == NO_ERRORS) /* Encoding the symbol's adress found in the lines we skipped according to the symbol kind we found. */
		{
			
			Instructions[adress].symbolUsed.are = 1;
			Instructions[adress].symbolUsed.declareAdress = 0;
			addToAdvancedExTable(adress , symbol , advancedExHead , numOfErrors);
		}
		else if ((strcmp(whatSymbol , "data") == STR_CMP_TRUTH || strcmp(whatSymbol , "string") == STR_CMP_TRUTH) && (*numOfErrors == NO_ERRORS))
		{	
			Instructions[adress].symbolUsed.are = 2;
			Instructions[adress].symbolUsed.declareAdress = *whereDeclared + *IC + MEMORY_BEGIN;	
		}
		else if (*numOfErrors == NO_ERRORS)
		{
			Instructions[adress].symbolUsed.are = 2;
			Instructions[adress].symbolUsed.declareAdress = *whereDeclared + MEMORY_BEGIN;
		}
	}
}

/***************************************************************************/

int checkForProblems2(char *symbol , int *numOfErrors , int *fileLine)
/* checkForProblems2 Checks for different problems within the symbol we received. */
{
	int length;

	length = strlen(symbol);
	if (isLegalSymbol2(symbol , length , numOfErrors , fileLine) == FALSE) /* Is legal symbol? */
	{
		return FALSE;
	}
	if (checkSavedSymbolName(symbol) == TRUE) /* Is saved name? */
	{
		printf("\nError: for the second run, potential symbol %s we skipped in the first run that belongs to an instruction in line %d is a saved name.\n\n" , symbol , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************/

void addToAdvancedExTable(int adress , char *symbol , advancedExPtr *advancedExHead , int *numOfErrors)
/* addToAdvancedExTable adds a symbol and its adress the the second external table. */
{
	advancedExPtr t = (advancedExPtr)malloc(sizeof(advancedExItem));
	advancedExPtr p1 = *advancedExHead;
	advancedExPtr p2;

	if (t == NULL)
	{
		printf("\nMemory allocation and thus addition of symbol to advanced external list failure.\n\n");
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	strcpy((t->advancedExTable_entry).advancedSymbolEx , symbol); /* New node with the external symbol data. */
	(t->advancedExTable_entry).whereUsedInFile = MEMORY_BEGIN + adress;
	while (p1 != NULL) /* Adding the new node to the list. */
	{
		p2 = p1;
		p1 = p1->advancedExTable_next;
	}
	if (p1 == *advancedExHead)	
	{
		*advancedExHead = t;
		t->advancedExTable_next = p1;
	}
	else
	{
		p2->advancedExTable_next = t;
		t->advancedExTable_next = p1;
	} 			
}

/***************************************************************************/

int lookForSymbol(char *symbol , char *whatSymbol , int adress , int *whereDeclared , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead)
/* lookForSymbol receives a symbol name and looks for it on the external or symbol tables. */
{
	symbolPtr p1 = *symbolHead;
	ExPtr p2 = *ExHead;
	
	if (p1 == NULL)
	{
		return FALSE;
	}
	while (p1 != NULL) /* Running and comparing through the symbol table. */
	{
		if (strcmp((p1->symbolTable_entry).symbolName , symbol) == STR_CMP_TRUTH)
		{
			strcpy(whatSymbol , (p1->symbolTable_entry).symbolKind);
			*whereDeclared = (p1->symbolTable_entry).symbolAdress;
			return 1;
		}
		p1 = p1->symbolTable_next;
	}
	if (p2 == NULL)
	{
		return FALSE;
	}
	while (p2 != NULL) /* Running and comparing through the external table. */
	{
		if (strcmp((p2->ExTable_entry).symbolEx , symbol) == STR_CMP_TRUTH)
		{
			strcpy(whatSymbol , "extern");
			adress = 0;
			return TRUE;
		}
		p2 = p2->ExTable_next;	
	}
	
	return FALSE;	
}
