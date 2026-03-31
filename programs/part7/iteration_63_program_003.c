/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i = n;  /* Hint for register allocation */
    volatile int sum = 0; /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1; /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u = n;
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter = n;
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {
        do {
            sum += 3;
        } while (--counter);
    }
    
    return sum + counter;
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j = n;
    volatile int sum = 0;
    
    /* Another variant that should generate the same pattern */
    for (j = n; j != 0; --j) {
        sum += 4;
    }
    
    return sum + j;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int total = 0;
    
    /* Execute all test functions to ensure coverage */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    return 0;
}
