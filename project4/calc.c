/* calc.c - Multithreaded calculator */

#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE];
int num_ops;

static pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
static sem_t progress_lock;

struct progress_t {
    int add;
    int mult;
    int group;
} progress;

/* Utiltity functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
    return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
    return isdigit(c);
}

/* End utility functions */


void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An unspecified error occured!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

/* 
 * Signals end of program
 * be careful: timeToFinish() also accesses buffer 
 */
int timeToFinish()
{
    return buffer[0] == '.';
}

/* 
 * Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
 *  and, if found, adds the two numbers and replaces the addition subexpression
 *  with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
 * to worry about associativity! 
 */
void *adder(void *arg)
{
    int bufferlen;
    int startOffset, remainderOffset;
    int i;
    int value1, value2;
    int result;
    int changed;
    char nString[50];

    while (1) {
        startOffset = remainderOffset = -1;
        value1 = value2 = -1;
        
        // lock the buffer
        pthread_mutex_lock(&buffer_lock);

        if (timeToFinish()) {
            pthread_mutex_unlock(&buffer_lock);
            return NULL;
        }

        /* storing this prevents having to recalculate it in the loop */
        bufferlen = (int) strlen(buffer);
        
        // no change yet
        changed = 0;

        for (i = 0; i < bufferlen; i++) {
            if ( buffer[i] == ';') {
                break;
            }
            
            // start with a digit
            if (isNumeric(buffer[i])) {
                startOffset = i;
                value1 = string2int(buffer + i);
                
                while (isNumeric(buffer[i])) {
                    i++;
                }
                
                // check if current expression is a + expression
                if (buffer[i] != '+' || !isNumeric(buffer[i+1])) {
                    continue;
                }
                
                value2 = string2int(buffer + i + 1);
                result = value1 + value2;
                
                // increment index past 2nd operand
                do {
                    i++;
                } while ( isNumeric(buffer[i]) );
                
                remainderOffset = i;
                sprintf(nString, "%d", result);
                
                // rearrange buffer
                strcpy(buffer + startOffset, nString);
                strcpy( (buffer + startOffset + strlen(nString)), (buffer + remainderOffset) );
                
                // set buffer length and position
                bufferlen = (int) strlen(buffer);
                i = startOffset + ((int) strlen(nString)) - 1;
                
                // indicate that current thread has updated the buffer
                changed = 1;
                
                // increment number of operations
                num_ops++;
            }
        }
        
        // unlock buffer
        pthread_mutex_unlock(&buffer_lock);
        
        // update progress
        sem_wait(&progress_lock);
        progress.add = changed ? 2 : 1;
        sem_post(&progress_lock);
        
        // yield processor to other threads
        sched_yield();
    }
}

/* 
 * Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
 * "5*6" and, if found, multiplies the two numbers and replaces the
 * mulitplication subexpression with the result ("1+(5*6)+8" becomes
 * "1+(30)+8"). 
 */
void *multiplier(void *arg)
{
    int bufferlen;
    int startOffset, remainderOffset;
    int i;
    int value1, value2;
    int result;     // product of value1, value2
    int changed;    // used to indicate buffer has changed
    char nString[50];


    while (1) {
        startOffset = remainderOffset = -1;
        value1 = value2 = -1;

        // lock the buffer
        pthread_mutex_lock(&buffer_lock);
        
        if (timeToFinish()) {
            pthread_mutex_unlock(&buffer_lock);
            return NULL;
        }
        
        /* storing this prevents having to recalculate it in the loop */
        bufferlen = (int) strlen(buffer);
        
        // no change yet
        changed = 0;
        
        for (i = 0; i < bufferlen; i++) {
            if ( buffer[i] == ';') {
                break;
            }
            
            // start with a digit
            if (isNumeric(buffer[i])) {
                startOffset = i;
                value1 = string2int(buffer + i);
                
                // increment index past 1st operand
                while (isNumeric(buffer[i])) {
                    i++;
                }
                
                // check if current expression is a * expression
                if (buffer[i] != '*' || !isNumeric(buffer[i+1])) {
                    continue;
                }
                
                value2 = string2int(buffer + i + 1);
                result = value1 * value2;
                
                do {
                    i++;
                } while (isNumeric(buffer[i]));
                
                remainderOffset = i;
                sprintf(nString, "%d", result);
                
                // rearrange buffer
                strcpy(buffer + startOffset, nString);
                strcpy( (buffer + startOffset + strlen(nString)), (buffer + remainderOffset) );
                
                // set buffer length and position
                bufferlen = (int) strlen(buffer);
                i = startOffset + ((int) strlen(nString)) - 1;
                
                // indicate that current thread has updated the buffer
                changed = 1;
                
                // increment number of operations
                num_ops++;
            }
        }
        
        // unlock buffer
        pthread_mutex_unlock(&buffer_lock);
        
        // update progress
        sem_wait(&progress_lock);
        progress.mult = changed ? 2 : 1;
        sem_post(&progress_lock);
        
        // yield processor to other threads
        sched_yield();
    }
}


/* 
 * Looks for a number immediately surrounded by parentheses [e.g.
 * "(56)"] in the buffer and, if found, removes the parentheses leaving
 * only the surrounded number. 
 */
