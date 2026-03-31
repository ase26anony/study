/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u should be 0 here */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;
        } while (--counter);
    }
    
    return sum + counter;  /* counter should be 0 here */
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Another variant of the pattern */
    j = n;
    while (j != 0) {
        sum += 4;
        j--;  /* Decrement in loop body, but still simple */
    }
    
    return sum + j;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 100;
        }
    }
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return 0 for success */
    return 0;
}
