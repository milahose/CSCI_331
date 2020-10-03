#include "enzyme.h"

int please_quit;
int use_yield;
int workperformed;

// The code executed by each enzyme.
void *run_enzyme(void *data) {
  /* This function should :
  1. Cast the void* pointer to thread_info_t* (defined in enzyme.h)
  2. Initialize the swapcount to zero
  3. Set the cancel type to PTHREAD_CANCEL_ASYNCHRONOUS (see pthread_setcanceltype)
  4. If the first letter of the string is a C then call pthread_cancel on this thread.
      (see also, pthread_self)
      Note, depeneding on your platform (and specifically for macOS) you have
            to replace the call to pthread_cancel with
            pthread_exit(PTHREAD_CANCELED) in order to make the cancel test
            pass.
  5. Create a while loop that only exits when please_quit is nonzero
  6. Within this loop: if the first character of the string has an ascii
      value greater than the second (info->string[0] > info->string[1]) then
      - Set workperformed = 1
      - Increment swapcount for this thread
      - Swap the two characters around
  7. If "use_yield" is nonzero then call pthread_yield at the end of the loop.
  8. Return a pointer to the updated structure.
  */
  // while(0) {
  //     pthread_yield();
  // };
  // return NULL;

  thread_info_t* info = data;
  info->swapcount = 0;
  int oldvalue;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldvalue);
  char *s = info -> string;
  if(s[0] == 'C') pthread_cancel(pthread_self());

  while(!please_quit) {
    if(s[0] > s[1]) {
      workperformed = 1;
      info -> swapcount++;
      char temp = s[0];
      s[0] = s[1];
      s[1] = temp;
    }

    if(use_yield) sched_yield();
  };

	return (void *) info;
}

// Make threads to sort string.
// Returns the number of threads created.
// There is a memory bug in this function.
int make_enzyme_threads(pthread_t * enzymes, char *string, void *(*fp)(void *)) {
  int i, rv, len;
  thread_info_t *info;
  len = strlen(string);
  info = (thread_info_t *)malloc(sizeof(thread_info_t));

  for (i = 0; i < len - 1; i++) {
    info->string = string + i;
    rv = pthread_create(enzymes + i, NULL, fp, info);
    if (rv) {
      fprintf(stderr,"Could not create thread %d : %s\n", i, strerror(rv));
      exit(1);
    }
  }
  return len - 1;
}

// Join all threads at the end.
// Returns the total number of swaps.
int join_on_enzymes(pthread_t *threads, int n) {
  int i;
  int totalswapcount = 0;

  // Just to make the code compile. You will need to replace every usage of
  // this variable in the code below. When you are done, placeholder can be
  // deleted.
  void* placeholder = 0;

  for(i = 0; i < n; i++) {
    void *status;
    int rv = pthread_join(threads[i], &status);

    if (placeholder) {
      fprintf(stderr,"Can't join thread %d:%s.\n",i,strerror(rv));
      continue;
    }

    if (placeholder == PTHREAD_CANCELED) {
      continue;
    } else if (status == NULL) {
      printf("Thread %d did not return anything\n",i);
    } else {
      printf("Thread %d exited normally: ",i);// Don't change this line
      int threadswapcount = (int)placeholder;
      // Hint - you will need to cast something.
      printf("%d swaps.\n",threadswapcount); // Don't change this line
      totalswapcount += threadswapcount;// Don't change this line
    }
  }
  return totalswapcount;
}

/* Wait until the string is in order. Note, we need the workperformed flag just
 * in case a thread is in the middle of swapping characters so that the string
 * temporarily is in order because the swap is not complete.
*/
void wait_till_done(char *string, int n) {
  int i;
  while (1) {
    pthread_yield();
    workperformed = 0;
    for (i = 0; i < n; i++)
      if (string[i] > string[i + 1]) {
        workperformed = 1;
      }
    if (workperformed == 0) {
      break;
    }
  }
}

void * sleeper_func(void *p) {
  sleep( (int) p);
  // Actually this may return before p seconds because of signals.
  // See man sleep for more information
  printf("sleeper func woke up - exiting the program\n");
  exit(1);
}

int smp2_main(int argc, char **argv) {
  pthread_t enzymes[MAX];
  int n, totalswap;
  char string[MAX];

  if (argc <= 1) {
    fprintf(stderr,"Usage: %s <word>\n",argv[0]);
    exit(1);
  }

  // Why is this necessary? Why cant we give argv[1] directly to the thread
  // functions?
  strncpy(string,argv[1],MAX);

  please_quit = 0;
  use_yield = 1;

  printf("Creating threads...\n");
  n = make_enzyme_threads(enzymes, string, run_enzyme);
  printf("Done creating %d threads.\n",n);

  pthread_t sleeperid;
  pthread_create(&sleeperid, NULL, sleeper_func, (void*)5);

  wait_till_done(string, n);
  please_quit = 1;
  printf("Joining threads...\n");
  totalswap = join_on_enzymes(enzymes, n);
  printf("Total: %d swaps\n", totalswap);
  printf("Sorted string: %s\n", string);

  exit(0);
}
