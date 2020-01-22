#include <stdio.h>
#include <stdlib.h>
#include <stopper.h>
#include <stdbool.h>
#include <pthread.h>

void readFiles(const char* inputPath1, const char* inputPath2, int** array1, int** array2);
void writeToFile(const char* outputPath, const char* program, int threads, double time, int** array);
bool verifyDimensions(const char* inputPath1, const char* inputPath2);
void dotProduct(int i, int j, int** matrix1, int** matrix2, int** res);
void* dotProductThread(void* params);
int** allocateMemory(int size1, int size2);
bool freeMemory(int** array, int size1);

int M = 0, N = 0, K = 0, L = 0;

typedef struct threadParameter {
    int** matrix1;
    int** matrix2;
    int** res;
    int begin;
    int end;
} threadParameter;

int main(int argc, char** argv) 
{
	if(argc < 4)
	{
		printf("Argument(s) missing!\nUse program as: %s <threads> <inputPath1> <inputPath2> <outputPath>", argv[0]);
		exit(0);
	}
	
	int threads = atoi(argv[1]) >= 1 ? (atoi(argv[1])) : 1;
	const char* inputPath1 = argv[2];
	const char* inputPath2 = argv[3];
	const char* outputPath = argv[4];

	stopperOMP st;
	startSOMP(&st);

	if(verifyDimensions(inputPath1, inputPath2) == false)
	{
		fprintf(stderr, "Matrices has wrong dimensions, cannot be multiplied!\n");
		exit(0);
	}
	
	printf("Run  \t  ||  \t Multiplying matrices of (%d, %d) and (%d, %d) on %3d thread(s) \t  ||  \t", M, N, K, L, threads);

    int** matrix1; int** matrix2; int** res;
	
	if((matrix1 = allocateMemory(M, N)) == NULL) { fprintf(stderr, "Out of memory for matrix with sizes (%d, %d)!\n", M, N); exit(0); }
	if((matrix2 = allocateMemory(K, L)) == NULL) { fprintf(stderr, "Out of memory for matrix with sizes (%d, %d)!\n", K, L); exit(0); }
	if((res = allocateMemory(M, L)) == NULL) { fprintf(stderr, "Out of memory for matrix with sizes (%d, %d)!\n", M, L); exit(0); }
	
	readFiles(inputPath1, inputPath2, matrix1, matrix2);

    pthread_t* threadsIds = (pthread_t*)malloc(sizeof(pthread_t) * threads);
    threadParameter* threadParams = (threadParameter*)malloc(sizeof(threadParameter) * threads);

    for(int i = 0; i < threads; i++)
    {
		threadParams[i].matrix1 = matrix1;
		threadParams[i].matrix2 = matrix2;
		threadParams[i].res = res;
		threadParams[i].begin = 0 + ((M * L * i) / threads);
		threadParams[i].end = 0 + ((M * L * (i + 1)) / threads);

		pthread_create(threadsIds + i, NULL, dotProductThread, threadParams + i);
    }

	for (int i = 0; i < threads; ++i ) 
	{
		pthread_join(threadsIds[i], NULL);
	}

    stopSOMP(&st);

    printf("   took %5lf second(s)\n", st.end - st.begin);
		
    writeToFile(outputPath, argv[0], threads, st.end - st.begin, res);

    if(freeMemory(matrix1, M) == false) { fprintf(stderr, "Freeing memory for matrix with sizes (%d, %d) failed!\n", M, N); exit(0); }
	if(freeMemory(matrix2, K) == false) { fprintf(stderr, "Freeing memory for matrix with sizes (%d, %d) failed!\n", M, N); exit(0); }
	if(freeMemory(res, M) == false) { fprintf(stderr, "Freeing memory for matrix with sizes (%d, %d) failed!\n", M, N); exit(0); }

    free(threadsIds);
    free(threadParams);
	
	pthread_exit(NULL);

    return 0;
}

