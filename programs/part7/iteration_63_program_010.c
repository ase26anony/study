/* Test program to trigger doloop pattern (compare (plus reg -1) (const_int 0)) */
#include <stdio.h>

/* Function 1: Basic signed decrementing loop with != 0 comparison */
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

/* Function 2: Unsigned decrementing loop */
__attribute__((noinline, noclone))
int test_unsigned_decrement(unsigned int n) {
    register unsigned int u;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: for (unsigned int u = N; u != 0; u--) */
    for (u = n; u != 0; u--) {
        sum += (int)u;  /* Simple non-empty body */
    }
    
    /* Use u after loop to prevent elimination */
    return sum + (int)u;
}

/* Function 3: Do-while loop with pre-decrement */
__attribute__((noinline, noclone))
int test_dowhile_decrement(int n) {
    register int count = n;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: do { ... } while (--count); */
    if (count > 0) {
        do {
            sum += count;
        } while (--count);
    }
    
    /* Use count after loop to prevent elimination */
    return sum + count;
}

/* Function 4: Alternative signed loop with explicit comparison */
__attribute__((noinline, noclone))
int test_alternative_signed(int n) {
    register int j = n;  /* Hint for register allocation */
    volatile int sum = 0;
    
    /* Pattern: while (j != 0) { ... j--; } */
    while (j != 0) {
        sum += j;
        j--;
    }
    
    return sum + j;
}

int main(void) {
    int total = 0;
    
    /* Call all test functions with moderate iteration counts */
    total += test_signed_decrement(100);
    total += test_unsigned_decrement(100);
    total += test_dowhile_decrement(100);
    total += test_alternative_signed(100);
    
    printf("Total: %d\n", total);
    return 0;
}
