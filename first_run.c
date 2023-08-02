#include "data.h"

int firstRun(char *nameOfFile) /* firstRun function initializes important variables, and passes each pre-assembled line for encoding. */
{
	FILE *f1 = fopen(nameOfFile , "r");
	assemblyType *Instructions = (assemblyType *)calloc(MEMORY_SIZE , sizeof(assemblyType)); /* Initializing data structures for storing instructions and data calls. */
	assemblyType *Data = (assemblyType *)calloc(MEMORY_SIZE , sizeof(assemblyType));
	int ic = 0; /* Counts instruction commands encoding lines. */	
	int *IC = &ic;
	int dc = 0; /* Counts data commands encoding lines. */	
	int *DC = &dc; 
	int numoferrors = 0; /* Counts amount of errors. */
	int *numOfErrors = &numoferrors;
	int fileline = 0; /* Counts each line, will be used for error displaying. */
	int *fileLine = &fileline;
	symbolPtr symbolHead = NULL; /* Data structure for storing declared symbols. */
	skipPtr skipHead = NULL; /* Data structure for stroing symbols to be skipped for second run. */
	ExPtr ExHead = NULL; /* Data structure for storing extern symbols. */
	EnPtr EnHead = NULL; /* Data structure for storing entry symbols. */
	char line[LINE_SIZE];

	if (f1 == NULL)
	{
		printf("\nOpening file: %s for reading failed.\n\n" , nameOfFile);
		printf("------------------------------------------------------\n");
		return FALSE;
	}
	while (fgets(line , LINE_SIZE , f1) != NULL) /* Counting lines and passing to next functions. */
	{
		if (*IC + *DC > MEMORY_SIZE-MEMORY_BEGIN) /* Making sure the encoding doesn't exceed allowed memory limit. */
		{
			printf("\nError: encoding has exceeded the allowed memory usuage limit of the machine, assembly terminates.");
			*numOfErrors = *numOfErrors + 1;
			break;	
		}
		*fileLine = *fileLine + 1;
		run1(f1 , line , IC , DC , numOfErrors , &symbolHead , &skipHead , &ExHead , &EnHead , fileLine , Instructions , Data);
	} 
	checkLegitExEnTable(IC , DC , numOfErrors , &symbolHead , &ExHead , &EnHead , fileLine); /* After finishing the first run checking for some errors within entry and extern tables. */
	fclose(f1);
	if (*numOfErrors == NO_ERRORS) /* If no errors were found then there's a point at going for the second run, else we end. */
	{
		if (secondRun(IC , DC , numOfErrors , &symbolHead , &skipHead , &ExHead , &EnHead , fileLine , Instructions , Data , nameOfFile) == 1)
		/* Activating the second run for further encoding. */
		{
			printf("\nAssembly is complete for file: %s, no errors have been detected.\n\n" , nameOfFile);
			printf("------------------------------------------------------\n");
			return TRUE;		
		}		
	}
	else
	{
		checkSkipTableErrors(&symbolHead , &ExHead , &EnHead , &skipHead , numOfErrors , fileLine);
	}
	free(Instructions);
	free(Data);
	
	printf("\nIn file: %s at least one error has been detected, ending with no output creation.\n\n" , nameOfFile);
	printf("------------------------------------------------------\n");
	return FALSE;
}

/***************************************************************************/

void run1(FILE *f1 , char *line , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine , assemblyType *Instructions , assemblyType *Data)
/* run1 receives each pre-assembled line, scans the line for potential symbol/data/instructions/etc declarations and passes the line to the next functions. */
{
	char *token;
	char copyLine[LINE_SIZE];
	char copyLine2[LINE_SIZE];
	int errors = *numOfErrors; /* We'll use errors to save current number of errors, and if an error was found in a line it will help us to stop searching the line and move on. */

	strcpy(copyLine , line);
	strcpy(copyLine2 , line);
	token = strtok(copyLine , " \t\n\f\r");
	if (token == NULL) /* No text in line, except maybe white chars so we skip. */
	{
		return;
	}
	else if (line[LINE_BEGINNING] == ';') /* comment line, moving on. */
	{
		return;
	}
	else if (strcmp(token , ".data") == STR_CMP_TRUTH || strcmp(token , ".string") == STR_CMP_TRUTH) /* Potential .data or .string command, passed for further checking if needed. */
	{
		checkDataOrString(line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Data);
	} 
	else if (strcmp(token , ".entry") == STR_CMP_TRUTH || strcmp(token , ".extern") == STR_CMP_TRUTH) /* Potential .entry or .extern command, passed for further checking if needed. */
	{
		checkEntryOrExtern(line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , ExHead , EnHead , fileLine);
	} 
	else if (checkForSymbol(line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine) == TRUE) /* Checking the first word for potential symbol declaration. */
	{	
		if (*numOfErrors - errors != 0) /* Making sure no errors have been found during symbol finding. */
		{
			return;
		}
		token = strtok(copyLine2 , " \t\n\f\r");
		token = strtok(NULL , " \t\n\f\r"); 
		/* Checking line for further data after symbol declaration. */
		if (strcmp(token , ".entry") == STR_CMP_TRUTH || strcmp(token , ".extern") == STR_CMP_TRUTH) /* Checking for .entry or .extern commands. */
		{	
			checkEntryOrExtern(line , copyLine2 , token , IC , DC , numOfErrors , symbolHead , skipHead , ExHead , EnHead , fileLine);	
		}
		else if (strcmp(token , ".data") == STR_CMP_TRUTH || strcmp(token , ".string") == STR_CMP_TRUTH) /* Checking for .data or .string commands. */
		{	
			checkDataOrString(line , copyLine2 , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Data);	
		}
		else
		{
			checkOperationCode(line , IC , DC , numOfErrors , symbolHead , skipHead , 2 , fileLine , Instructions); /* Checking for operations. */		
		}
	}
	else 
	{
		if (*numOfErrors - errors != 0)
		{
			return;
		}
		checkOperationCode(line , IC , DC , numOfErrors , symbolHead , skipHead , 1 , fileLine , Instructions); /* No symbol declaration, passing for operation checking. */
	}
}
	
