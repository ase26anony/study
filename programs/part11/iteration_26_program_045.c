/* loop-doloop-coverage.c
 * Designed to trigger GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 of loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop transformations */
#define KEEP_LOOP(x) asm volatile("" : : "r"(x) : "memory")

/* Volatile sink to prevent dead code elimination */
static volatile int global_sink = 0;

/* Test function 1: Basic for loop with decrement and != 0 condition */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Pattern: for (int i = n; i != 0; i--) */
    for (int i = n; i != 0; i--) {
        local_sink += i;
        KEEP_LOOP(i);  /* Prevent optimization */
    }
    
    global_sink += local_sink;
}

/* Test function 2: While loop with decrement in condition */
__attribute__((noinline, noclone, optimize("O2")))
void test_while_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Pattern: while (i--) */
    while (i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    
    global_sink += local_sink;
}

/* Test function 3: For loop with unsigned counter */
__attribute__((noinline, noclone, optimize("O2")))
void test_unsigned_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Pattern: for (unsigned i = n; i != 0; --i) */
    for (unsigned i = n; i != 0; --i) {
        local_sink += (int)i;
        KEEP_LOOP(i);
    }
    
    global_sink += local_sink;
}

/* Test function 4: Explicit decrement with i = i - 1 */
__attribute__((noinline, noclone, optimize("O2")))
void test_explicit_decr(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Pattern: while (i != 0) { i = i - 1; } */
    while (i != 0) {
        local_sink += i;
        i = i - 1;  /* Explicit subtraction instead of i-- */
        KEEP_LOOP(i);
    }
    
    global_sink += local_sink;
}

/* Test function 5: Nested loops to increase analysis complexity */
__attribute__((noinline, noclone, optimize("O2")))
void test_nested_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Outer loop with volatile bound */
    for (int outer = 3; outer != 0; outer--) {
        /* Inner loop with decrement pattern */
        for (int i = n; i != 0; i--) {
            local_sink += i * outer;
            KEEP_LOOP(i);
        }
    }
    
    global_sink += local_sink;
}

/* Test function 6: Loop with > 0 condition instead of != 0 */
__attribute__((noinline, noclone, optimize("O2")))
void test_greater_zero(volatile int n) {
    volatile int local_sink = 0;
    
    /* Pattern: for (int i = n; i > 0; i--) */
    for (int i = n; i > 0; i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    
    global_sink += local_sink;
}

/* Architecture-specific test for ARM hardware loops */
#ifdef __ARM_ARCH
__attribute__((noinline, noclone, optimize("O2")))
void test_arm_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Use __builtin_expect to guide branch prediction */
    if (__builtin_expect(i > 0, 1)) {
        do {
            local_sink += i;
            KEEP_LOOP(i);
        } while (--i != 0);
    }
    
    global_sink += local_sink;
}
#endif

/* MIPS-specific test */
#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    for (int i = n; i != 0; i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    
    global_sink += local_sink;
}
#endif

/* Main driver with varying loop bounds */
int main(void) {
    volatile int bounds[] = {100, 500, 1000, 5000};
    int num_bounds = sizeof(bounds) / sizeof(bounds[0]);
    
    printf("Starting doloop pattern tests...\n");
    
    for (int b = 0; b < num_bounds; b++) {
        volatile int n = bounds[b];
        
        /* Call all test functions with current bound */
        test_for_loop(n);
        test_while_loop(n);
        test_unsigned_loop(n);
        test_explicit_decr(n);
        test_nested_loop(n);
        test_greater_zero(n);
        
        #ifdef __ARM_ARCH
        test_arm_hardware_loop(n);
        #endif
        
        #ifdef __mips__
        test_mips_hardware_loop(n);
        #endif
        
        /* Add some variation by using different bounds for different calls */
        test_for_loop(n / 2);
        test_while_loop(n / 3);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", global_sink);
    
    return 0;
}
