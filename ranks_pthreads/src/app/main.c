#include <stdio.h>
#include <stdlib.h>
#include <stopper.h>
#include <pthread.h>

void readFile(const char* inputPath, int array[], int N);
void writeToFile(const char* outputPath, const char* program, int threads, double time, int array[], int N);
void putInPlace(int res[], int values[], int i, int N);
void* putInPlaceThread(void* params);

typedef struct threadParameter {
    int* res;
    int* array;
    int begin;
    int end;
    int N;
} threadParameter;

int main(int argc, char** argv)
{
	if(argc < 4)
	{
		printf("Argument(s) missing!\nUse program as: %s <N> <threads> <inputPath> <outputPath>", argv[0]);
		exit(0);
	}

	int N = atoi(argv[1]);
	int threads = atoi(argv[2]) >= 1 ? (atoi(argv[2])) : 1;
	const char* inputPath = argv[3];
	const char* outputPath = argv[4];
	
	stopperOMP st;
	startSOMP(&st);
  
    printf("Run  \t  ||  \t Ranking on %3d thread(s) \t  ||  \t", threads);

    int array[N], res[N];

	readFile(inputPath, array, N);

	pthread_t* threadIds = (pthread_t*)malloc(sizeof(pthread_t) * threads);
	threadParameter* threadParams = (threadParameter*)malloc(sizeof(threadParameter) * threads);

	for(int i = 0; i < threads; i++)
	{
		threadParams[i].res = res;
		threadParams[i].array = array;
		threadParams[i].begin = 0 + ((N * i) / threads);
		threadParams[i].end = 0 + ((N * (i + 1)) / threads);
		threadParams[i].N = N;

		pthread_create(threadIds + i, NULL, putInPlaceThread, threadParams + i);
	}

	for (int i = 0; i < threads; ++i ) 
	{
		pthread_join(threadIds[i], NULL);
	}

	stopSOMP(&st);
	
	printf("   took %5lf second(s)\n", st.end - st.begin);
	
	writeToFile(outputPath, argv[0], threads, st.end - st.begin, res, N);

	free(threadIds);
	free(threadParams);

	pthread_exit(NULL);
	
	return 0;
}

void writeToFile(const char* outputPath, const char* program, int threads, double time, int array[], int N)
{
	FILE * filePointer;
	if(filePointer = fopen(outputPath, "w"))
	{
		fprintf(filePointer, "--PROGRAM %s --\n%s%2d%s%5lf%s\n--OUTPUT--\n", program, "Running the program on ", threads, " threads took ", time, " seconds per thread");
		for(int i = 0; i < N; i++)
		{
			if(i % 5 == 0) { fprintf(filePointer, "\n||\t%5d\t", array[i]); }
			else { fprintf(filePointer, "||\t%5d\t", array[i]); }
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", outputPath);
	}
}

void readFile(const char* inputPath, int array[], int N)
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
	}
}

void putInPlace(int final[], int values[], int i, int N)
{
  int testval = values[i];
  int rank = 0;
  int count = 0;
  for(int j = 0; j < N; j++) {
    if(testval > values[j])
    {
        rank++;
    }
    if(testval == values[j] && j < i)
    {
        count++;
    }
  }

  rank = rank + count;
  final[rank] = testval;   
}

void* putInPlaceThread(void* params)
{
	threadParameter* parameters = (threadParameter*)params;

	for(int i = parameters -> begin; i < parameters -> end; i++) 
	{
		putInPlace(parameters -> res, parameters -> array, i, parameters -> N);
	}

	pthread_exit(NULL);
}