/***************************************************************************/

void checkOperationCode(char *line , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int which , int *fileLine , assemblyType *Instructions)
/* checkingOperationsCode has been notified for potential operation usuage and passes potential operation name and line for further searching. */
{
	char copyLine[LINE_SIZE];
	char *token;
	
	strcpy(copyLine , line);
	token = strtok(copyLine , " \t\n\f\r");
	if (which == 2)
	{
		token = strtok(NULL , " \t\n\f\r");
	}
	checkOperationName(line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);	
}

/***************************************************************************/

void checkOperationName(char *line  , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* checkOperationName decides which operation has been used, and it knows how many operands each operation should get and passes its findings for further searching. We have options for zero operands, one or two. */
{
	if (strcmp(token , "mov") == STR_CMP_TRUTH)
	{	
		twoOpsCheck(MOV , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "cmp") == STR_CMP_TRUTH)
	{
		twoOpsCheck(CMP , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "add") == STR_CMP_TRUTH)
	{
		twoOpsCheck(ADD , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "sub") == STR_CMP_TRUTH)
	{
		twoOpsCheck(SUB , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "not") == STR_CMP_TRUTH)
	{
		oneOpsCheck(NOT , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "clr") == STR_CMP_TRUTH)
	{
		oneOpsCheck(CLR , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "lea") == STR_CMP_TRUTH)
	{
		twoOpsCheck(LEA , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "inc") == STR_CMP_TRUTH)
	{
		oneOpsCheck(INC , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "dec") == STR_CMP_TRUTH)
	{
		oneOpsCheck(DEC , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "jmp") == STR_CMP_TRUTH)
	{
		oneOpsCheck(JMP , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "bne") == STR_CMP_TRUTH)
	{
		oneOpsCheck(BNE , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "red") == STR_CMP_TRUTH)
	{
		oneOpsCheck(RED , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "prn") == STR_CMP_TRUTH)
	{
		oneOpsCheck(PRN , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "jsr") == STR_CMP_TRUTH)
	{
		oneOpsCheck(JSR , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "rts") == STR_CMP_TRUTH)
	{
		zeroOpsCheck(RTS , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else if (strcmp(token , "stop") == STR_CMP_TRUTH)
	{
		zeroOpsCheck(STOP , line , copyLine , token , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else /* Word found doesn't represent any operation name, error. */
	{
		printf("\nError: correct operation name missing | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
}

/***************************************************************************/

void zeroOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* zeroOpsCheck is activated when there is a call to an operation with no operands, makes sure that there are no errors and encodes the opeartion as an instruction in the "Instructions" data structure. */
{
	token = strtok(NULL , " \t\n\f\r");
	if (token != NULL)
	{
		printf("\nError: illegal miscellaneous text after operation with 0 operands | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	else if (*numOfErrors == NO_ERRORS)
	{
		Instructions[*IC].instructionData.are = 0;
		Instructions[*IC].instructionData.opDes = 0;
		Instructions[*IC].instructionData.opSrc = 0;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = 0;
		Instructions[*IC].instructionData.param2 = 0;
		*IC = *IC + 1;
	}	
}

/***************************************************************************/

void oneOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* oneOpsCheck is activated when there is a call to an operation with a single operand and scans through the operand for further searching. The function also searches for potential errors and encodes the operation usuage. */
{
	char *op;
	int opType; /* What type of operand: 0,1,2 or 3? */
	int i = -1;
	int *reg = &i; /* Will be used if operand is of type 3 (register). */
	int srcOrDes = 1; /* Used for deciding source or destination operands. */
	
	token = strtok(NULL , " \t\n\f\r");
	op = token;
	if (op == NULL)
	{
		printf("\nError: missing operand after operation with 1 operand | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;		
	}
	token = strtok(NULL , " \t\n\f\r");
	if (token != NULL) /* Checking for errors. */
	{
		printf("\nError: illegal miscellaneous text after operand of operation with 1 operand or not allowed space between components of a type 2 operand | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;			
	}
	if (opCode == JMP || opCode == BNE || opCode == JSR) /* Potential operations with type 2 operands. */
	{
		/* Special check for type 2 operand. */
		if (checkType2(op , opCode , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions) == FALSE)
		{ 
			if (*numOfErrors == NO_ERRORS)
			{
				opType = typeOperand(op , reg); /* If not type 2 operand, we encode the operation data and pass the operand for 0,1 or 3 type encoding. */
				Instructions[*IC].instructionData.are = 0;
				Instructions[*IC].instructionData.opDes = opType;
				Instructions[*IC].instructionData.opSrc = 0;
				Instructions[*IC].instructionData.code = opCode;
				Instructions[*IC].instructionData.param1 = 0;
				Instructions[*IC].instructionData.param2 = 0;
				*IC = *IC + 1;
				codeOperand(op , srcOrDes , reg , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
			}
		}
	}
	else if (*numOfErrors == NO_ERRORS) /* Operations with 0,1 or 3 operand types only, encoding the operation and passing the operand for encoding. */
	{
		opType = typeOperand(op , reg);
		Instructions[*IC].instructionData.are = 0;
		Instructions[*IC].instructionData.opDes = opType;
		Instructions[*IC].instructionData.opSrc = 0;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = 0;
		Instructions[*IC].instructionData.param2 = 0;
		*IC = *IC + 1;
		codeOperand(op , srcOrDes , reg , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else /* Checking operands for potential errors after errors have been already found in the file. */
	{
		checkOperandForErrors(op , reg , IC , DC , numOfErrors , skipHead , fileLine);
	}		
}

/***************************************************************************/

int checkType2(char *op , int opCode , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* checkType2 checks an operand with type 2 potential and if it is, encodes it and its operation. */
{
	char *token; 
	char *op0;
	char *param1; /* Potential operand parameters. */
	char *param2;
	int opType = 2;
	int param1Type; /* Parameters of type 0,1 or 3. */
	int param2Type;
	int i1 = -1;
	int i2 = -1;
	int *reg1 = &i1;
	int *reg2 = &i2;
	int firstOrSecond = 1; /* First or second parameter in order? */
	
	if (consecutiveCommas(op) == TRUE) /* Making sure there's only one comma within the operands batch, else an error. */
	{
		printf("\nError: too many commas within an operation that should receive 1 at max, signifies potential miscellaneous text | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;	
	}
	token = strtok(op , " \t\n\f\r("); /* Seperating the opernad into its parameters and symbol. */
	op0 = token;
	token = strtok(NULL , " \t\n\f\r,");
	param1 = token;
	token = strtok(NULL , " \t\n\f\r)");
	param2 = token;
	if (op0 == NULL || param1 == NULL || param2 == NULL)
	{
		return FALSE;	
	} 	
	param1Type = typeOperand(param1 , reg1);
	param2Type = typeOperand(param2 , reg2);
	if (param1Type == -1 || param2Type == -1) /* Maybe number without '#' before it. */
	{
		printf("\nError: potential operand might be a number with no '#' before it | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;		
	}
	if (param1Type == REGISTER_OP && param2Type == REGISTER_OP && *numOfErrors == NO_ERRORS) /* If both parameters are registers we encode them in the same line. */
	{
		Instructions[*IC].instructionData.are = 0; /* Encoding operation. */
		Instructions[*IC].instructionData.opDes = opType;
		Instructions[*IC].instructionData.opSrc = 0;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = param1Type;
		Instructions[*IC].instructionData.param2 = param2Type;
		*IC = *IC + 1;
		addToSkipTable(op0 , IC , DC , numOfErrors , skipHead , fileLine); /* Adding potential symbol to the skip table (for second run). */
		Instructions[*IC].numData.a = 0;
		*IC = *IC + 1;
		Instructions[*IC].regs.are = 0;
		Instructions[*IC].regs.reg2 = *reg2; /* Encoding both registers together. */
		Instructions[*IC].regs.reg1 = *reg1;
		*IC = *IC + 1;	
	}
	else if (*numOfErrors == NO_ERRORS) /* Encoding parameters in different lines. */
	{
		Instructions[*IC].instructionData.are = 0; /* Encoding operation. */
		Instructions[*IC].instructionData.opDes = opType;
		Instructions[*IC].instructionData.opSrc = 0;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = param1Type;
		Instructions[*IC].instructionData.param2 = param2Type;
		*IC = *IC + 1;	
		addToSkipTable(op , IC , DC , numOfErrors , skipHead , fileLine); /* Potential symbol operand is added to skip table for second run encoding. */
		Instructions[*IC].numData.a = 0;
		*IC = *IC + 1;
		codeOperand(param1 , firstOrSecond , reg1 , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions); /* Encoding parameters just like regular 0,1,3 type operands. */
		firstOrSecond++;
		codeOperand(param2 , firstOrSecond , reg2 , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else /* Checking operands for potential errors after errors have been already found in the file. */
	{
		checkOperandForErrors(param1 , reg1 , IC , DC , numOfErrors , skipHead , fileLine);
		checkOperandForErrors(param2 , reg2 , IC , DC , numOfErrors , skipHead , fileLine);
	}

	return TRUE; 			
}

/***************************************************************************/

void twoOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* twoOpsCheck is activated when there is a call to an operation with two operands and scans through the operands for further searching. The function also searches for potential errors and encodes the operation usuage. */
{
	char *op1;
	char *op2;
	int op1Type; /* Types, registers and etc like before. */
	int op2Type;
	int i1 = -1;
	int i2 = -1;
	int *reg1 = &i1;
	int *reg2 = &i2;
	int srcOrDes = 1;

	if (consecutiveCommas(line) == TRUE) /* Making sure there's only one comma within the operation, else an error. */
	{
		printf("\nError: too many commas within a 2 operand operation, signifies miscellaneous potential text in the operation | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	if (missingCommas(line) == TRUE)
	{
		printf("\nError: no commas within a 2 operand operation, signifies for potential lack of operands and their seperation | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	token = strtok(NULL , " \t\n\f\r,"); /* Extracting operands from the line. */
	if (token == NULL)
	{
		printf("\nError: missing operand after operation with 2 operands | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;		
	}
	op1 = token;
	token = strtok(NULL , " \t\n\f\r,");
	if (token == NULL)
	{
		printf("\nError: missing operand after operation with 2 operands | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;		
	}
	op2 = token;
	token = strtok(NULL , " \t\n\f\r");
	if (token != NULL) /* Making sure there's no uneeded text. */
	{
		puts(token);
		printf("\nError: illegal miscellaneous text after two operands of operation with 2 operands | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;			
	}
	op1Type = typeOperand(op1 , reg1); /* Types */
	op2Type = typeOperand(op2 , reg2);
	if (op1Type == -1 || op2Type == -1) /* Maybe number without '#' before it. */ 
	{
		printf("\nError: potential operand might be a number with no '#' before it | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;		
	}
	if (op1Type == REGISTER_OP && op2Type == REGISTER_OP && *numOfErrors == NO_ERRORS) /* If both operands are registers encoding them in the same line. */
	{
		Instructions[*IC].instructionData.are = 0;
		Instructions[*IC].instructionData.opDes = op2Type;
		Instructions[*IC].instructionData.opSrc = op1Type;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = 0;
		Instructions[*IC].instructionData.param2 = 0;
		*IC = *IC + 1;
		Instructions[*IC].regs.are = 0;
		Instructions[*IC].regs.reg2 = *reg2;
		Instructions[*IC].regs.reg1 = *reg1;
		*IC = *IC + 1;	
	}
	else if (*numOfErrors == 0) /* Else encoding the operands separately. */
	{
		Instructions[*IC].instructionData.are = 0;
		Instructions[*IC].instructionData.opDes = op2Type;
		Instructions[*IC].instructionData.opSrc = op1Type;
		Instructions[*IC].instructionData.code = opCode;
		Instructions[*IC].instructionData.param1 = 0;
		Instructions[*IC].instructionData.param2 = 0;
		*IC = *IC + 1;
		codeOperand(op1 , srcOrDes , reg1 , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
		srcOrDes++;
		codeOperand(op2 , srcOrDes , reg2 , IC , DC , numOfErrors , symbolHead , skipHead , fileLine , Instructions);
	}
	else /* Checking operands for potential errors after errors have been already found in the file. */
	{
		checkOperandForErrors(op1 , reg1 , IC , DC , numOfErrors , skipHead , fileLine);
		checkOperandForErrors(op2 , reg2 , IC , DC , numOfErrors , skipHead , fileLine);
	}
}

/***************************************************************************/

int typeOperand(char *op , int *reg) /* Type operand decides whether an opernad is of type 0 (number) type 1 (symbol) or type 3 (register). */
{
	int i = 0;
	char savedRegisters[SAVED_REGISTERS][SAVED_REGISTERS] = {"r0" , "r1" , "r2" , "r3" , "r4" , "r5" , "r6" , "r7"};
	
	if (op[LINE_BEGINNING] == '#') /* Number must begin with '#'. */
	{
		return TYPE_ZERO;
	}
	else if (isdigit(op[LINE_BEGINNING]) != FALSE)
	{
		return -1;
	}
	while (i <= SAVED_REGISTERS) /* Else checking if the  */
	{
		if (strcmp(savedRegisters[i] , op) == STR_CMP_TRUTH)
		{
			*reg = i;
			return TYPE_THREE;
		}
		i++;
	}
	
	return TYPE_ONE; /* If no register or number, than operand might be a potential symbol. */	
}

/***************************************************************************/

void checkOperandForErrors(char *op , int *reg , int *IC , int *DC , int *numOfErrors , skipPtr *skipHead , int *fileLine) 
/* If errors have been detected in the file, this function still receives an operand an checks for various error potentials. */
{
	char token[LINE_SIZE];
	
	if (op[LINE_BEGINNING] == '#') /* If '#' is found there must be an inetegr after it, else an error. */
	{
		strncpy(token , op+1 , strlen(op)-1); 
		token[strlen(op)-1] = '\0';
		if (checkInteger(token) != 1)
		{
			printf("\nError: no integer next to: '#' | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
		}
	}
	else if (*reg != -1) /* Legal register, no errors. */
	{
		return;
	}
	else /* Else it must be some sort of symbol, at the end of first run it will be decided whether the symbol is legal. */
	{
		addToSkipTable(op , IC , DC , numOfErrors , skipHead , fileLine);	
	}	
}

/***************************************************************************/

void codeOperand(char *op , int srcOrDes , int *reg , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions)
/* codeOperand receives an operand and its type and encodes it in the Instructions data structure. */
{
	int num;
	char token[LINE_SIZE];

	if (op[LINE_BEGINNING] == '#')
	{
		strncpy(token , op+1 , strlen(op)-1); 
		token[strlen(op)-1] = '\0';
		if (checkInteger(token) == 1) /* If operand is a number we turn it into an integer. */
		{
			num = atoi(token);
			if (num < 0)
			{
				num = twosComplement((-1)*num , 1);
			}
		}
		else /* Else '#' was used without a number, error. */
		{
			printf("\nError: no integer next to: '#' | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return;	
		}
		if (*numOfErrors == NO_ERRORS)
		{
			Instructions[*IC].dataOfNum.are = 0;
			Instructions[*IC].dataOfNum.num = num;
			*IC = *IC + 1;
		}
		return;
	}
	else if (*reg != -1) /* Opernad is of register type so we code a register (as source or destination). */
	{
		if (srcOrDes == 1 && *numOfErrors == NO_ERRORS)
		{
			Instructions[*IC].regs.are = 0;
			Instructions[*IC].regs.reg2 = 0;
			Instructions[*IC].regs.reg1 = *reg;
			*IC = *IC + 1;
			return;		
		}
		else if (srcOrDes == 2 && *numOfErrors == NO_ERRORS)
		{
			Instructions[*IC].regs.are = 0;
			Instructions[*IC].regs.reg2 = *reg;
			Instructions[*IC].regs.reg1 = 0;
			*IC = *IC + 1;
			return;		
		}		
	}
	else if (*numOfErrors == NO_ERRORS)/* Else operand is a potential symbol so we add it to skip table. */
	{
		addToSkipTable(op , IC , DC , numOfErrors , skipHead , fileLine);
		Instructions[*IC].numData.a = 0;
		*IC = *IC + 1;	
		return;		
	}

	return;
}

/***************************************************************************/

int checkForSymbol(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine)
/* checkForSymbol receives a line and checks whether the first word in the line might be a symbol declaration. */
{
	char symbolKind[LINE_SIZE]; /* Will store symbol kind - data/string/code/etc. */
	char *symbolName = token;
	int length = strlen(token) - 1; 
	
	if (checkSavedOperationName(symbolName) == TRUE)
	{
		return FALSE;	
	} 
	if (symbolName[length] != ':') /* Symbol declaration must end with ':'. */
	{
		printf("\nError: missing ':' at the end of a potential symbol declaration | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;
	} 
	symbolName[length] = '\0'; /* Deleting ':' from the potential symbol. */
	if (isLegalSymbol(symbolName , length , numOfErrors , fileLine) == FALSE) /* Checking for errors within the symbol. */
	{
		return FALSE;
	}
	whatKindOfSymbol(copyLine , token , symbolKind); /* Checking for the kind of symbol. */
	if (strcmp(symbolKind , "empty") == STR_CMP_TRUTH) /* empty means no kind of symbol of any sort was found, error. */
	{
		printf("\nError: missing text after symbol declaration | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;
	}
	if (checkSavedSymbolName(symbolName) == TRUE) /* Making sure symbol does't use a saved name (register/operation/etc). */
	{
		printf("\nError: declared symbol is using a saved symbol name | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;
	}
	if (strcmp(symbolKind , ".entry") == STR_CMP_TRUTH || strcmp(symbolKind , ".extern") == STR_CMP_TRUTH) /* if there's an extern or entry command after the symbol declaration the symbol must be ignored. */
	{
		printf("\nWarning: attempt to declare a symbol before .entry or .extern command, will be ignored | line: %d\n\n" , *fileLine);
		return TRUE;	
	}
	if (alreadyInSymbolTable(symbolName , IC , DC , numOfErrors , symbolHead , fileLine) == 1) /* Checking whether the symbol hasn't been declared and added yet to the symbol table. */
	{
		printf("\nError: declared symbol has already been declared before | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;	
	}
	else /* Passing the symbol we found to be added to the symbol table. */
	{
		addToSymbolTable(symbolName , symbolKind , IC , DC , numOfErrors , symbolHead , fileLine);
	}
	
	return TRUE;
}

/***************************************************************************/

void whatKindOfSymbol(char *copyLine , char *token , char *symbolKind)
/* whatKindOfSymbol decides the kind of the declared symbol - data/string/code/etc by examining the rest of the line where symbol was declared. */
{
	token = strtok(NULL , " \t\n\f\r");
	if (token == NULL) /* Empty text or white chars, no kind. */
	{
		strcpy(symbolKind , "empty");
		symbolKind[5] = '\0';		
	}
	else if (strcmp(token , ".data") == STR_CMP_TRUTH)
	{
		strcpy(symbolKind , "data");
		symbolKind[4] = '\0';
	}
	else if (strcmp(token , ".string") == STR_CMP_TRUTH)
	{
		strcpy(symbolKind , "string");
		symbolKind[6] = '\0';
	}
	else if (strcmp(token , ".entry") == STR_CMP_TRUTH)
	{
		strcpy(symbolKind , "entry");
		symbolKind[5] = '\0';
	}
	else if(strcmp(token , ".extern") == STR_CMP_TRUTH)
	{
		strcpy(symbolKind , "extern");
		symbolKind[6] = '\0';	
	}
	else /* None of the above, kind is "code". */
	{
		strcpy(symbolKind , "code");
		symbolKind[4] = '\0';
	}	
}

/***************************************************************************/

void checkDataOrString(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Data)
/* checkDataOrString checks whether command is a .data or .string command. */
{
	if (strcmp(token , ".data") == STR_CMP_TRUTH)
	{
		/* .data command. */
		checkData(line , copyLine , token , IC , DC , numOfErrors , fileLine , Data);
	}
	else
	{
		/* .string command. */
		checkString(line , copyLine , token , IC , DC , numOfErrors , fileLine , Data);	
	}
}

/***************************************************************************/

void checkData(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data)
/* checkData creates imporatnt variables, searches for numbers after ".data" command and encodes them. */
{
	char *str = (char *)malloc(sizeof(char)*LINE_SIZE); /* Will store the string with the numbers. */
	int i;
	int z = 0;
	int j = 0;
	int *num_count = &j; /* How many numbers we find. */
	int *arr; /* Will store the numebrs we find. */

	token = strtok(NULL , " \t\n\f\r");
	if (token == NULL)
	{
		printf("\nError: no numbers after .data command | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	if (isdigit(token[LINE_BEGINNING]) == FALSE && token[LINE_BEGINNING] != '+' && token[LINE_BEGINNING] != '-') 
	/* Making sure text between .data and numbers begins with a number digit, or +- signs. */
	{
		printf("\nError: miscellaneous text between .data and first digit of first number | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	i = reachFirstDigit(line);
	strncpy(str , line+i , strlen(line)); /* Let line store only the numbers seperated by the commas. */
	arr = checkDataCorrectness(str , num_count); /* Store the line ints within arr. */
	if (arr == NULL) /* Something within the numbers provided was incorrect, error. */
	{
		dataErrorCases(num_count , fileLine); /* Returning the correct error possibility according to num_count's value. */
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	while (z < *num_count) /* Encoding each numebr found by the functions in arr. */
	{
		codeNumber(*(arr+z) , IC , DC , numOfErrors , fileLine , Data);
		z++;
	}
	free(str);	
}

/***************************************************************************/

void codeNumber(int num , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data) /* codeNumber encodes a number found in a ".data" command. */
{
	if (num < 0)
	{
		num = twosComplement((-1)*num , 2); /* If number is negative we use twosComplement function for its representation. */
	}
	if (*numOfErrors == NO_ERRORS)
	{
		Data[*DC].numData.a = num;
		*DC = *DC + 1;	
	}
}

/***************************************************************************/

void checkString(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data)
/* checkString creates imporatnt variables, searches for a string after ".string" command and encodes its characters. */
{
	int i;
	char *token2;

	token = strtok(NULL , " \t\n\f\r");
	if (token == NULL) /* Checking for correct syntax, missing or miscellaneous text and other potential errors. */
	{
		printf("\nError: no string after .string command | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	token2 = strtok(line , " \t\n\f\r");
	if (strcmp(token , ".string") == STR_CMP_TRUTH)
	{
		token2 = strtok(NULL , " \t\n\f\r");
		token2 = strtok(NULL , " \t\n\f\r");
	}
	else
	{
		token2 = strtok(NULL , " \t\n\f\r");
		token2 = strtok(NULL , " \t\n\f\r");
		token2 = strtok(NULL , " \t\n\f\r");
	}
	if (token2 != NULL)
	{
		printf("\nError: illegal miscellaneous text after .string command and the string used. | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;		
	}
	if (token[LINE_BEGINNING] != '"')
	{
		printf("\nError: string after .string command doesn't start with \" | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	if (token[strlen(token)-1] != '"')
	{
		printf("\nError: string after .string command doesn't end with \" | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;	
	}
	for (i = 1 ; i < strlen(token) - 1 ; i++) /* Encoding each character of the string. */
	{
		codeLetter(token[i] , IC , DC , numOfErrors , fileLine , Data);	
	}
	codeLetter('\0' , IC , DC , numOfErrors , fileLine , Data); /* Encoding null termiantor in the end. */
}

/***************************************************************************/

void codeLetter(char letter , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data) /* codeLetter encodes chars found in ".string" command. */
{
	if (*numOfErrors == NO_ERRORS)
	{
		Data[*DC].string.b = letter;
		*DC = *DC + 1;
	}
}

/***************************************************************************/

int alreadyInSymbolTable(char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , int *fileLine)
/* alreadyInSymbolTable receives a potential symbol name and checks whether there is a symbol with the same name in the symbol table already. */
{
	symbolPtr p1 = *symbolHead;
	
	if (p1 == NULL)
	{
		return FALSE;
	}
	while (p1 != NULL) /* Running and comparing through the symbol list. */
	{
		if (strcmp((p1->symbolTable_entry).symbolName , token) == STR_CMP_TRUTH)
		{
			return TRUE;
		}
		p1 = p1->symbolTable_next;
	}
	
	return FALSE;
}

/***************************************************************************/

int alreadyInExTable(char *token , int *IC , int *DC , int *numOfErrors , ExPtr *ExHead , int *fileLine)
/* alreadyInExlTable receives a potential symbol name and checks whether there is a symbol with the same name in the external table already. */
{
	ExPtr p1 = *ExHead;
	
	if (p1 == NULL)
	{
		return FALSE;
	}
	while (p1 != NULL) /* Running and comparing through the external list. */
	{
		if (strcmp((p1->ExTable_entry).symbolEx , token) == STR_CMP_TRUTH)
		{
			return TRUE;
		}
		p1 = p1->ExTable_next;
	}
	
	return FALSE;
}

/***************************************************************************/

int alreadyInEnTable(char *token , int *IC , int *DC , int *numOfErrors , EnPtr *EnHead , int *fileLine)
/* alreadyInEnlTable receives a potential symbol name and checks whether there is a symbol with the same name in the enternal table already. */
{
	EnPtr p1 = *EnHead;
	
	if (p1 == NULL)
	{
		return FALSE;
	}
	while (p1 != NULL) /* Running and comparing through the enternal list. */
	{
		if (strcmp((p1->EnTable_entry).symbolEn , token) == STR_CMP_TRUTH)
		{
			return TRUE;
		}
		p1 = p1->EnTable_next;
	}
	
	return FALSE;
}

/***************************************************************************/

void addToSymbolTable(char *token , char *symbolKind , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , int *fileLine)
/* addToSymbolTable receives a symbol name and its kind and adds the symbol with its kind to the symbol list. */
{
	symbolPtr t = (symbolPtr)malloc(sizeof(symbolItem));
	symbolPtr p1 = *symbolHead;
	symbolPtr p2;

	if (t == NULL)
	{
		printf("\nMemory allocation and thus addition of symbol to symbol list failure | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	strcpy((t->symbolTable_entry).symbolName , token);
	strcpy((t->symbolTable_entry).symbolKind , symbolKind); /* New node with the symbol data. */
	if (strcmp(symbolKind , "string") == STR_CMP_TRUTH || strcmp(symbolKind , "data") == STR_CMP_TRUTH)
	{
		(t->symbolTable_entry).symbolAdress = *DC;	
	} 
	else
	{
		(t->symbolTable_entry).symbolAdress = *IC;	
	}
	while (p1 != NULL) /* Adding the new node to the existing list. */
	{
		p2 = p1;
		p1 = p1->symbolTable_next;
	}
	if (p1 == *symbolHead)	
	{
		*symbolHead = t;
		t->symbolTable_next = p1;
	}
	else
	{
		p2->symbolTable_next = t;
		t->symbolTable_next = p1;
	} 	
}

/***************************************************************************/

void addToSkipTable(char *token , int *IC , int *DC , int *numOfErrors , skipPtr *skipHead , int *fileLine)
/* addToSkipTable receives a potential symbol name and adds the name to the skip table. */
{
	skipPtr t = (skipPtr)malloc(sizeof(skipItem));
	skipPtr p1 = *skipHead;
	skipPtr p2;

	if (t == NULL)
	{
		printf("\nMemory allocation and thus addition of symbol to skip list failure | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	strcpy((t->skipTable_entry).skipName , token);
	(t->skipTable_entry).skipAdress = *IC;
	(t->skipTable_entry).skipFileLine = *fileLine; /* New node with the symbol data. */
	while (p1 != NULL) /* Adding the new node to the existing list. */
	{
		p2 = p1;
		p1 = p1->skipTable_next;
	}
	if (p1 == *skipHead)	
	{
		*skipHead = t;
		t->skipTable_next = p1;
	}
	else
	{
		p2->skipTable_next = t;
		t->skipTable_next = p1;
	} 	
}

/***************************************************************************/

void addToExTable(char *token , int *IC , int *DC , int *numOfErrors , ExPtr *ExHead , int *fileLine)
/* addToExTable receives a potential symbol name and adds the name to the external table. */
{
	ExPtr t = (ExPtr)malloc(sizeof(ExItem));
	ExPtr p1 = *ExHead;
	ExPtr p2;

	if (t == NULL)
	{
		printf("\nMemory allocation and thus addition of symbol to external list failure | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	strcpy((t->ExTable_entry).symbolEx , token); /* New node with the symbol data. */
	while (p1 != NULL) /* Adding the new node to the existing list. */
	{
		p2 = p1;
		p1 = p1->ExTable_next;
	}
	if (p1 == *ExHead)	
	{
		*ExHead = t;
		t->ExTable_next = p1;
	}
	else
	{
		p2->ExTable_next = t;
		t->ExTable_next = p1;
	} 		
}

/***************************************************************************/

void addToEnTable(char *token , int *IC , int *DC , int *numOfErrors , EnPtr *EnHead , int *fileLine)
/* addToEnTable receives a potential symbol name and adds the name to the enternal table. */
{
	EnPtr t = (EnPtr)malloc(sizeof(EnItem));
	EnPtr p1 = *EnHead;
	EnPtr p2;

	if (t == NULL)
	{
		printf("\nMemory allocation and thus addition of symbol to enternal list failure | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return;
	}
	strcpy((t->EnTable_entry).symbolEn , token); /* New node with the symbol data. */
	while (p1 != NULL) /* Adding the new node to the existing list. */
	{
		p2 = p1;
		p1 = p1->EnTable_next;
	}
	if (p1 == *EnHead)	
	{
		*EnHead = t;
		t->EnTable_next = p1;
	}
	else
	{
		p2->EnTable_next = t;
		t->EnTable_next = p1;
	} 		
}

/***************************************************************************/

void checkEntryOrExtern(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine) /* Checks a ".entry" or ".extern" command, checks the potential symbol after the command and adds it to the correct table. */
{
	char symbolKind[LINE_SIZE];
	char token2[LINE_SIZE];
	
	if (checkForProblems1(line , copyLine , token , IC , DC , numOfErrors , fileLine , token2 , symbolKind) == FALSE) /* Checking for various problems within the command such as symbol legality. */
	{
		return;
	}
	if (strcmp(symbolKind , "extern") == STR_CMP_TRUTH) /* Checking whether the symbol has already been defined as an extern/entry. */
	{
		if (alreadyInExTable(token2 , IC , DC , numOfErrors , ExHead , fileLine) == 1)
		{
			printf("\nError: symbol %s has been defined twice as en extern | line: %d\n\n" , token2 , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return;		
		}
		else if (alreadyInEnTable(token2 , IC , DC , numOfErrors , EnHead , fileLine) == 1)
		{
			printf("\nError: symbol %s has been defined both as an extern and as an entry | line: %d\n\n" , token2 , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return;
		}
		else
		{
			addToExTable(token2 , IC , DC , numOfErrors , ExHead , fileLine); /* Adding a symbol to the external table. */	
		}
	}
	else if (strcmp(symbolKind , "entry") == STR_CMP_TRUTH)
	{
		if (alreadyInEnTable(token2 , IC , DC , numOfErrors , EnHead , fileLine) == 1)
		{
			printf("\nError: symbol %s has been defined twice as en entry | line: %d\n\n" , token2 , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return;	
		}
		else if (alreadyInExTable(token2 , IC , DC , numOfErrors , ExHead , fileLine) == 1)
		{
			printf("\nError: symbol %s has been defined both as an extern and as an entry | line: %d\n\n" , token2 , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return;
		}
		else
		{
			addToEnTable(token2 , IC , DC , numOfErrors , EnHead , fileLine); /* Adding a symbol to the enternal table. */
		}
	}
}	

/***************************************************************************/

int checkForProblems1(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , char *token2 , char *symbolKind)
/* Checks for different problems within a line that conatians a ".entry" or ".extern" command. */
{
	int length;

	if (strcmp(token , ".entry") == STR_CMP_TRUTH) /* entry or external. */
	{
		strcpy(symbolKind , "entry");
		symbolKind[5] = '\0';
	}
	else 
	{
		strcpy(symbolKind , "extern");
		symbolKind[6] = '\0';
	}
	token = strtok(NULL ," \t\n\f\r");
	if (token == NULL) /* No symbol after the comand.  */
	{
		printf("\nError: missing symbol after .entry or .extern command | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;	
	}
	length = strlen(token);
	if (isLegalSymbol(token , length , numOfErrors , fileLine) == FALSE) /* Legal symbol syntax. */
	{
		return FALSE;
	}
	if (checkSavedSymbolName(token) == TRUE) /* If symbol is using a saved name. */
	{
		printf("\nError: symbol after .extern or .entry command is using a saved name | line: %d\n\n" , *fileLine);
		*numOfErrors = *numOfErrors + 1;
		return FALSE;
	}
	strcpy(token2 , token);
	token = strtok(NULL ," \t\n\f\r");
	if (token != NULL) /* Illegal text after command. */
	{	
		if (strcmp(symbolKind , "entry") == STR_CMP_TRUTH)
		{
			printf("\nError: illegal miscellaneous text after symbol in .entry command | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return FALSE;	
		}
		else if (strcmp(symbolKind , "extern") == STR_CMP_TRUTH)
		{
			printf("\nError: illegal miscellaneous text after symbol in .extern command | line: %d\n\n" , *fileLine);
			*numOfErrors = *numOfErrors + 1;
			return FALSE;
		}
	}

	return TRUE;		
} 

/***************************************************************************/

void checkLegitExEnTable(int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine)
/* By the end of the first run checking that the enternal and external tables received have no mistakes. */
{
	ExPtr p1 = *ExHead;
	EnPtr p2 = *EnHead;
	
	while (p1 != NULL)
	{
		if (alreadyInSymbolTable((p1->ExTable_entry).symbolEx , IC , DC , numOfErrors , symbolHead , fileLine) == 1)
		/* Checking that each symbol in the external table hasn't been declared in the file and is not in the symbol table. */
		{
			printf("\nError: symbol %s has both been declared in the file and used as extern.\n\n" , (p1->ExTable_entry).symbolEx);
			*numOfErrors = *numOfErrors + 1;
			return;
		}
		p1 = p1->ExTable_next;
	}
	while (p2 != NULL)
	{
		if (alreadyInSymbolTable((p2->EnTable_entry).symbolEn , IC , DC , numOfErrors , symbolHead , fileLine) == FALSE)
		/* Checking that each symbol in the enetrnal table has been declared in the file and is in the symbol table. */
		{
			printf("\nError: symbol %s has been used as entry but wasn't declared.\n\n" , (p2->EnTable_entry).symbolEn);
			*numOfErrors = *numOfErrors + 1;
			return;
		}
		p2 = p2->EnTable_next;
	}
}

/***************************************************************************/

void checkSkipTableErrors(symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , skipPtr *skipHead , int *numOfErrors , int *fileLine) 
/* A function that runs through the skip list if second run is not initiated and seaches for errors within the skipped symbols. */
{
	skipPtr p1 = *skipHead;
	char symbol[LINE_SIZE];
	char dummy1[LINE_SIZE];
	int dummy2 = 0;
	int dum3;
	int *dummy3 = &dum3;
	
	while (p1 != NULL)
	{
		strcpy(symbol , (p1->skipTable_entry).skipName);
		*fileLine = (p1->skipTable_entry).skipFileLine;
		checkForProblems2(symbol , numOfErrors , fileLine); 
		/* Running through the skip table and passing each symbol and where it was found to searching for problems. */
		if (lookForSymbol(symbol , dummy1 , dummy2 , dummy3 , symbolHead , ExHead , EnHead) == FALSE) 
		/* lookForSymbol belongs to the second run checks whether the skipped symbol used has been declared or used as an extern. */
		{
			printf("\nError: in the second run, potential symbol %s we skipped in the first run that belongs to an instruction in line %d wasn't declared or isn't an extern.\n\n" , symbol , *fileLine);	
		}
		p1 = p1->skipTable_next;
	}
}
