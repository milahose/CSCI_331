int run_smp1_tests(int argc, char **argv);
int main(int argc, char **argv);
int wrap_wait(int *status);
int wrap_waitpid(int pid, int *status, int options);
#define wait	wrap_wait
#define	waitpid	wrap_waitpid
#define	strlen(s)	(fflush(stdout),strlen(s))
char *ubfgets2(char *buf, int len, FILE *f);
#define	fgets	ubfgets2
