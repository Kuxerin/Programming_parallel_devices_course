#include <stopper.h>
#include <stdarg.h>
#include <stdio.h>
#include <omp.h>

void startS(stopper *st)
{
  st->begin= clock();
}

void stopS(stopper *st)
{
  st->end= clock();
}

int tprintf(stopper *st, const char* fmt, ...)
{
  float d= st->end - st->begin;
  long ms, s;
  
  d*=1000;
  d/=CLOCKS_PER_SEC;

  ms= d;
  s= ms / 1000;
  ms= ms % 1000;
	
  va_list arg;
  va_start(arg, fmt);
  printf("%03ld.%02ld ", s, ms);
  vprintf(fmt, arg);
  va_end(arg);
  fflush(stdout);
}

void startSOMP(stopperOMP *st)
{
  st->begin= omp_get_wtime();
}

void stopSOMP(stopperOMP *st)
{
  st->end= omp_get_wtime();
}

int tprintfOMP(stopperOMP *st, const char* fmt, ...)
{
    double d= st->end - st->begin;
    
    va_list arg;
    va_start(arg, fmt);
    printf("%02lf ", d);
    vprintf(fmt, arg);
    va_end(arg);
    fflush(stdout);
}