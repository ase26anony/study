/* loop-doloop-patterns.c
 * Test program to trigger specific decrementing loop patterns in GCC's doloop optimization.
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-patterns.c
 * Then examine the .rtl-doloop dump file for the pattern:
 *   (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_for_neq_zero(int iterations) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = iterations; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;  /* i is 0 here, but ensures i is live after loop */
}

/* Function 2: Unsigned decrementing for loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_for_neq_zero(unsigned int iterations) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = iterations; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* Ensure u is used */
}

/* Function 3: Signed do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_signed_do_while(int iterations) {
    register int n;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    n = iterations;  /* Initialize counter */
    
    /* Pattern: do { ... } while (--n); */
    if (n > 0) {  /* Ensure loop executes at least once for valid pattern */
        do {
            sum += 3;
        } while (--n);  /* Critical: pre-decrement in condition */
    }
    
    return sum + n;  /* Ensure n is used */
}

/* Function 4: Alternative signed for loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed_for(int iterations) {
    register int j;
    volatile int sum = 0;
    int array[100];  /* Small array for indexing */
    
    /* Pattern: for (int j = N; j != 0; j--) with array access */
    for (j = iterations; j != 0; j--) {
        array[j % 100] = j;  /* Use index in body */
        sum += array[j % 100];
    }
    
    return sum + j;
}

int main(int argc, char **argv) {
    int total = 0;
    int base_iterations = 100;  /* Moderate, compile-time constant */
    
    /* Call each test function with different iteration counts
       to ensure all execution paths are taken */
    total += test_signed_for_neq_zero(base_iterations);
    total += test_unsigned_for_neq_zero(base_iterations);
    total += test_signed_do_while(base_iterations);
    total += test_alternative_signed_for(base_iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return total == 0 ? 1 : 0;  /* Non-zero return if all loops executed */
}
