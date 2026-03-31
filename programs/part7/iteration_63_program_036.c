/* loop-doloop-test.c
 * Test program to trigger the specific decrementing loop pattern in GCC's doloop optimization.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep counter in register */
    volatile int sum = 0;  /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use counter in return to prevent elimination */
    return sum + i;  /* i is 0 after loop */
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep counter in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;  /* u is 0 after loop */
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Initialize to positive value */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {  /* Ensure at least one iteration */
        do {
            sum += 3;
        } while (--count);
    }
    
    return sum + count;  /* count is 0 after loop */
}

/* Function 4: Additional variant with array access */
__attribute__((noinline, noclone))
int test_array_access(int n) {
    register int j;
    volatile int arr[10] = {0};  /* Small array to avoid stack issues */
    int idx = 0;
    
    /* Pattern: for (int j = N; j != 0; j--) */
    for (j = n; j != 0; j--) {
        arr[idx % 10] = j;  /* Use counter in array access */
        idx++;
    }
    
    return j + arr[0];  /* j is 0 after loop */
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    /* Use command line argument if provided, otherwise default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_array_access(iterations);
    
    printf("Total: %d\n", total);
    
    return 0;
}
