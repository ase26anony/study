/* test_doloop_coverage.c
 * Compile with: gcc -O2 -fdoloop -fdump-rtl-doloop -c test_doloop_coverage.c
 * Also try: gcc -O3 -funroll-loops -fno-peel-loops -fno-move-loop-invariants -c test_doloop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop transformations by using volatile parameters */
#define NOINLINE __attribute__((noinline, noclone))

/* Test function 1: Basic for loop with decrement and explicit compare to zero */
NOINLINE int test_for_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    /* This should generate: for (i = n; i != 0; i--) */
    for (int i = n; i != 0; i--) {
        /* Simple side effect using the loop counter */
        sink += i;
        result ^= i;
        
        /* Prevent optimization of loop counter */
        asm volatile("" : : "r"(i));
    }
    return result + sink;
}

/* Test function 2: While loop with post-decrement in condition */
NOINLINE int test_while_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    int i = n;
    
    /* This should generate: while (i--) { ... } */
    while (i--) {
        sink += i;
        result |= i;
        asm volatile("" : : "r"(i));
    }
    return result + sink;
}

/* Test function 3: For loop with unsigned counter */
NOINLINE int test_unsigned_loop(volatile unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int result = 0;
    
    /* Unsigned decrement to zero */
    for (unsigned int i = n; i != 0; --i) {
        sink += i;
        result &= i;
        asm volatile("" : : "r"(i));
    }
    return (int)(result + sink);
}

/* Test function 4: Explicit decrement with subtraction */
NOINLINE int test_explicit_decrement(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    int i = n;
    
    /* Using explicit subtraction instead of i-- */
    while (i != 0) {
        sink += i;
        result += i * i;
        asm volatile("" : : "r"(i));
        i = i - 1;  /* Explicit subtraction */
    }
    return result + sink;
}

/* Test function 5: Nested loops to increase analysis complexity */
NOINLINE int test_nested_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    /* Outer loop with volatile bound */
    for (int outer = 3; outer != 0; outer--) {
        /* Inner loop with decrement pattern */
        for (int i = n; i != 0; i--) {
            sink += i * outer;
            result ^= (i + outer);
            asm volatile("" : : "r"(i));
        }
    }
    return result + sink;
}

/* Test function 6: Loop with array access using volatile index */
NOINLINE int test_array_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    int array[100];
    
    /* Initialize array */
    for (int j = 0; j < 100; j++) {
        array[j] = j;
    }
    
    /* Decrementing loop with array access */
    for (int i = n; i > 0; i--) {
        /* Use modulo to prevent out-of-bounds access */
        int idx = i % 97;
        sink += array[idx];
        array[idx] = i;
        result += array[idx];
        asm volatile("" : : "r"(i));
    }
    return result + sink;
}

/* Test function 7: Loop with __builtin_expect hint */
NOINLINE int test_expected_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    int i = n;
    
    while (__builtin_expect(i != 0, 1)) {
        sink += i;
        result -= i;
        asm volatile("" : : "r"(i));
        i--;
    }
    return result + sink;
}

/* Architecture-specific test for ARM (hardware loop support) */
#ifdef __ARM_ARCH
NOINLINE int test_arm_hardware_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    /* Use pragma to encourage loop optimization */
    #pragma GCC optimize("O2")
    for (int i = n; i != 0; i--) {
        sink += i;
        result |= i;
        
        /* ARM-specific asm to hint at hardware loops */
        #ifdef __ARM_ARCH
        asm volatile("" : : "r"(i) : "cc");
        #endif
    }
    return result + sink;
}
#endif

/* Main function with multiple test iterations */
int main(void) {
    volatile int checksum = 0;
    
    /* Test with different bounds to increase coverage chance */
    volatile int bounds[] = {100, 500, 1000, 1, 10};
    
    for (int b = 0; b < 5; b++) {
        volatile int n = bounds[b];
        
        checksum += test_for_loop(n);
        checksum += test_while_loop(n);
        checksum += test_unsigned_loop((unsigned int)n);
        checksum += test_explicit_decrement(n);
        checksum += test_nested_loop(n);
        checksum += test_array_loop(n);
        checksum += test_expected_loop(n);
        
        #ifdef __ARM_ARCH
        checksum += test_arm_hardware_loop(n);
        #endif
        
        /* Add some variation by using different comparison patterns */
        if (n > 0) {
            checksum += test_for_loop(n - 1);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
