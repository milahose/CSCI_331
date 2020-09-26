/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "testrunner.h"
#include "grading_tests.h"

#define quit_if(cond) do {if (cond) exit(EXIT_FAILURE);} while(0)

#undef	fgets

static FILE *waitfd = NULL, *waitpidfd = NULL;

/* Prepare input, reroute file descriptors, and run the program. */
static void run_test(const char *input, int argc, char **argv)
{
	int fork_pid = -1;
	int infds[2];
	/* Prepare input */
	FILE *in = fopen("smp1.in", "w");
	fputs(input, in);
	fclose(in);
	pipe(infds);
	fork_pid = fork();
	if (fork_pid == 0) {
		/* Reroute standard file descriptors */
		//freopen("smp1.in",  "r", stdin );
		fflush(stdin);
		dup2(infds[0], 0);
		close(infds[0]);
		close(infds[1]);
		freopen("smp1.out", "w", stdout);
		freopen("smp1.err", "w", stderr);
		/* Run the program */
		quit_if(main(argc, argv) != EXIT_SUCCESS);
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	} else if (fork_pid > 0) {
		FILE *pgm = fdopen(infds[1], "w");
		int buflen = strlen(input)+1;
		char *buf = (char *) malloc(buflen*sizeof(char));
		in = fopen("smp1.in", "r");
		while (!feof(in)) {
			fgets(buf, buflen, in);
		//	fputs(buf, stdout);
		//	fflush(stdout);
			fputs(buf, pgm);
			fflush(pgm);
			usleep(40000); // wait 0.04 seconds between lines
		}
		fclose(in);
		waitpid(fork_pid, 0, 0);
		fclose(pgm);
		close(infds[1]);
	} else {
		fprintf(stderr, "run_test: fork() error\n");
	}
}

