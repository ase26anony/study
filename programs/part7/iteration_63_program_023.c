/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;
    }
    
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {
        do {
            sum += 3;
        } while (--count);
    }
    
    return sum + count;
}

/* Function 4: Nested loop to test different context */
__attribute__((noinline, noclone))
int test_nested_decrement(int n) {
    register int outer;  /* Hint to keep in register */
    register int inner;  /* Hint to keep in register */
    volatile int sum = 0;
    
    /* Outer loop */
    for (outer = n; outer != 0; outer--) {
        /* Inner loop with same pattern */
        for (inner = 5; inner != 0; inner--) {
            sum += 4;
        }
    }
    
    return sum + outer + inner;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int total = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_nested_decrement(iterations / 10);  /* Smaller for nested */
    
    printf("Total: %d\n", total);
    return 0;
}
