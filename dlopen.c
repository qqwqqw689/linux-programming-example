#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(int argc, char **argv)
{
    void *handle;
    double (*cosine)(double);
    char *error;
    // The function dlopen() loads the dynamic library file.
    // RTLD_LAZY : Perform lazy binding.
    // The function dlerror() returns a human readable 
    // string describing the most recent error that occurred
   handle = dlopen("libm.so.6", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

   dlerror();    /* Clear any existing error */


    // The function dlsym() takes a "handle" of a dynamic library returned 
    // by dlopen() and the null-terminated symbol name, returning the address
    // where that symbol is loaded into memory.
    cosine = (double (*)(double)) dlsym(handle, "cos");

   if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    printf("%f\n", (*cosine)(2.0));
    dlclose(handle);
    exit(EXIT_SUCCESS);
}