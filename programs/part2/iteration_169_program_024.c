/* test.c - Contains intentional warnings for testing error recovery */
#include <stdio.h>

int unused_variable;  /* Deliberate warning: unused variable */

int main(int argc, char **argv) {
    /* Deliberate warning: unused parameter */
    printf("Test program\n");
    
    /* Conditional compilation to avoid syntax errors */
    #ifdef DELIBERATE_ERROR
    This will cause a compilation error
    #endif
    
    return 0;
}