void *degrouper(void *arg)
{
    int bufferlen;
    int startOffset = 0;
    int i;
    int changed;

    while (1) {
        // lock the buffer
        pthread_mutex_lock(&buffer_lock);
        
        if (timeToFinish()) {
            pthread_mutex_unlock(&buffer_lock);
            return NULL;
        }

        /* storing this prevents having to recalculate it in the loop */
        bufferlen = (int) strlen(buffer);

        // set changed flag to 0
        changed = 0;
        
        for (i = 0; i < bufferlen; i++) {
            if (buffer[i] == ';') {
                break;
            }
            
            // check for '(' followed by a naked number followed by ')'
            if (buffer[i] == '(' && isNumeric(buffer[i + 1])) {
                startOffset = i;
                
                // increment index past all digits
                do {
                    i++;
                } while (isNumeric(buffer[i]));
                
                // unwind incremented index if not ')'
                if ( buffer[i] != ')') {
                    i--;
                    continue;
                }
            
                // remove ')' by shifting the tail end of the expression
                strcpy( (buffer + i), (buffer + i + 1) );
                
                // remove '(' by shifting the beginning of the expression
                strcpy( (buffer + startOffset), (buffer + startOffset + 1) );
                
                // set buffer length and position
                bufferlen -= 2;
                i = startOffset;
                
                changed = 1;
                num_ops++;
            }
        }
        
        // unlock buffer
        pthread_mutex_unlock(&buffer_lock);
        
        // update progress
        sem_wait(&progress_lock);
        progress.group = changed ? 2 : 1;
        sem_post(&progress_lock);
        
        // yield processor to other threads
        sched_yield();
    }
}


/* 
 * sentinel waits for a number followed by a ; (e.g. "453;") to appear
 * at the beginning of the buffer, indicating that the current
 * expression has been fully reduced by the other threads and can now be
 * output.  It then "dequeues" that expression (and trailing ;) so work can
 * proceed on the next (if available). 
 */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;

    while (1) {
        // lock the buffer
        pthread_mutex_lock(&buffer_lock);
        
        if (timeToFinish()) {
            pthread_mutex_unlock(&buffer_lock);
            return NULL;
        }

        /* storing this prevents having to recalculate it in the loop */
        bufferlen = (int) strlen(buffer);

        for (i = 0; i < bufferlen; i++) {
            if (buffer[i] == ';') {
                if (i == 0) {
                    printErrorAndExit("Sentinel found empty expression!");
                } else {
                    /* null terminate the string */
                    numberBuffer[i] = '\0';
                    
                    /* print out the number we've found */
                    fprintf(stdout, "%s\n", numberBuffer);
                    
                    /* shift the remainder of the string to the left */
                    strcpy(buffer, &buffer[i + 1]);
                    
                    break;
                }
            } else if (!isNumeric(buffer[i])) {
                // current expression still needs to finish processing
                break;
            } else {
                numberBuffer[i] = buffer[i];
            }
            
            // check for progress
            if (buffer[0] != '\0') {
                // lock progress semaphore
                sem_wait(&progress_lock);
                
                // if all fields are 1 or 2 then all threads have run
                if (progress.add && progress.mult && progress.group) {
                    if (progress.add > 1 || progress.mult > 1 || progress.group > 1) {
                        // made progress: reset fields
                        progress.add = progress.mult = progress.group = 0;
                    } else {
                        fprintf(stdout, "No progress can be made\n");
                        exit(EXIT_FAILURE);
                    }
                }
                
                sem_post(&progress_lock);
            }
        }
        
        // unlock buffer
        pthread_mutex_unlock(&buffer_lock);
        // yield processor to other threads
        sched_yield();
    }
}

/* 
 * reader reads in lines of input from stdin and writes them to the
 * buffer 
 */
void *reader(void *arg)
{
    while (1) {
        char tBuffer[100];
        int currentlen;
        int newlen;
        int free;

        // read expression from stdin
        fgets(tBuffer, sizeof(tBuffer), stdin);

        /* Sychronization bugs in remainder of function need to be fixed */

        // length of new expression as a string
        newlen = (int) strlen(tBuffer);
        currentlen = (int) strlen(buffer);

        /* if tBuffer comes back with a newline from fgets, remove it */
        if (tBuffer[newlen - 1] == '\n') {
            /* shift null terminator left */
            tBuffer[newlen - 1] = tBuffer[newlen];
            newlen--;
        }

        do {
            // lock buffer
            pthread_mutex_lock(&buffer_lock);
            
            currentlen = (int) strlen(buffer);
            free = sizeof(buffer) - currentlen - 2;
            
            // unlock buffer
            pthread_mutex_unlock(&buffer_lock);
            sched_yield();
        } while (free < newlen);

        // lock buffer
        pthread_mutex_lock(&buffer_lock);
        
        // we can add another expression now
        strcat(buffer, tBuffer);
        strcat(buffer, ";");
        
        // reset flag that indicates progress status
        sem_wait(&progress_lock);
        progress.add = progress.mult = progress.group = 0;
        sem_post(&progress_lock);
        
        // unlock buffer
        pthread_mutex_unlock(&buffer_lock);

        /* Stop when user enters '.' */
        if (tBuffer[0] == '.') {
            return NULL;
        }
    }
}


/* Where it all begins */
int smp3_main(int argc, char **argv)
{
    void *arg = 0;		/* dummy value */

    /* let's create our threads */
    if (pthread_create(&multiplierThread, NULL, multiplier, arg)
	|| pthread_create(&adderThread, NULL, adder, arg)
	|| pthread_create(&degrouperThread, NULL, degrouper, arg)
	|| pthread_create(&sentinelThread, NULL, sentinel, arg)
	|| pthread_create(&readerThread, NULL, reader, arg)) {
	printErrorAndExit("Failed trying to create threads");
    }

    /* you need to join one of these threads... but which one? */
    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(degrouperThread);
    pthread_detach(readerThread);
    pthread_join(sentinelThread, NULL); // main thread waits for this one to finish

    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);
    return EXIT_SUCCESS;
}