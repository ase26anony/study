/* loop-doloop-patterns.c
 * Test program to trigger specific doloop pattern in GCC RTL.
 * Target pattern: (compare (plus reg -1) (const_int 0))
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-doloop -S loop-doloop-patterns.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Signed decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_signed_decrement(int n) {
    register int i = n;  /* Hint for register allocation */
    volatile int sum = 0; /* Prevent elimination of loop body */
    
    /* Pattern: for (int i = N; i != 0; i--) */
    for (i = n; i != 0; i--) {
        sum += 1;  /* Simple, non-empty body */
    }
    
    /* Use i after loop to prevent elimination */
    return sum + i;
}

/* Function 2: Unsigned decrementing loop with != 0 comparison */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u = n;  /* Register hint */
    volatile int accumulator = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        accumulator += 2;
    }
    
    return accumulator + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int counter = n;  /* Must be positive for do-while */
    volatile int result = 0;
    
    if (counter <= 0) return 0;
    
    /* Pattern: do { ... } while (--counter); */
    do {
        result += 3;
    } while (--counter);
    
    return result + counter;
}

/* Function 4: Alternative signed loop with explicit != 0 */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j = n;
    volatile int total = 0;
    int dummy_array[1];  /* Simple array for indexing */
    
    /* Pattern with array access in body */
    for (j = n; j != 0; j--) {
        dummy_array[0] = total;
        total += 4;
    }
    
    return total + j;
}

int main(void) {
    int total = 0;
    const int iterations = 100;
    
    /* Execute all test functions */
    total += test_signed_decrement(iterations);
    total += test_unsigned_decrement(iterations);
    total += test_dowhile_decrement(iterations);
    total += test_alternative_signed(iterations);
    
    printf("Total: %d\n", total);
    
    /* Validate with a simple check */
    if (total == (100*1 + 100*2 + 100*3 + 100*4)) {
        printf("All loops executed correctly\n");
    } else {
        printf("Unexpected result\n");
    }
    
    return 0;
}
