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
    return sum + i;  /* i should be 0 here */
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Register hint */
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
    register int counter;  /* Register hint */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;
        } while (--counter);  /* Pre-decrement in condition */
    }
    
    return sum + counter;  /* counter should be 0 here */
}

/* Test 4: Another variant with different iteration count */
__attribute__((noinline, noclone))
int test_alternate_pattern(int n) {
    register int j;
    volatile int arr[2] = {0, 0};  /* Use array to prevent optimizations */
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[j % 2] += j;  /* Simple array access using index */
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
    total += test_alternate_pattern(iterations);
    
    printf("Total: %d\n", total);
    
    /* Verify expected results */
    int expected = (iterations * 1) +      /* test_signed_decrement */
                   (iterations * 2) +      /* test_unsigned_decrement */
                   (iterations * 3) +      /* test_dowhile_decrement */
                   (4950 + 5000);          /* test_alternate_pattern (sum of 1..100) */
    
    if (total == expected) {
        printf("All loops executed correctly.\n");
    } else {
        printf("Unexpected result: got %d, expected %d\n", total, expected);
    }
    
    return 0;
}
