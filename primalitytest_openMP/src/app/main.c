#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stopper.h>
#include <omp.h>

unsigned long long gcd(unsigned long long a, unsigned long long b);
unsigned long long modPow(unsigned long long base, unsigned long long exponent, unsigned long long modulus);
bool isPrime(unsigned int number);
bool* allocateMemoryB(unsigned long long size);
unsigned long long* allocateMemoryULL(unsigned long long size);
bool freeMemoryB(bool* array);
bool freeMemoryULL(unsigned long long* array);
void readFile(const char* inputPath, unsigned long long size, unsigned long long* array, unsigned int divider);
void writeToFile(const char* outputPath, const char* program, unsigned int threads, unsigned long long size, unsigned long long* array1, bool* array2, unsigned int divider);
unsigned long long countNumbers(const char* inputPath);
unsigned int getDividerforN(unsigned long long size);

bool hasWritten = false;
unsigned long long partPointer = 0;

int main(int argc, char** argv)
{	
	if(argc != 3 && argc != 4)
	{
		printf("Argument(s) missing! Use program as:\n '%s <threads> <prime>'    ||    '%s <threads> <inputPath> <outputPath>'", argv[0], argv[0]);
		exit(-1);
	}
	
	stopperOMP st;
	startSOMP(&st);
	
	const char* inputPath = argv[2];
	const char* outputPath = argv[3];
	unsigned int threads = atoi(argv[1]) > 1 ? atoi(argv[1]) : 1;
	
	srand(time(NULL));
	
	bool* results;
	unsigned long long* numbers;
	
	unsigned int divider = 1;
	
	if(argc == 3)
	{
		unsigned long long testNumber = atoi(argv[2]);
		
		printf("Testing %llu for primality ... \t Result: %s\t  ||  \t", testNumber, (isPrime(testNumber) == true ? "TRUE" : "FALSE"));
		
		stopSOMP(&st);
		
		printf("   took %5lf second(s)\n", st.end - st.begin);
	}
	else
	{	
		printf("Counting input...\n\n");
		
		unsigned long long N = countNumbers(inputPath);
		divider = 2 * getDividerforN(N);
		
		printf("Running...  \t  ||  \t Finding primes on %2u thread(s) in %llu numbers\t  ||  \t\n", threads, N);
		
		if((numbers = allocateMemoryULL(N / divider)) == NULL) { fprintf(stderr, "Out of memory for array 'numbers' with size (%llu)!\n", N); exit(-1); }
		if((results = allocateMemoryB(N / divider)) == NULL) { fprintf(stderr, "Out of memory for array 'res' with size (%llu)!\n", N); exit(-1); }
		
		for(unsigned int j = 0; j < divider; j++)
		{ 
			printf("Part[%2u] of [%2u]\t  ||  \t", j + 1, divider);

			readFile(inputPath, N, numbers, divider);

			#pragma omp parallel for num_threads(threads)
			for(unsigned long long i = 0; i < N / divider; i++)
			{
				results[i] = isPrime(numbers[i]);
			}
			
			stopSOMP(&st);
			
			printf("Writing output to file...\n");
			writeToFile(outputPath, argv[0], threads, N, numbers, results, divider);
		}
		printf("*** took %5lf second(s)\n", st.end - st.begin);
	}
	
	freeMemoryULL(numbers);
	freeMemoryB(results);	
	
	return 0;
}

unsigned long long gcd(unsigned long long a, unsigned long long b)
{
	unsigned long long t;
    while(b != 0)
	{
       t = b; 
       b = a % b; 
       a = t; 
	}
    return a;
}

unsigned long long modPow(unsigned long long base, unsigned long long exponent, unsigned long long modulus)
{
	if(modulus == 1)
	{
		return 0;
	}
	else
	{
		unsigned int result = 1;
		base = base % modulus;
		while(exponent > 0)
		{
			if(exponent % 2 == 1)
			{
				result = (result * base) % modulus;
			}
			exponent = exponent >> 1ULL;
			base = (base * base) % modulus;
		}
		return result;
	}
}

bool isPrime(unsigned int number)
{
	if(number <= 1)
	{
		return false;
	}
	else
	{ 	
		unsigned int k = 0;
		do 
		{
			unsigned long long base = (rand() % (number - 1)) + 1; 
			if((gcd(base, number) != 1) || ((modPow(base, number - 1, number) % number) != 1)) 
			{
				return false;
			}
			k++;
			
		} while(k < (number < 100 ? number : 100));
		return true;
	}
}

bool* allocateMemoryB(unsigned long long size)
{
	bool* array = (bool*)malloc(size * sizeof(bool)); 
	if(array == NULL)
	{
		return NULL;
	}
	return array;
}

unsigned long long* allocateMemoryULL(unsigned long long size)
{
	unsigned long long* array = (unsigned long long*)malloc(size * sizeof(unsigned long long)); 
	if(array == NULL)
	{
		return NULL;
	}
	return array;
}

bool freeMemoryB(bool* array)
{	
	free(array);
	array = NULL;
	if(array != NULL)
	{
		return false;
	}
	return true;
}

bool freeMemoryULL(unsigned long long* array)
{	
	free(array);
	array = NULL;
	if(array != NULL)
	{
		return false;
	}
	return true;
}

void readFile(const char* inputPath, unsigned long long size, unsigned long long* array,  unsigned int divider)
{
	FILE * filePointer;
	if(filePointer = fopen(inputPath, "r"))
	{
		fseek(filePointer, partPointer, SEEK_SET);
		for(unsigned long long i = 0; i < size / divider; i++)
		{
			fscanf(filePointer, "%llu", &array[i]);
		}
		partPointer = ftell(filePointer);

		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", inputPath);
		exit(-1);
	}
}

void writeToFile(const char* outputPath, const char* program, unsigned int threads, unsigned long long size, unsigned long long* array1, bool* array2, unsigned int divider)
{
	FILE * filePointer;
	if(filePointer = fopen(outputPath, "a+"))
	{
		fprintf(filePointer, "--PROGRAM %s --\n%s%2u%s\n--OUTPUT--\n", program, "The program was run on ", threads, " threads.");
		for(unsigned long long i = 0; i < size / divider; i++)
		{
			if(i % 10 == 0) 
			{ 
				fprintf(filePointer, "\n||  %8llu - %5s  ", array1[i], (array2[i] == true ? "TRUE" : "FALSE")); 
			}
			else
			{ 
				fprintf(filePointer, "||  %8llu - %5s  ", array1[i], (array2[i] == true ? "TRUE" : "FALSE")); 
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", outputPath);
		exit(-1);
	}
}

unsigned long long countNumbers(const char* inputPath)
{
    char c;
    unsigned long long count = 0;
	FILE * filePointer;
	if(filePointer = fopen(inputPath, "r"))
	{
		c = getc(filePointer);
		while(!feof(filePointer))
		{
			c = getc(filePointer);
			if(c == '\n' || c == EOF)
			{
				count++;
			}
		}
		fclose(filePointer);
	}
	else
	{
		printf("Unable to open file %s!", inputPath);
		exit(-1);
	}
	return count;
}

unsigned int getDividerforN(unsigned long long size)
{
	unsigned int exponent = 0;
	unsigned int i = 0;
	while(pow(2, exponent) < size)
	{
		exponent++;
	}
	
	while(exponent > pow(2, i))
	{
		i++;
	}
	
	return (unsigned int)pow(2, i);
}
