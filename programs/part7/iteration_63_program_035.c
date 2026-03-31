/* loop-doloop-test.c
 * Test program to trigger specific doloop pattern in GCC RTL.
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-test.c
 * Then examine loop-doloop-test.c.*r.doloop for the pattern:
 * (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u should be 0 here */
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Initialize to positive value */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;
        } while (--count);  /* Pre-decrement in condition */
    }
    
    return sum + count;  /* count should be 0 here */
}

/* Test 4: Another variant with different iteration count */
__attribute__((noinline, noclone))
int test_alternate_signed(int n) {
    register int j;
    volatile int arr[2] = {0, 0};  /* Use array to prevent optimizations */
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[j % 2] += j;  /* Simple operation using index */
    }
    
    return arr[0] + arr[1] + j;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternate_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return non-zero if any test failed (all should return positive values) */
    return (total == 0) ? 1 : 0;
}
