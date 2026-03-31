/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
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
    register int counter;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure loop executes at least once for positive n */
        do {
            sum += 3;
        } while (--counter);
    }
    
    return sum + counter;  /* counter should be 0 here */
}

/* Function 4: Alternative signed loop with explicit register */
__attribute__((noinline, noclone))
int test_alt_signed(int n) {
    register int j asm("r12");  /* Stronger register hint */
    volatile int arr[10];  /* Use array to create simple body */
    int k;
    
    /* Initialize array element to prevent optimization */
    for (k = 0; k < 10; k++) arr[k] = 0;
    
    /* Pattern: for (register int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[j % 10] = j;  /* Simple array access using counter */
    }
    
    /* Use j after loop */
    return arr[0] + j;  /* j should be 0 here */
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_alt_signed(iterations);
    
    printf("Total: %d\n", total);
    
    return total != 0 ? 0 : 1;  /* Non-zero exit if all loops were eliminated */
}
