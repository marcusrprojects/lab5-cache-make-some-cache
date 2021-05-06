/*
 * cache.c - a hardware cache simulator.
 *
 * Marcus Ribeiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <math.h>

typedef unsigned long long ull;

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
ull global_timer = 0;

/*

This represents one cache line, not including dirty bits or actual data since they are
not utilized in this program.

*/
typedef struct cache_line { //can't initialize struct values here
	char valid_bit; //1 or 0
	ull lru; //refers to updated global_timer
	ull tag;
} cache_line;

/**
 * readfile:
 *
 * Reads in a file using fgets, fclose, and sscanf. Parses this file to simulate loading data into a cache, adding to hit, miss, and eviction counters
 * as it traverses
 *
 * Parameters: trace file to read, an initialized cache.
 */
void read_file(char* file, cache_line** cache) {

	FILE * fPointer = fopen(file, "r");
	char buf[100];

	if (fPointer == NULL) {
		perror("Cannot open file");
	}

	while (fgets(buf, 100, fPointer) != NULL) { //count misses, hits, and evicts

		/* define variables to be utilized further */
		char instruct;
		ull address;
		int size;

		sscanf(buf," %c %llx,%d ",&instruct,&address,&size); //avoids I instruction and stores instruction, address and size for future use.

		/* Deconstruct the address to get important numbers: tag and set index */
		ull tag = (address >> (b + s));
		ull index = (address >> b) & ((1 << s) - 1);

		/* update global timer for LRU */
		global_timer++;

		if (instruct == 'L' || instruct == 'S' || instruct == 'M') { //L, S, and M instructions should all initially go through the same process in this program
		cache_line* currentSet = cache[index];

		int if_dealt_with = 0; //0 means that this buffer has not been put into the cache yet

		for (int i = 0; i < E; i++) {

			/* if it is a cache hit */
			if ((tag == currentSet[i].tag) && (currentSet[i].valid_bit == 1) && (if_dealt_with == 0)) {
				if_dealt_with++;
				hit_count++;
				currentSet[i].lru = global_timer;
			}

			/* if it is a cache miss in a set that already has the same tag, but with invalid data */
			else if ((tag == currentSet[i].tag) && (currentSet[i].valid_bit != 1) && (if_dealt_with == 0)) {
				if_dealt_with++;
				miss_count++;
				currentSet[i].valid_bit = 1;
				currentSet[i].lru = global_timer;
			}
		}

		/* If it is a cache miss that did not have an invalid representative in the cache and there is an empty space in the set */
		if (if_dealt_with == 0) {
			for (int i = 0; i < E; i++) {
				if ((currentSet[i].lru == 0) && (if_dealt_with == 0)) { //has not been accessed, so store it
					miss_count++;
					currentSet[i].tag = tag;
					currentSet[i].valid_bit = 1;
					currentSet[i].lru = global_timer;
					if_dealt_with++;
				}
			}
		}

		/* If it is a cache miss that did not have an invalid representative in the cache and that had no empty spaces in its set, it should evict data */
		if (if_dealt_with == 0) {
			miss_count++;
			int min_index = 0;
			for (int i = 1; i < E; i++) {
				if (currentSet[i].lru < currentSet[min_index].lru) {
					min_index = i;
				}
			}
			eviction_count++;
			currentSet[min_index].tag = tag;
			currentSet[min_index].valid_bit = 1;
			currentSet[min_index].lru = global_timer;
		}
	}

	/* If instruction "M," meaning data modify, there is guarranteed to be another hit after the previous hit/miss */
	if (instruct == 'M') {
		cache_line* currentSet = cache[index];
		int if_dealt_with = 0;
		for (int i = 0; i < E; i++) {
			if ((tag == currentSet[i].tag) && (currentSet[i].valid_bit == 1) && (if_dealt_with == 0)) {
				if_dealt_with++;
				hit_count++;
				currentSet[i].lru = global_timer;
			}
		}
	}

	}
	fclose(fPointer);
}

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

  /* declare cache vars */
  cache_line* cache_set = (cache_line*)malloc(E * sizeof(cache_line));
  cache_line** cache = (cache_line**)malloc(S * sizeof(cache_set));

  /* Put cache_line structs into the cache, initializing everything to 0 */
  for (int i = 0; i < S; i++) { //for each set
	cache[i] = malloc(E * sizeof(cache_line));
	for (int j = 0; j < E; j++) { //for each line in a set

		cache[i][j].valid_bit = 0;
		cache[i][j].lru = 0;
		cache[i][j].tag = 0;
  	}
  }

  /* Read in the specified file */
  read_file(trace_file, cache);

  /* Output the final cache statistics */
  printSummary(hit_count, miss_count, eviction_count);

  /* Free memory */
  for (int i = 0; i < S; i++) { //for each set
	free(cache[i]);
  }

  free(cache);

  return 0;

}

