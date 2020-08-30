/*
About this program:
- This program counts words.
- The specific words that will be counted are passed in as command-line
  arguments.
- The program reads words (one word per line) from standard input until EOF or
  an input line starting with a dot '.'
- The program prints out a summary of the number of times each word has
  appeared.
- Various command-line options alter the behavior of the program.

E.g., count the number of times 'cat', 'nap' or 'dog' appears.
> ./main cat nap dog
Given input:
 cat
 .
Expected output:
 Looking for 3 words
 Result:
 cat:1
 nap:0
 dog:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smp0_tests.h"

// This is no longer needed with the malloc implementation
#define LENGTH(s) (sizeof(s) / sizeof(*s))

/* Structures */
typedef struct {
  char *word;
  int counter;
} WordCountEntry;


int process_stream(WordCountEntry entries[], int entry_count)
{
  short line_count = 0;
  char buffer[30];

  while (fgets(buffer, sizeof(buffer), stdin)) {
    if (!strcmp(buffer, ".\n")) break;

    // Create token w/ space, tab, and newline delimeters
    char *token = strtok(buffer, " \t\n");

    /* Compare against each entry */
    while (token != NULL) {
      for (int i = 0; i < entry_count; i++) {
        if (!strcmp(entries[i].word, token)) {
          entries[i].counter++;
        }
      }
      token = strtok(NULL, " \t\n");
    }

    line_count++;
  }
  return line_count;
}


void print_result(WordCountEntry entries[], int entry_count)
{
  fprintf(stdout, "Result:\n");
  for (int i = 0; i < entry_count; i++) {
    fprintf(stdout, "%s:%d\n", entries[i].word, entries[i].counter);
  }
}


void printHelp(const char *name)
{
  fprintf(stderr, "usage: %s [-h] [-f]FILENAME <word1> ... <wordN>\n", name);
}


int main(int argc, char **argv)
{
  const char *prog_name = *argv;
  char *file_name = NULL;

  int arg_count = argc - 1;
  int entryCount = 0;

  WordCountEntry *entries;
  entries = (WordCountEntry *)malloc(sizeof(WordCountEntry) * arg_count);

  /* Entry point for the testrunner program */
  if (argc > 1 && !strcmp(argv[1], "-test")) {
    run_smp0_tests(argc - 1, argv + 1);
    return EXIT_SUCCESS;
  }

  // Increment by 1 to skip program name
  argv++;

  while (*argv != NULL) {
    if (**argv == '-') {

      switch ((*argv)[1]) {
        case 'h':
          printHelp(prog_name);
          return EXIT_FAILURE;
        case 'f':
          file_name = (*argv + 2);
          if (strlen(file_name) > 0) // don't print message unless there's actually a file name
            fprintf(stdout, "Results printed to '%s' file in working directory.\n", file_name);
          freopen(file_name, "w", stdout); // open file stream
          break;
        default:
          fprintf(stderr, "%s: Invalid option %s. Use -h for help.\n", prog_name, *argv);
      }
    } else {
      if (entryCount < arg_count) {
        entries[entryCount].word = *argv;
        entries[entryCount++].counter = 0;
      }
    }
    argv++;
  }
  if (entryCount == 0) {
    fprintf(stderr, "%s: Please supply at least one word. Use -h for help.\n", prog_name);
    return EXIT_FAILURE;
  }
  if (entryCount == 1) {
    fprintf(stdout, "Looking for a single word\n");
  } else {
    fprintf(stdout, "Looking for %d words\n", entryCount);
  }

  process_stream(entries, entryCount);
  print_result(entries, entryCount);

  return EXIT_SUCCESS;
}
