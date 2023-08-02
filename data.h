/* Assembly project by Omer Komissarchik - 215314725 */
/* Contains: */
/* assembler.c - Receives files and pass to pre-assembly and encoding. */
/* pre_assembler.c - Receives a pre-assembled file, assembles it and returns the pre-assembly. */
/* first_run.c - First run of encoding and saving symbols. */
/* second_run.c - Second run of final encoding. */
/* output.c - Creation of various output files according to the final assembly we made. */
/* utility_funcs.c - Utility functions to assist with calulations and checks within the main assembly functions. */

#include <stdio.h> /* Standard libraries. */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_SIZE 100 /* Finals we use. */
#define MEMORY_BEGIN 100
#define MEMORY_SIZE 256
#define MOV 0
#define CMP 1
#define ADD 2
#define SUB 3
#define NOT 4
#define CLR 5
#define LEA 6
#define INC 7
#define DEC 8
#define JMP 9
#define BNE 10
#define RED 11
#define PRN 12
#define JSR 13
#define RTS 14
#define STOP 15
#define REGISTER_OP 3
#define SYMBOL_SIZE 30
#define BYTE 8
#define CELL_SIZE 14
#define SAVED_NAMES 30
#define SAVED_NAMES_TWO 20
#define SAVED_REGISTERS 8
#define TYPE_ZERO 0
#define TYPE_ONE 1
#define TYPE_THREE 3
#define TRUE 1
#define FALSE 0
#define STR_CMP_TRUTH 0
#define LINE_BEGINNING 0
#define FLAG_OFF 0
#define FLAG_ON 1
#define NO_ERRORS 0

/* Differnet data structures we use - arrays/lists/etc. */

/* Macro list - holds macro's name and all its commands. */
typedef struct {char mcrID[100]; char *mcrInside;} mcr;
typedef struct node * ptr;
typedef struct node {mcr macro_entry; ptr macro_next;} mcrItem;

/* Symbol list - holds declared symbol's name, kind and declaration adress. */
typedef struct {char symbolName[100]; char symbolKind[100]; int symbolAdress;} symbolTable;
typedef struct nodeSymbol * symbolPtr;
typedef struct nodeSymbol {symbolTable symbolTable_entry; symbolPtr symbolTable_next;} symbolItem;

/* External list - holds external symbol's name. */
typedef struct {char symbolEx[100];} ExTable;
typedef struct nodeEx * ExPtr;
typedef struct nodeEx {ExTable ExTable_entry; ExPtr ExTable_next;} ExItem;

/* Enternal list - holds enternal symbol's name and where they were declared in the file. */
typedef struct {char symbolEn[100]; int whereInSymbolTable;} EnTable;
typedef struct nodeEn * EnPtr;
typedef struct nodeEn {EnTable EnTable_entry; EnPtr EnTable_next;} EnItem;

/* Advanced external list - holds external symbol's name and also adresses where they were used in the file. */
typedef struct {char advancedSymbolEx[100]; int whereUsedInFile;} advancedExTable;
typedef struct advancedNodeEx * advancedExPtr;
typedef struct advancedNodeEx {advancedExTable advancedExTable_entry; advancedExPtr advancedExTable_next;} advancedExItem;

/* Skip list - holds potentialy ignored in first run symbol's name, usage adress and line of file from where they were skipped. */
typedef struct {char skipName[100]; int skipAdress; int skipFileLine;} skipTable;
typedef struct nodeSkip * skipPtr;
typedef struct nodeSkip {skipTable skipTable_entry; skipPtr skipTable_next;} skipItem;

/* Various bit-fields for storing different kinds of data using 14 bits. */
typedef struct {unsigned int a: 14;} numCode; /* Number that occupies 14 bits for ".data" commands. */
typedef struct {unsigned int b: 14;} stringCode; /* Chars of a string that occupy 14 bits for ".string" commands. */
typedef struct {unsigned int are: 2; unsigned int opDes: 2; unsigned int opSrc: 2; unsigned int code: 4; unsigned int param2: 2; unsigned int param1: 2;} instructionCode; /* Representation of operation code: ARE, destination and source, opcode , parameters. */ 
typedef struct {unsigned int are: 2; unsigned int num: 12;} typeZero; /* Number operand occupies 12 bits, 2 for ARE. */
typedef struct {unsigned int are: 2; unsigned int reg2: 6; unsigned int reg1: 6;} twoRegs; /* Two registers in the same line. */
typedef struct {unsigned int are: 2; unsigned int declareAdress: 12;} symbol; /* Symbol, includes ARE and declaration adress. */

/* This is the type of the assembly array, the union allows us to represent info in whichever bit-field we choose. */
typedef union {numCode numData; stringCode string; instructionCode instructionData; typeZero dataOfNum; twoRegs regs; symbol symbolUsed;} assemblyType; 

/* assembler.c functions */
int main(int argc , char *argv[]); void passToPreAssembly(int argc , char *argv[]); void run(char *argv , int count);

