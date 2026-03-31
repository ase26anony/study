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
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Test 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    /* Use u after loop */
    return sum + (u == 0 ? 1 : 0);
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter = n;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {
        do {
            sum += 3;
        } while (--counter);
    }
    
    /* Use counter after loop */
    return sum + (counter == 0 ? 1 : 0);
}

/* Test 4: Another variant with different iteration count */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j;
    volatile int arr[2] = {0, 0};
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[j % 2] = j;  /* Simple array access using index */
    }
    
    return arr[0] + arr[1] + (j == 0 ? 1 : 0);
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure code paths are taken */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Return success if total is reasonable */
    return (total > 0) ? 0 : 1;
}