/* P5.1: Test of executing commands in the path (1/2 point: only 'ls') */
static int test_path_weak(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *err;
	int pid_tmp;
	/* Run the test */
	run_test("ls\n/bin/ls\nexit\n", 1, args);
	/* Check output */
	err = fopen("smp1.err", "r");
	quit_if(fscanf(err, "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n"
	                    "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n",
	               &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 4);
	quit_if(!feof(err));
	fclose(err);
	return EXIT_SUCCESS;
}

/* P5.1: Test of executing commands in the path (another 1/2 point: not just 'ls') */
static int test_path(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *err;
	int pid_tmp;
	/* Run the test */
	run_test("true\n/bin/true\nexit\n", 1, args);
	/* Check output */
	err = fopen("smp1.err", "r");
	quit_if(fscanf(err, "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n"
	                    "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n",
	               &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 4);
	quit_if(!feof(err));
	fclose(err);
	return EXIT_SUCCESS;
}

/* P5.2: Test of command line counter */
static int test_counter(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out;
	int pid_tmp;
	/* Run the test */
	run_test("\n/bin/true\n\n/bin/true\n/bin/true\n\n/bin/echo hello\n\n\n/bin/true\n/bin/echo last one\nexit\n", 1, args);
	/* Check output */
	out = fopen("smp1.out", "r");
	quit_if(fscanf(out, "Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)2> Shell(pid=%d)2> Shell(pid=%d)3> Shell(pid=%d)4> Shell(pid=%d)4> hello\n"
	                    "Shell(pid=%d)5> Shell(pid=%d)5> Shell(pid=%d)5> Shell(pid=%d)6> last one\n"
	                    "Shell(pid=%d)7> Exiting process %d\n", &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 13);
	fclose(out);
	return EXIT_SUCCESS;
}

/* P5.3: Test of re-executing earlier commands */
static int test_rerun(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pid_tmp, warned_not_valid;
	/* Run the test */
	run_test("/bin/echo test\n!1\n!2\n/bin/echo yeah\n!4\n!6\nexit\n", 1, args);
	/* Check output */
	out = fopen("smp1.out", "r");
	quit_if(fscanf(out, "Shell(pid=%d)1> test\n"
	                    "Shell(pid=%d)2> test\n"
	                    "Shell(pid=%d)3> test\n"
	                    "Shell(pid=%d)4> yeah\n"
	                    "Shell(pid=%d)5> yeah\n"
	                    "Shell(pid=%d)6> Shell(pid=%d)",
		       &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 7);
	fclose(out);
	err = fopen("smp1.err", "r");
	/* Next, check for the "Not valid" message: */
	warned_not_valid = 0;
	/* Use a while loop because multiple processes write to stderr concurrently. */
	while (!warned_not_valid && !feof(err)) {
		char not_valid[11];
		fgets(not_valid, 11, err);
		if (!strncasecmp(not_valid, "Not valid", 9))
			warned_not_valid = 1;
	}
	quit_if(!warned_not_valid);
	return EXIT_SUCCESS;
}

/* P5.4: Test of waitpid() */
static int test_waitpid(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out;
	int pid_tmp;
	/* Run the test */
	unlink("smp1.wait");
	unlink("smp1.waitpid");
	waitfd = fopen("smp1.wait", "w");
	waitpidfd = fopen("smp1.waitpid", "w");
	run_test("/bin/true\nexit\n", 1, args);
	fclose(waitpidfd);
	fclose(waitfd);
	/* Check output */
	out = fopen("smp1.waitpid", "r");
	quit_if(out == NULL);
	quit_if(fscanf(out, "waitpid %d\n", &pid_tmp) != 1);
	//quit_if(pid_tmp <= 0); // Instructions weren't clear: must wait on particular process
	fclose(out);
	out = fopen("smp1.wait", "r");
	fgetc(out);
	quit_if(!feof(out));
	fclose(out);
	return EXIT_SUCCESS;
}

/* P5.5: Test of sub (includes going to a different directory) */
static int test_sub(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pids[10];
	/* Run the test */
	run_test("cd /tmp\n/bin/pwd\nsub\ncd /\n/bin/pwd\nexit\n/bin/pwd\nexit\n", 1, args);
	/* Check output */
	out = fopen("smp1.out", "r");
	err = fopen("smp1.err", "r");
	/* First, check that the subshells were invoked. */
	/*
	*/
	fscanf(out, "Shell(pid=%d)1> Shell(pid=%d)2> /tmp\n"
	            "Shell(pid=%d)3> Shell(pid=%d)1> Shell(pid=%d)2> /\n"
	            "Shell(pid=%d)3> Exiting process %d\n"
	            "Shell(pid=%d)4> /tmp\n"
	            "Shell(pid=%d)5> Exiting process %d\n",
	       &pids[0], &pids[1], &pids[2], &pids[3], &pids[4], &pids[5], &pids[6], &pids[7], &pids[8], &pids[9]);
	quit_if(!((pids[0] == pids[1])&&(pids[1] == pids[2])&&(pids[2] != pids[3])&&(pids[3] == pids[4])&&(pids[4] == pids[5])&&(pids[5] == pids[6])&&(pids[6] != pids[7])&&(pids[7] == pids[8])&&(pids[8] == pids[9])));
	fclose(out);
	fclose(err);
	return EXIT_SUCCESS;
}

/* P5.6: Test of depth-limited sub */
static int test_sub_depth(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pids[4], warned_too_deep;
	/* Run the test */
	run_test("sub\nsub\nsub\nexit\nexit\nexit\n", 1, args);
	/* Check output */
	out = fopen("smp1.out", "r");
	err = fopen("smp1.err", "r");
	/* First, check that the subshells were invoked. */
	fscanf(out, "Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)2> ", &pids[0], &pids[1], &pids[2], &pids[3]);
	quit_if(!((pids[0] != pids[1]) && (pids[1] != pids[2]) && (pids[0] != pids[2]) && (pids[2] == pids[3])));
	/* Next, check for the "Too deep!" message: */
	warned_too_deep = 0;
	/* Use a while loop because multiple processes write to stderr concurrently. */
	while (!warned_too_deep && !feof(err)) {
		char too_deep[11];
		fgets(too_deep, 11, err);
		if (!strncasecmp(too_deep, "Too deep!", 9))
			warned_too_deep = 1;
	}
	quit_if(!warned_too_deep);
	fclose(out);
	fclose(err);
	return EXIT_SUCCESS;
}

/*
 * Main entry point for SMP1 test harness
 */
int run_smp1_tests(int argc, char **argv)
{
	/* Tests can be invoked by matching their name or their suite name
	   or 'all' */
	testentry_t tests[] = {
		{ "sub-depth", "smp1", test_sub_depth },
		{ "sub", "smp1", test_sub },
		{ "waitpid", "smp1", test_waitpid },
		{ "rerun", "smp1", test_rerun },
		{ "counter", "smp1", test_counter },
		{ "path", "smp1", test_path },
		{ "path-weak","smp1", test_path_weak } };
	int result = run_testrunner(argc, argv, tests, sizeof(tests) / sizeof (testentry_t));
	unlink("smp1.in");
	unlink("smp1.out");
	unlink("smp1.err");
	unlink("smp1.waitpid");
	unlink("smp1.wait");
	return result;
}

#undef	wait
#undef	waitpid

int wrap_wait(int *status)
{
	if (waitfd != NULL) {
		fprintf(waitfd, "wait\n");
		fflush(waitfd);
	}
	return wait(status);
}

int wrap_waitpid(int pid, int *status, int options)
{
	if (waitpidfd != NULL) {
		fprintf(waitpidfd, "waitpid %d\n", pid);
		fflush(waitpidfd);
	}
	return waitpid(pid, status, options);
}

char *ubfgets2(char *buf, int len, FILE *f)
{
        int i = 0, fd = fileno(f);
        do {
                read(fd, buf+i, 1);
                i++;
        } while ((i < len-1) && (buf[i-1] != '\n'));
        buf[i] = 0;
        return buf;
}