/* pre_assembler.c functions */
FILE* preAssembly(FILE *pre , char *nameOfPast); void searchLine(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line); void maybeNewMcr(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token); void addToTable(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token); int maybeReplace(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token); char* copyToMcr(FILE *pre , FILE *past , ptr *Head , int *mcrFlag , char *line , char *copyLine , char *token , char *copyLine2);

/* first_run.c functions */ 
int firstRun(char *nameOfFile); void run1(FILE *f1 , char *line , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine , assemblyType *Instructions , assemblyType *Data); void checkOperationCode(char *line , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int which , int *fileLine , assemblyType *Instructions); void checkOperationName(char *line  , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); void zeroOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); void oneOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); int checkType2(char *op , int opCode , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); void twoOpsCheck(int opCode , char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); int typeOperand(char *op , int *reg); void codeOperand(char *op , int srcOrDes , int *reg , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Instructions); int checkForSymbol(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine); void checkDataOrString(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , int *fileLine , assemblyType *Data); void checkData(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data); void codeNumber(int num , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data); void checkString(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data); void codeLetter(char letter , int *IC , int *DC , int *numOfErrors , int *fileLine , assemblyType *Data); void whatKindOfSymbol(char *copyLine , char *token , char *symbolKind); int alreadyInSymbolTable(char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , int *fileLine); void addToSymbolTable(char *token , char *symbolKind , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , int *fileLine); void addToSkipTable(char *token , int *IC , int *DC , int *numOfErrors , skipPtr *skipHead , int *fileLine); void addToExTable(char *token , int *IC , int *DC , int *numOfErrors , ExPtr *ExHead , int *fileLine); void addToEnTable(char *token , int *IC , int *DC , int *numOfErrors , EnPtr *EnHead , int *fileLine); void checkEntryOrExtern(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine); void checkLegitExEnTable(int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine); int checkForProblems1(char *line , char *copyLine , char *token , int *IC , int *DC , int *numOfErrors , int *fileLine , char *token2 , char *symbolKind); int alreadyInExTable(char *token , int *IC , int *DC , int *numOfErrors , ExPtr *ExHead , int *fileLine); int alreadyInEnTable(char *token , int *IC , int *DC , int *numOfErrors , EnPtr *EnHead , int *fileLine); void checkOperandForErrors(char *op , int *reg , int *IC , int *DC , int *numOfErrors , skipPtr *skipHead , int *fileLine); void checkSkipTableErrors(symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , skipPtr *skipHead , int *numOfErrors , int *fileLine);

/* second_run.c functions */
int secondRun(int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , skipPtr *skipHead , ExPtr *ExHead , EnPtr *EnHead , int *fileLine , assemblyType *Instructions , assemblyType *Data , char *nameOfFile); void run2(char *symbol , int adress , int *IC , int *DC , int *numOfErrors , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , advancedExPtr *advancedExHead , int *fileLine , assemblyType *Instructions , assemblyType *Data); int lookForSymbol(char *symbol , char *whatSymbol , int adress , int *whereDeclared , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead); void addToAdvancedExTable(int adress , char *symbol , advancedExPtr *advancedExHead , int *numOfErrors); int checkForProblems2(char *symbol , int *numOfErrors , int *fileLine);

/* output.c functions */
int createOutputs(int *IC , int *DC , symbolPtr *symbolHead , ExPtr *ExHead , EnPtr *EnHead , advancedExPtr *advancedExHead , assemblyType *assembly , char *nameOfFile); int createObject(int *IC , int *DC , assemblyType *assembly , char *nameOfFile); int createExternal(symbolPtr *symbolHead , advancedExPtr *advancedExHead , char *nameOfFile); int createEnternal(symbolPtr *symbolHead , EnPtr *EnHead , char *nameOfFile , int *IC); void insertAmount(FILE *object , int *IC , int *DC); void insertBinary(FILE *object , assemblyType *assembly , int *adress); int lookForSymbolAdress(char *symbolEn , symbolPtr *symbolHead , int *IC); void insertEntry(FILE *enternal , char *symbolEn , int num);  void insertExtern(FILE *external , char *symbolEx , int num); 

/* utility_funcs.c functions */
char* createAS(char* argv); char* createAM(char* argv); void checkLegalMacroName(char *token , FILE *past); int twosComplement(int num , int which); int checkInteger(char *token); int isLegalSymbol(char *token , int length , int *numOfErrors , int *fileLine); int* checkDataCorrectness(char *str , int *num_count); int checkRestOfLine(char *str , int i); int reachFirstDigit(char *line); int isLetter(char c); int checkSavedSymbolName(char *token); char* convertToOb(char *nameOfFile); char* convertToEnt(char *nameOfFile); char* convertToExt(char *nameOfFile); void toBinary(int num , char *binary); int isEmpty(FILE *file); int isLegalSymbol2(char *token , int length , int *numOfErrors , int *fileLine); void dataErrorCases(int *x , int *fileLine); int checkSavedOperationName(char *token); int missingCommas(char *line); int consecutiveCommas(char *line);
