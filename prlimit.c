#define _GNU_SOURCE
/*
If you define this macro, everything is included: ISO C89, ISO C99, 
POSIX.1, POSIX.2, BSD, SVID, X/Open, LFS, and GNU extensions.
*/
#define _FILE_OFFSET_BITS 64
/*
This macro determines which file system interface 
shall be used, one replacing the other. 
*/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE);} while (0)
// void perror(const char *string);
// The perror() function prints an error message to stderr.
int main(int argc, char *argv[])
{
    struct rlimit old, new;
    struct rlimit *newp;
    pid_t pid;

    if (!(argc == 2 || argc == 4)) {
        fprintf(stderr, "Usage: %s <pid> [<new-soft-limit> "
        "<new-hard-limit>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid = atoi(argv[1]); /* PID of target process */

    newp = NULL;
    if (argc == 4) {
        new.rlim_cur = atoi(argv[2]);
        new.rlim_max = atoi(argv[3]);
        newp = &new;
    }

    /*
    The rlim_cur member specifies the current or soft limit and the 
    rlim_max member specifies the maximum or hard limit.
    */

    /* Set CPU time limit of target process; retrieve and display
    previous limit */

    // int prlimit(pid_t pid, int resource, const struct rlimit *new_limit, struct rlimit *old_limit);

    // If the old_limit argument is a not NULL, then a successful call to prlimit() places
    // the previous soft and hard limits for resource in the rlimit structure pointed to
    // by old_limit.

    // RLIMIT_CPU
    // CPU time limit in seconds.
    if (prlimit(pid, RLIMIT_CPU, newp, &old) == -1)
        errExit("prlimit-1");

    printf("Previous limits: soft=%lld; hard=%lld\n", (long long) old.rlim_cur, (long long) old.rlim_max);

    /* Retrieve and display new CPU time limit */

    if (prlimit(pid, RLIMIT_CPU, NULL, &old) == -1)
        errExit("prlimit-2");
    printf("New limits: soft=%lld; hard=%lld\n", (long long) old.rlim_cur, (long long) old.rlim_max);

    exit(EXIT_SUCCESS);
}