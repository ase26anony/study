/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>

/* Pattern 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Critical pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Pattern 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Critical pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += 2;  /* Simple non-empty body */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;
}

/* Pattern 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int n) {
    register int counter;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize counter */
    counter = n;
    
    /* Critical pattern: do { ... } while (--counter); */
    if (counter > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;  /* Simple non-empty body */
        } while (--counter);
    }
    
    /* Use counter after loop to prevent elimination */
    return sum + counter;
}

/* Pattern 4: Alternative signed decrement with explicit check */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Critical pattern: j = N; while (j != 0) { ... j--; } */
    j = n;
    while (j != 0) {
        sum += 4;
        j--;  /* Decrement in body, not in condition */
    }
    
    /* Use j after loop to prevent elimination */
    return sum + j;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions to ensure coverage */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_do_while(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    return 0;
}
