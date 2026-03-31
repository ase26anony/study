/* loop-doloop-test.c - Test program for doloop optimization pattern matching */

#include <stdio.h>
#include <stdlib.h>

/* Test 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    volatile int sum = 0;
    register int i;  /* Hint to keep in register */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += i;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Test 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    volatile int sum = 0;
    register unsigned int u;  /* Hint to keep in register */
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;
}

/* Test 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    volatile int sum = 0;
    register int counter;  /* Hint to keep in register */
    
    /* Pattern: do { ... } while (--counter); */
    counter = n;
    if (counter > 0) {  /* Ensure loop executes at least once for do-while */
        do {
            sum += counter;
        } while (--counter);
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + counter;
}

/* Test 4: Another variant with different body */
__attribute__((noinline, noclone))
int test_array_access(int n) {
    volatile int arr[100];
    register int i;
    
    /* Initialize array */
    for (int j = 0; j < 100; j++) {
        arr[j] = j;
    }
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        arr[i % 100] = i;  /* Simple array access */
    }
    
    /* Use i after loop to prevent elimination */
    return arr[0] + i;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure they're not eliminated */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_array_access(iterations);
    
    printf("Total: %d\n", total);
    
    return 0;
}
