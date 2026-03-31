/* test.c - Contains intentional warnings and errors for testing */
#include <stdio.h>

/* Deliberate warning: unused parameter */
int unused_function(int unused_param) {
    return 0;
}

/* Function with potential warning */
int potentially_uninitialized() {
    int x;  /* May be used uninitialized */
    if (0) {
        x = 5;
    }
    return x;  /* Warning: x may be used uninitialized */
}

/* Main function with syntax error in comment */
int main(void) {
    printf("Test program\n");
    
    /* Trigger -Werror if enabled */
    int y = potentially_uninitialized();
    
    return 0;
}

/* Additional code to create multiple compilation phases */
#ifdef PREPROCESS_ONLY
#warning "Preprocessing only"
#endif

#ifdef ASSEMBLE_ONLY
    asm volatile ("nop");
#endif
