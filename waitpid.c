#include <sys/wait.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    pid_t cpid, w;
    int wstatus;
    cpid = fork();
    // fork - create a child process
    // On success, the PID of the child process is returned in the parent, and 0 is returned in the child.
    // On failure, -1 is returned in the parent, no child process is created
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid == 0) {            /* Code executed by child */
        printf("Child PID is %jd\n", (intmax_t) getpid());
        // getpid() returns the process ID (PID) of the calling process.
        // intmax_t 	maximum width integer type 
        if (argc == 1)
            pause();
            // pause() causes the calling process (or thread) to sleep until a
            // signal is delivered that either terminates the process or causes
            // the invocation of a signal-catching function.
        _exit(atoi(argv[1]));
        // _exit() terminates the calling process "immediately". 
        // int atoi (const char * str);
        // Convert string to integer
    } else {                    /* Code executed by parent */
        do {
            w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
            // pid_t waitpid(pid_t pid, int *stat_loc, int options);
            // wait for a child process to stop or terminate
            // pid_t : process ID or a process group ID to identify the child process or processes on which waitpid() should operate.
            // stat_loc : Pointer to an area where status information about how the child process ended is to be placed.
            // options : An integer field containing flags that define how waitpid() should operate.
            // WUNTRACED : return if a child has stopped
            // WCONTINUED : return if a stopped child has been resumed by delivery of SIGCONT
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            // WIFEXITED(status)
            // returns true if the child terminated normally
            // WEXITSTATUS(status)
            // returns the exit status of the child. 
            // WIFSIGNALED(status)
            // returns true if the child process was terminated by a signal.
            // WTERMSIG(status)
            // returns the number of the signal that caused the child process to terminate. 
            // WIFSTOPPED(status)
            // returns true if the child process was stopped by delivery of a signal.
            // WSTOPSIG(status)
            // returns the number of the signal which caused the child to stop.
            // WIFCONTINUED(status)
            // returns true if the child process was resumed by delivery of SIGCONT.
            if (WIFEXITED(wstatus)) {
                printf("exited, status=%d\n", WEXITSTATUS(wstatus));
            } else if (WIFSIGNALED(wstatus)) {
                printf("killed by signal %d\n", WTERMSIG(wstatus));
            } else if (WIFSTOPPED(wstatus)) {
                printf("stopped by signal %d\n", WSTOPSIG(wstatus));
            } else if (WIFCONTINUED(wstatus)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
        exit(EXIT_SUCCESS);
    }
}

/*
& ：This symbol is used to put the command in the background
-STOP : SIGSTOP signal
-CONT : SIGCONT signal
-TERM : SIGTERM signal
 $      ./a.out &
           Child PID is 32360
           [1] 32359
           $ kill -STOP 32360
           stopped by signal 19
           $ kill -CONT 32360
           continued
           $ kill -TERM 32360
           killed by signal 15
           [1]+  Done                    ./a.out
           $
*/