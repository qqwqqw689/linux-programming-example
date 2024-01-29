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

            // int posix_spawnattr_init(posix_spawnattr_t *attr);
            // The posix_spawnattr_init() function shall initialize
            // a spawn attributes object attr with the default value for all
            // of the individual attributes used by the implementation.
            s = posix_spawnattr_init(&attr);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_init");

            // POSIX_SPAWN_SETSIGMASK
            // Set the signal mask to the signal set specified in the
            // spawn-sigmask attribute of the object pointed to by attrp.

            // int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);
            // The posix_spawnattr_setflags() function shall set the spawn-flags attribute
            // in an initialized attributes object referenced by attr.
            // The spawn-flags attribute is used to indicate which process attributes are
            // to be changed in the new process image when invoking posix_spawn()
            s = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_setflags");
            
            // int sigfillset(sigset_t *set);
            // initialize and fill a signal set
            // all signals defined in this volume of POSIX.1‐2017 are included
            sigfillset(&mask);

            // int posix_spawnattr_setsigmask(posix_spawnattr_t *restrict attr,
            // const sigset_t *restrict sigmask);
            // The posix_spawnattr_setsigmask() function shall set the spawn-sigmask
            // attribute in an initialized attributes object referenced by attr.
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
    // intmax_t: Integer type with the maximum width supported.

    /* Monitor status of the child until it terminates. */

    do {
        s = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
        // pid_t waitpid(pid_t pid, int *status, int options);
        // wait for state changes in a child of the calling process,
        // and obtain information about the child whose state has changed.
        // WUNTRACED: return if a child has stopped
        // WCONTINUED：return if a stopped child has been resumed by delivery of SIGCONT.
        // SIGCONT tells LINUX to resume the process paused earlier
        if (s == -1)
            errExit("waitpid");

        printf("Child status: ");

        // WIFEXITED: determines whether the child process ended normally.
        // WEXITSTATUS:  If the WIFEXITED macro indicates that the child process exited normally,
        // the WEXITSTATUS macro returns the exit code specified by the child process.
        // WIFSIGNALED: It determines if the child process exited because it raised a signal
        // that caused it to exit.
        // WTERMSIG: This macro queries the termination status of a child process
        // to determine which signal caused the child process to exit.
        // WIFSTOPPED: This macro evaluates to a nonzero (true) value 
        // if the child process is currently stopped.
        // WSTOPSIG: this macro returns the signal number of the signal that caused
        // the child process to stop.
        // WIFCONTINUED: returns true if the child process was resumed by delivery of SIGCONT.
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