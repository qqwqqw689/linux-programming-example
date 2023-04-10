// https://man7.org/linux/man-pages/man3/pthread_create.3.html

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
 
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0) // 1
// The perror() function prints an error message to stderr
// errno - number of last error
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0) // 2

struct thread_info {    /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
    char     *argv_string;      /* From command-line argument */
};

/* Thread start function: display address near top of our stack,
   and return upper-cased copy of argv_string. */

static void *
thread_start(void *arg)
{
    struct thread_info *tinfo = (thread_info*)arg;
    char *uargv;

    printf("Thread %d: top of stack near %p; argv_string=%s\n", tinfo->thread_num, (void *) &tinfo, tinfo->argv_string);
    uargv = strdup(tinfo->argv_string);
    // char * strdup( const char *str1 ) : 
    // Returns a pointer to a null-terminated byte string, which is a duplicate of the string pointed to by str1. 
    if (uargv == NULL)
        handle_error("strdup"); // 2
    for (char *p = uargv; *p != '\0'; p++)
        *p = toupper(*p);
    // toupper - Convert lowercase letter to uppercase
    return uargv;
}

int
main(int argc, char *argv[])
{
    // argc : the number of arguments passed to the program
    // argv : Pointer to the first element of an array of argc + 1 pointers
    int s, opt, num_threads;
    pthread_attr_t attr;
    ssize_t stack_size;
    void *res;

     /* The "-s" option specifies a stack size for our threads. */

    stack_size = -1;
    while ((opt = getopt(argc, argv, "s:")) != -1) {
        // int getopt(int argc, char * const argv[], const char *optstring);
        /*
        * An element of argv that starts with '-' is an option element
        * optstring is a string containing the legitimate option characters. 
        * If such a character is followed by a colon, the option requires an argument, 
        * so getopt() places a pointer to the following text in the same argv-element, 
        * or the text of the following argv-element in optarg.
        * If an option was successfully found, then getopt() returns the option character. 
        * If all command-line options have been parsed, then getopt() returns -1.
        * the next call to getopt() can resume the scan with the following option character or argv-element
        */
        switch (opt) {
        case 's':
            stack_size = strtoul(optarg, NULL, 0);
            // unsigned long int strtoul (const char* str, char** endptr, int base);
            // Convert string to unsigned long integer
            break;

        default:
            fprintf(stderr, "Usage: %s [-s stack-size] arg...\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

           num_threads = argc - optind;
           // The variable optind is the index of the next element to be processed in argv.

           /* Initialize thread creation attributes. */

           s = pthread_attr_init(&attr);
           // int pthread_attr_init(pthread_attr_t *attr);
           // initializes the thread attributes object pointed to by attr with default attribute
           // values. 
           // On success, return 0;
           if (s != 0)
               handle_error_en(s, "pthread_attr_init");

           if (stack_size > 0) {
               s = pthread_attr_setstacksize(&attr, stack_size);
               // int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
               // sets the stack size
               // On success, return 0;
               if (s != 0)
                   handle_error_en(s, "pthread_attr_setstacksize");
           }

           /* Allocate memory for pthread_create() arguments. */

           struct thread_info *tinfo = (thread_info*)calloc(num_threads, sizeof(*tinfo));
           // void *calloc( size_t num, size_t size );
           // Allocates memory for an array of num objects of size and 
           // initializes all bytes in the allocated storage to zero. 
           if (tinfo == NULL)
               handle_error("calloc");

           /* Create one thread for each command-line argument. */

           for (int tnum = 0; tnum < num_threads; tnum++) {
               tinfo[tnum].thread_num = tnum + 1;
               tinfo[tnum].argv_string = argv[optind + tnum];

               /* The pthread_create() call stores the thread ID into
                  corresponding element of tinfo[]. */

               s = pthread_create(&tinfo[tnum].thread_id, &attr,
                                  &thread_start, &tinfo[tnum]);
               if (s != 0)
                   handle_error_en(s, "pthread_create");
           }

           /* Destroy the thread attributes object, since it is no
              longer needed. */

           s = pthread_attr_destroy(&attr);
           if (s != 0)
               handle_error_en(s, "pthread_attr_destroy");

           /* Now join with each thread, and display its returned value. */

           for (int tnum = 0; tnum < num_threads; tnum++) {
               s = pthread_join(tinfo[tnum].thread_id, &res);
               if (s != 0)
                   handle_error_en(s, "pthread_join");

               printf("Joined with thread %d; returned value was %s\n",
                       tinfo[tnum].thread_num, (char *) res);
               free(res);      /* Free memory allocated by thread */
           }

           free(tinfo);
           exit(EXIT_SUCCESS);
       }