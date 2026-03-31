/* loop-doloop-test.c
 * Test program to trigger doloop optimization pattern:
 * (compare (plus reg -1) (const_int 0))
 * 
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-test.c
 * Check doloop dump for the pattern in each test function.
 */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Register hint */
    volatile int accumulator = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        accumulator += 2;
    }
    
    /* Ensure u is used */
    return accumulator + (u == 0 ? 1 : 0);
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter = n;  /* Start with positive value */
    volatile int result = 0;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure at least one iteration */
        do {
            result += 3;
        } while (--counter);
    }
    
    return result + (counter == 0 ? 1 : 0);
}

/* Test 4: Another variant with different iteration count */
__attribute__((noinline, noclone))
int test_alternate_decrement(int n) {
    register int j = n;
    volatile int total = 0;
    int array[1];  /* Simple array for indexing */
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        array[0] = j;  /* Use index in body */
        total += array[0] & 1;  /* Simple computation */
    }
    
    return total + (j == 0 ? 1 : 0);
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternate_decrement(iterations);
    
    printf("Total: %d\n", total);
    
    /* Verify expected total:
     * test_signed_decrement: 100 * 1 + 1 = 101
     * test_unsigned_decrement: 100 * 2 + 1 = 201
     * test_dowhile_decrement: 100 * 3 + 1 = 301
     * test_alternate_decrement: (sum of 100 odd/even) + 1 ≈ 50 + 1 = 51
     * Total ≈ 654
     */
    
    return 0;
}
