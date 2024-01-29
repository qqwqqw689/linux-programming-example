#include <errno.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

// perror - print a system error message
// errno - number of last error

#define errExit(msg)    do { perror(msg); \
                             exit(EXIT_FAILURE); } while (0)

#define errExitEN(en, msg) \
                        do { errno = en; perror(msg); \
                            exit(EXIT_FAILURE); } while (0)

char **environ;

int main(int argc, char *argv[])
{
    pid_t child_pid;
    int s, opt, status;
    sigset_t mask;
    posix_spawnattr_t attr;
    posix_spawnattr_t *attrp;
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_t *file_actionsp;

    /* Parse command-line options, which can be used to specify an
       attributes object and file actions object for the child. */

    attrp = NULL;
    file_actionsp = NULL;

    // getopt() function parses the command-line arguments

    while ((opt = getopt(argc, argv, "sc")) != -1) {
        switch (opt) {
        case 'c':       /* -c: close standard output in child */

            /* Create a file actions object and add a "close"
               action to it. */

            // int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
            // Upon successful completion, function shall return zero;
            // otherwise, an error number shall be returned to indicate the error.
            s = posix_spawn_file_actions_init(&file_actions);
            if (s != 0)
                errExitEN(s, "posix_spawn_file_actions_init");

            // int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *
            // file_actions, int fildes);
            // STDOUT_FILENO ： Standard output
            // The posix_spawn_file_actions_addclose() function shall add a close action
            // to the object referenced by file_actions that shall cause the file descriptor
            // fildes to be closed (as if close(fildes) had been called) when a new
            // process is spawned using this file actions object.
            s = posix_spawn_file_actions_addclose(&file_actions,
                                                         STDOUT_FILENO);
            if (s != 0)
                errExitEN(s, "posix_spawn_file_actions_addclose");

            file_actionsp = &file_actions;
            break;

        case 's':       /* -s: block all signals in child */

            /* Create an attributes object and add a "set signal mask"
               action to it. */

            s = posix_spawnattr_init(&attr);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_init");

            // POSIX_SPAWN_SETSIGMASK
            // Set the signal mask to the signal set specified in the
            // spawn-sigmask attribute of the object pointed to by attrp.
            s = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_setflags");

            sigfillset(&mask);
            s = posix_spawnattr_setsigmask(&attr, &mask);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_setsigmask");

            attrp = &attr;
            break;
        }
    }

    /* Spawn the child. The name of the program to execute and the
       command-line arguments are taken from the command-line arguments
       of this program. The environment of the program execed in the
       child is made the same as the parent's environment. */

    s = posix_spawnp(&child_pid, argv[optind], file_actionsp, attrp,
                     &argv[optind], environ);

    // The variable optind is the index of the next element to be processed in argv.
    // (getopt)          
    if (s != 0)
        errExitEN(s, "posix_spawn");

    /* Destroy any objects that we created earlier. */

    if (attrp != NULL) {
        s = posix_spawnattr_destroy(attrp);
        if (s != 0)
            errExitEN(s, "posix_spawnattr_destroy");
    }

    if (file_actionsp != NULL) {
        s = posix_spawn_file_actions_destroy(file_actionsp);
        if (s != 0)
            errExitEN(s, "posix_spawn_file_actions_destroy");
    }

    printf("PID of child: %jd\n", (intmax_t) child_pid);

    /* Monitor status of the child until it terminates. */

    do {
        s = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
        if (s == -1)
            errExit("waitpid");

        printf("Child status: ");
        if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    exit(EXIT_SUCCESS);
}