/* loop-doloop-test.c - Test program for GCC doloop optimization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop optimizations */
#define NOOPT __attribute__((noinline, noclone, optimize("O2")))

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Test 1: Basic for loop with decrement and explicit compare to zero */
NOOPT void test_for_loop(volatile int n) {
    volatile int local_sink = 0;
    /* Pattern: for (i = n; i != 0; i--) */
    for (int i = n; i != 0; i--) {
        /* Simple side effect using loop counter */
        local_sink += i;
        /* Prevent optimization of loop counter */
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

/* Test 2: While loop with decrement in condition */
NOOPT void test_while_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    /* Pattern: while (i--) */
    while (i--) {
        local_sink += i;
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

/* Test 3: For loop with unsigned counter */
NOOPT void test_unsigned_loop(volatile int n) {
    volatile int local_sink = 0;
    /* Pattern: for (unsigned i = n; i != 0; --i) */
    for (unsigned int i = n; i != 0; --i) {
        local_sink += (int)i;
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

/* Test 4: Explicit decrement with subtraction */
NOOPT void test_explicit_decr(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    /* Pattern: while (i != 0) { i = i - 1; } */
    while (i != 0) {
        local_sink += i;
        asm volatile("" : : "r"(i));
        i = i - 1;  /* Explicit subtraction instead of i-- */
    }
    global_sink += local_sink;
}

/* Test 5: Nested loops to increase analysis complexity */
NOOPT void test_nested_loop(volatile int n) {
    volatile int local_sink = 0;
    volatile int outer = n / 10;
    if (outer < 1) outer = 1;
    
    for (int j = 0; j < outer; j++) {
        /* Inner loop with decrement pattern */
        for (int i = n; i != 0; i--) {
            local_sink += i + j;
            asm volatile("" : : "r"(i));
        }
    }
    global_sink += local_sink;
}

/* Test 6: Do-while style with pre-decrement */
NOOPT void test_dowhile_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    if (i <= 0) return;
    
    do {
        local_sink += i;
        asm volatile("" : : "r"(i));
    } while (--i != 0);
    
    global_sink += local_sink;
}

/* Test 7: Loop with array access using volatile index */
NOOPT void test_array_loop(volatile int n) {
    volatile int local_sink = 0;
    int array[1024];
    
    /* Initialize array */
    for (int k = 0; k < 1024; k++) {
        array[k] = k;
    }
    
    /* Target loop with decrement pattern */
    for (int i = n; i > 0; i--) {
        /* Access array with index derived from loop counter */
        volatile int idx = i % 1024;
        local_sink += array[idx];
        asm volatile("" : : "r"(i));
    }
    
    global_sink += local_sink;
}

/* Architecture-specific test for hardware loop support */
#ifdef __ARM_ARCH
NOOPT void test_arm_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Use __builtin_expect to guide branch prediction */
    for (int i = n; __builtin_expect(i != 0, 1); i--) {
        local_sink += i;
        /* ARM-specific asm hint */
        asm volatile("" : : "r"(i) : "cc");
    }
    
    global_sink += local_sink;
}
#endif

#ifdef __mips__
NOOPT void test_mips_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    for (int i = n; i != 0; i--) {
        local_sink += i;
        asm volatile("" : : "r"(i));
    }
    
    global_sink += local_sink;
}
#endif

/* Main driver with varying loop bounds */
int main() {
    volatile int bounds[] = {100, 500, 1000, 2000};
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
        test_dowhile_loop(n);
        test_array_loop(n);
        
        #ifdef __ARM_ARCH
        test_arm_hardware_loop(n);
        #endif
        
        #ifdef __mips__
        test_mips_hardware_loop(n);
        #endif
        
        printf("Completed tests with bound %d\n", n);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", global_sink);
    
    return 0;
}
