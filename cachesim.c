/*
 * cache.c - a hardware cache simulator.
 *
 * [YOUR NAME(S) HERE]
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <math.h>

/* Globals set by command line arguments */
int verbose = 0; /* whether to print verbose output */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* lines per set */
char* trace_file = NULL; /* input trace filename */

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;

/* 
 * Print the cache simulation statistics. The simulator must call
 * this function in order to be properly tested. Do not modify!
 */
void printSummary(int hits, int misses, int evictions) {
  printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
  FILE* output_fp = fopen(".cachesim_results", "w");
  assert(output_fp);
  fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
  fclose(output_fp);
}

/*
 * Print program usage information. Do not modify!
 */
void printUsage(char** argv) {
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Print verbose output.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace filename.\n");
  printf("\nExamples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/t1.trace\n", argv[0]);
  printf("  linux>  %s -s 8 -E 2 -b 4 -t traces/t1.trace -v\n", argv[0]);
  exit(0);
}

/*
 * Reads command-line arguments, runs the cache simulation, and prints
 * the simulation statistics (hits, misses, and evictions) on completion.
 */
int main(int argc, char** argv) {

  /* Parse command line arguments */
  char c;
  while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
    switch (c) {
      case 's':
        s = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        trace_file = optarg;
        break;
      case 'v': // turn on verbose output
        verbose = 1;
        break;
      case 'h': // print the program help message
        printUsage(argv);
        exit(0);
      default: // unrecognized option; print the help message
        printUsage(argv);
        exit(1);
    }
  }

  /* Make sure that all required command line args were specified */
  if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
    printf("%s: Missing required command line argument\n", argv[0]);
    printUsage(argv);
    exit(1);
  }

  /* Compute S and B from command line args */
  S = (int) pow(2, s);
  B = (int) pow(2, b);

  /* An example of verbose output. */
  if (verbose) {
    printf("simulation starting and reading from %s\n", trace_file);
  }


  /**********************************************************************
   * TODO: Run the cache simulation. Remember to modularize using helper
   * functions; don't write the rest of the program inside of main!
   **********************************************************************/


  /* Output the final cache statistics */
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}

