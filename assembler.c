#include "data.h"

int main(int argc , char *argv[]) /* main - receives the input file through command line and passes them to pre-assembly and next functions. */
{
	passToPreAssembly(argc , argv);
	printf("\nEnd of assembly, good job.\n\n");
	return TRUE;	
}

/***************************************************************************/

void passToPreAssembly(int argc , char *argv[]) /* Checks whether there is no file input, if there is it sends the input to run function. */
{
	int i;

	if (argc == 1)
	{
		printf("\nNo files to assemble.\n\n");	
	}
	for (i = 1 ; i < argc ; i++)
	{
		run(argv[i] , i);	
	}	
}

/***************************************************************************/

void run(char *argv , int count) /* run runs a loop for all the input files, "fixes" their names and passes them to pre-assembly and first/second encoding runs. */
{	
	FILE *pre;
	FILE *past;
	char* name1;
	char* name2;
	
	printf("\n%d.)\n" , count);
	name1 = createAS(argv);
	name2 = createAM(argv);
	if (name1 == NULL || name2 == NULL)
	{
		printf("\nSkipping %s file due to memory allocation failure, moving on.\n\n" , argv);
		printf("------------------------------------------------------\n");
		return;	
	}
	pre = fopen(name1 , "r+");
	if (pre == NULL)
	{
		printf("\nOpening file: %s failed, might not exsist, moving on.\n\n" , name1);
		printf("------------------------------------------------------\n");
		return;
	}
	if (isEmpty(pre) == 1)
	{
		printf("\nFile is empty.\n\n");
		return;	
	}
	else
	{
		printf("\nFile: %s accessed, attempting pre_assembly.\n\n" , name1);
		past = preAssembly(pre , name2);	
	}	
	if (past == NULL)
	{
		printf("\nPre-assembly of file: %s failed, file: %s wasn't cretaed, moving on.\n\n" , name1 , name2);
		printf("------------------------------------------------------\n");
		return;	
	}
	else
	{
		printf("\nPre-assembly of file: %s was successful, file: %s was created.\n\n" , name1 , name2);
		firstRun(name2);		
	}
	free(name1);
	free(name2);	
}
