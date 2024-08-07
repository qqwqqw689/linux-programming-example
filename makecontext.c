#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

static ucontext_t uctx_main, uctx_func1, uctx_func2;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void
func1(void)
{
    printf("%s: started\n", __func__);
    printf("%s: swapcontext(&uctx_func1, &uctx_func2)\n", __func__);
    if (swapcontext(&uctx_func1, &uctx_func2) == -1)
        handle_error("swapcontext");
    printf("%s: returning\n", __func__);
}

static void
func2(void)
{
    printf("%s: started\n", __func__);
    printf("%s: swapcontext(&uctx_func2, &uctx_func1)\n", __func__);
    if (swapcontext(&uctx_func2, &uctx_func1) == -1)
        handle_error("swapcontext");
    printf("%s: returning\n", __func__);
}

int
main(int argc, char *argv[])
{
    char func1_stack[16384];
    char func2_stack[16384];
    // int getcontext(ucontext_t *ucp);
    // The function getcontext() initializes the structure pointed to by
    // ucp to the currently active context.
    if (getcontext(&uctx_func1) == -1)
        handle_error("getcontext");
    // uc_stack : Stack used for this context.
    // ss_sp :  point to the base of the memory region allocated for the stack
    // ss_size :  the size of the memory region
    uctx_func1.uc_stack.ss_sp = func1_stack;
    uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
    uctx_func1.uc_link = &uctx_main;
    // uctx_main : points to the context that will be resumed
    // when the current context terminate

    // void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
    // argc : the number of arguments.
    makecontext(&uctx_func1, func1, 0);

    if (getcontext(&uctx_func2) == -1)
        handle_error("getcontext");
    uctx_func2.uc_stack.ss_sp = func2_stack;
    uctx_func2.uc_stack.ss_size = sizeof(func2_stack);
    /* Successor context is f1(), unless argc > 1 */
    uctx_func2.uc_link = (argc > 1) ? NULL : &uctx_func1;

    makecontext(&uctx_func2, func2, 0);
    printf("%s: swapcontext(&uctx_main, &uctx_func2)\n", __func__);
    // On error, swapcontext() returns -1 and sets errno appropriately.
    if (swapcontext(&uctx_main, &uctx_func2) == -1)
        handle_error("swapcontext");

    printf("%s: exiting\n", __func__);
    exit(EXIT_SUCCESS);
}