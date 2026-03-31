/* loop-doloop-patterns.c
 * Test program to trigger specific RTL pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-patterns.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    volatile int sum = 0;
    register int i;  /* Hint to keep counter in register */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + i;  /* i is now 0, but ensures counter is used */
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
unsigned int test_unsigned_decrement(unsigned int n) {
    volatile unsigned int sum = 0;
    register unsigned int u;  /* Hint to keep counter in register */
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += u % 10;  /* Simple operation using counter */
    }
    
    /* Use counter after loop */
    return sum + u;  /* u is now 0 */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    volatile int sum = 0;
    register int count = n;  /* Explicit counter variable */
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 1;
        } while (--count);  /* Pre-decrement in condition */
    }
    
    /* Use counter after loop */
    return sum + count;  /* count is now 0 or negative */
}

/* Function 4: Another variant with different body */
__attribute__((noinline, noclone))
int test_array_access(int n) {
    volatile int arr[10] = {0};
    register int i;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        arr[i % 10] += i;  /* Array access using counter */
    }
    
    /* Use counter and array */
    return arr[0] + i;
}

int main(int argc, char *argv[]) {
    int total = 0;
    int iterations = 100;  /* Compile-time constant for loop bounds */
    
    /* Call all test functions to ensure they're executed */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_array_access(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (simplistic check) */
    return (total == 0) ? 1 : 0;
}
