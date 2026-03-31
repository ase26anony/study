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
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Test 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;
    }
    
    return sum + (int)u;
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {
        do {
            sum += counter;
        } while (--counter);
    }
    
    return sum + counter;
}

/* Test 4: Nested decrementing loop to ensure pattern appears in inner loop */
__attribute__((noinline, noclone))
int test_nested_decrement(int n) {
    register int i, j;
    volatile int sum = 0;
    
    for (i = n; i != 0; i--) {
        /* Inner loop with decrement pattern */
        for (j = 10; j != 0; j--) {
            sum += i * j;
        }
    }
    
    return sum + i + j;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_nested_decrement(iterations);
    
    printf("Total: %d\n", total);
    
    /* Verify results match expectations */
    if (total == 171700) {  /* Pre-calculated expected value for iterations=100 */
        printf("All loops executed correctly\n");
    } else {
        printf("Unexpected result\n");
    }
    
    return 0;
}