void writeToFile(const char* outputPath, const char* program, int threads, double time, int** array)
{
	FILE * filePointer;
	if(filePointer = fopen(outputPath, "w"))
	{
		fprintf(filePointer, "--PROGRAM %s --\n%s%2d%s%5lf%s\n--OUTPUT--\n", program, "Running the program on ", threads, " threads took ", time, " seconds per thread");
		for(int i = 0; i < M; i++)
		{
			for(int j = 0; j < L; j++)
			{
				if(j % M == 0) 
				{ 
					fprintf(filePointer, "$$\n||\t%5d\t", array[i][j]); 
				}
				else 
				{ 
					fprintf(filePointer, "||\t%5d\t", array[i][j]); 
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", outputPath);
	}
}

void readFiles(const char* inputPath1, const char* inputPath2, int** array1, int** array2)
{
	FILE * filePointer;
	if(filePointer = fopen(inputPath1, "r"))
	{
		for(int i = 0; i < M; i++)
		{
			for(int j = 0; j < N; j++)
			{
				fscanf(filePointer, "%d", &array1[i][j]);
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", inputPath1);
	}
	
	if(filePointer = fopen(inputPath2, "r"))
	{
		for(int i = 0; i < K; i++)
		{
			for(int j = 0; j < L; j++)
			{
				fscanf(filePointer, "%d", &array2[i][j]);
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", inputPath2);
	}
}

bool verifyDimensions(const char* inputPath1, const char* inputPath2)
{
	char c;
	FILE * filePointer;
	if(filePointer = fopen(inputPath1, "r"))
	{
		c = getc(filePointer);
		while(!feof(filePointer))
		{
			c = getc(filePointer);
			if(c == '\n' || c == ';' || c == EOF)
			{
				M++;
				N++;
			}
			else if(c == ' ' || c == ',')
			{
				N++;
			}
		}
		fclose(filePointer);
		N = N/M;
	}
	else
	{
		printf("Unable to open file %s!", inputPath1);
		return false;
	}

	if(filePointer = fopen(inputPath2, "r"))
	{
		c = getc(filePointer);
		while(!feof(filePointer))
		{
			c = getc(filePointer);
			if(c == '\n' || c == ';' || c == EOF)
			{
				K++;
				L++;
			}
			else if(c == ' ' || c == ',')
			{
				L++;
			}
		}
		fclose(filePointer);
		L = L/K;
	}
	else
	{
		printf("Unable to open file %s!", inputPath2);
		return false;
	}
	
	if(N != K)
	{
		printf("Matrices are wrong dimensions to multiply! Please specify another input matrices!");
		return false;
	}
	
	return true;
}

void dotProduct(int i, int j, int** matrix1, int** matrix2, int** res)
{
	int sum = 0;
	for(int k = 0; k < N; k++)
	{
		sum = sum + (matrix1[i][k] * matrix2[k][j]);
	}
	res[i][j] = sum;
}

void* dotProductThread(void* params)
{
    threadParameter* parameters = (threadParameter*)params;

    for(int index = parameters -> begin; index < parameters -> end; index++) 
    {
		int i = index / L;
		int j = index % L;

		dotProduct(i, j, parameters -> matrix1, parameters -> matrix2, parameters -> res);
    }

    pthread_exit(NULL);
}

int** allocateMemory(int size1, int size2)
{
	int** array = (int**)malloc(size1 * sizeof(int *)); 
	if(array == NULL)
	{
		return NULL;
	}
    for (int i = 0; i < size1; i++) 
	{
		array[i] = (int*)malloc(size2 * sizeof(int)); 
		if(array[i] == NULL)
		{
			return NULL;
		}
	}
	return array;
}

bool freeMemory(int** array, int size1)
{
	for (int i = 0; i < size1; i++) 
	{
		free(array[i]);
		array[i] = NULL;
		if(array[i] != NULL)
		{
			return false;
		}
	}
	
	free(array);
	array = NULL;
	if(array != NULL)
	{
		return false;
	}
	
	return true;
}