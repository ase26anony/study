/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;  /* Different body to distinguish from first */
    }
    
    /* Use u after loop */
    return sum + (u == 0 ? 1 : 0);
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure loop executes at least once for positive n */
        do {
            sum += 3;  /* Different body */
        } while (--counter);
    }
    
    /* Use counter after loop */
    return sum + (counter == 0 ? 1 : 0);
}

/* Function 4: Nested loop to test different context */
__attribute__((noinline, noclone))
int test_nested_loop(int n) {
    register int i, j;
    volatile int sum = 0;
    
    /* Outer loop with decrement */
    for (i = n; i != 0; i--) {
        /* Inner loop with different counter */
        for (j = 5; j != 0; j--) {
            sum += i + j;
        }
    }
    
    /* Use counters after loop */
    return sum + (i == 0 ? 1 : 0) + (j == 0 ? 1 : 0);
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Call all test functions to ensure execution */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_nested_loop(10);  /* Smaller for nested */
    
    printf("Total: %d\n", total);
    return 0;
}
