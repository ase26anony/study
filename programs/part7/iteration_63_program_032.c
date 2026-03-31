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
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;  /* i should be 0 here */
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;  /* u should be 0 here */
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += counter;
        } while (--counter);
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + counter;  /* counter should be 0 here */
}

/* Test 4: Another variant with different iteration count */
__attribute__((noinline, noclone))
int test_variant(int n) {
    register int j;  /* Hint for register allocation */
    volatile int arr[2] = {0, 0};
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[j & 1] += j;  /* Simple array access using index */
    }
    
    /* Use j after loop to prevent elimination */
    return arr[0] + arr[1] + j;  /* j should be 0 here */
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_variant(iterations);
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    return 0;
}
