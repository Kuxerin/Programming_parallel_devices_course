#include <stdio.h>
#include <stdlib.h>
#include <stopper.h>
#include <omp.h>

void readFile(const char* inputPath, int array[]);
void writeToFile(const char* outputPath, const char* program, int threads, double time, int array[]);
int putInPlace(int values[], int src);

int N;

int main(int argc, char** argv)
{
	if(argc < 4)
	{
		printf("Argument(s) missing!\nUse program as: %s <N> <threads> <inputPath> <outputPath>", argv[0]);
		exit(0);
	}
	
	N = atoi(argv[1]);
	int threads = atoi(argv[2]);
	const char* inputPath = argv[3];
	const char* outputPath = argv[4];

    stopperOMP st;
    startSOMP(&st);
	
	printf("Run  \t  ||  \t Ranking on %3d thread(s) \t  ||  \t", threads);
	
	int array[N], res[N];
  
	readFile(inputPath, array);

	#pragma omp parallel for num_threads(threads)
	for(int i = 0; i < N; i++)
	{
		res[putInPlace(array, i)] = array[i];
	}

    stopSOMP(&st);	
	
	printf("   took %5lf second(s)\n", st.end - st.begin);

	writeToFile(outputPath, argv[0], threads, st.end - st.begin, res);
	
	return 0;
}

void writeToFile(const char* outputPath, const char* program, int threads, double time, int array[])
{
	FILE * filePointer;
	if(filePointer = fopen(outputPath, "w"))
	{
		fprintf(filePointer, "--PROGRAM %s --\n%s%2d%s%5lf%s\n--OUTPUT--\n", program, "Running the program on ", threads, " threads took ", time, " seconds per thread");
		for(int i = 0; i < N; i++)
		{
			if(i % 5 == 0) 
			{	
				fprintf(filePointer, "\n||\t%5d\t", array[i]); 
			}
			else 
			{ 
				fprintf(filePointer, "||\t%5d\t", array[i]); 
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", outputPath);
		exit(0);
	}
}

void readFile(const char* inputPath, int array[])
{
	FILE * filePointer;
	if(filePointer = fopen(inputPath, "r"))
	{
		for(int i = 0; i < N; i++)
		{
			fscanf(filePointer, "%d", &array[i]);
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", inputPath);
		exit(0);
	}
}
	
int putInPlace(int values[], int src)
{
    int testval = values[src];
    int rank = 1;
    int count = 0;
    
    for(int i = 0; i < N; i++)
    {
        if(testval > values[i])
		{
			rank++;
		}
        if(testval == values[i] && (i < src))
		{
            count++;
		}
    }
    rank += count;

    return rank - 1;
}
