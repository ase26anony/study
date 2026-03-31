/* Test program for doloop pattern: (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int iterations) {
    register int i;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = iterations; i != 0; i--) {
        sum += 1;  /* Simple non-empty body */
    }
    
    /* Use i in return to prevent elimination */
    return sum + (i == 0 ? 1 : 0);
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int iterations) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = iterations; u != 0; u--) {
        sum += 2;  /* Different constant to distinguish from Function 1 */
    }
    
    /* Use u in return to prevent elimination */
    return sum + (u == 0 ? 1 : 0);
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_do_while(int iterations) {
    register int n;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Initialize n with iterations */
    n = iterations;
    
    /* Pattern: do { ... } while (--n); */
    if (n > 0) {  /* Ensure we enter the loop at least once */
        do {
            sum += 3;  /* Different constant to distinguish */
        } while (--n);  /* Pre-decrement and test against zero */
    }
    
    /* Use n in return to prevent elimination */
    return sum + (n == 0 ? 1 : 0);
}

/* Function 4: Alternative signed decrement with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int iterations) {
    register int counter;  /* Hint for register allocation */
    volatile int accumulator = 0;
    
    /* Pattern with explicit comparison */
    counter = iterations;
    while (counter != 0) {
        accumulator += 4;
        counter--;  /* Post-decrement in loop body */
    }
    
    /* Ensure counter is used */
    return accumulator + (counter == 0 ? 1 : 0);
}

int main(void) {
    int total = 0;
    const int base_iterations = 100;
    
    /* Call all test functions with different iteration counts
       to ensure all execution paths are taken */
    total += test_signed_decrement(base_iterations);
    total += test_unsigned_decrement(base_iterations);
    total += test_do_while(base_iterations);
    total += test_alternative_signed(base_iterations);
    
    /* Also test with small iteration counts */
    total += test_signed_decrement(5);
    total += test_unsigned_decrement(3);
    total += test_do_while(1);
    total += test_alternative_signed(10);
    
    printf("Total: %d\n", total);
    return 0;
}
