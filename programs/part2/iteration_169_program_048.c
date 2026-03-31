/* Test file with intentional warnings and errors */
#include <stdio.h>

/* Deliberate warning: unused parameter */
int unused_param(int x) {
    return 0;
}

/* Deliberate warning: implicit declaration */
void implicit_declaration_test() {
    printf("Test: %d\n", undefined_function());  /* Will cause warning */
}

int main(int argc, char **argv) {
    /* Conditional compilation to create different preprocessing outputs */
    #ifdef SPECIAL_DEFINE
    printf("Special define is set\n");
    #endif
    
    /* Another warning: unused variable */
    int unused = 42;
    
    /* Return success */
    return 0;
}
