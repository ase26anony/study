/* loop-doloop-coverage.c
 * Designed to trigger GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 of loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop optimizations */
#define KEEP_LOOP(x) asm volatile("" : : "r"(x) : "memory")

/* Volatile sink to prevent dead code elimination */
static volatile int global_sink = 0;

/* Test 1: Basic for loop with decrement and != 0 comparison */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_neq_zero(volatile int n) {
    volatile int local_sink = 0;
    /* Pattern: for (reg; reg != 0; reg--) */
    for (int i = n; i != 0; i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    global_sink += local_sink;
}

/* Test 2: While loop with post-decrement in condition */
__attribute__((noinline, noclone, optimize("O2")))
void test_while_postdec(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    /* Pattern: while (i--) */
    while (i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    global_sink += local_sink;
}

/* Test 3: For loop with > 0 comparison */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_gt_zero(volatile int n) {
    volatile int local_sink = 0;
    /* Pattern: for (reg; reg > 0; reg--) */
    for (int i = n; i > 0; i--) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    global_sink += local_sink;
}

/* Test 4: Unsigned counter with explicit decrement */
__attribute__((noinline, noclone, optimize("O2")))
void test_unsigned_dec(volatile unsigned int n) {
    volatile unsigned int local_sink = 0;
    /* Pattern: for (unsigned reg; reg != 0; reg = reg - 1) */
    for (unsigned int i = n; i != 0; i = i - 1) {
        local_sink += i;
        KEEP_LOOP(i);
    }
    global_sink += local_sink;
}

/* Test 5: Do-while with pre-decrement */
__attribute__((noinline, noclone, optimize("O2")))
void test_dowhile_predec(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    if (i > 0) {
        do {
            --i;
            local_sink += i;
            KEEP_LOOP(i);
        } while (i != 0);
    }
    global_sink += local_sink;
}

/* Test 6: Nested loops to increase analysis complexity */
__attribute__((noinline, noclone, optimize("O2")))
void test_nested_loops(volatile int n) {
    volatile int local_sink = 0;
    volatile int outer = n / 10;
    
    for (int j = outer; j != 0; j--) {
        int inner = n;
        /* Inner loop with the target pattern */
        for (int i = inner; i != 0; i--) {
            local_sink += i * j;
            KEEP_LOOP(i);
        }
        KEEP_LOOP(j);
    }
    global_sink += local_sink;
}

/* Test 7: Loop with array access side effect */
__attribute__((noinline, noclone, optimize("O2")))
void test_array_access(volatile int n) {
    volatile int local_sink = 0;
    int array[256];
    
    /* Initialize array */
    for (int k = 0; k < 256; k++) {
        array[k] = k;
    }
    
    /* Target loop with array access */
    for (int i = n; i != 0; i--) {
        /* Access with volatile-like behavior */
        local_sink += array[i & 0xFF];
        KEEP_LOOP(i);
    }
    global_sink += local_sink;
}

/* Architecture-specific tests for hardware loop support */
#ifdef __ARM_ARCH
__attribute__((noinline, noclone, optimize("O2")))
void test_arm_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Use __builtin_expect to guide branch prediction */
    for (int i = n; __builtin_expect(i != 0, 1); i--) {
        local_sink += i;
        /* ARM-specific asm to hint at hardware loops */
        asm volatile("" : : "r"(i) : "cc", "memory");
    }
    global_sink += local_sink;
}
#endif

#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    for (int i = n; i != 0; i--) {
        local_sink += i;
        /* MIPS delay slot hint */
        asm volatile("" : : "r"(i) : "memory");
    }
    global_sink += local_sink;
}
#endif

/* Main driver with multiple iterations */
int main(void) {
    volatile int test_cases[] = {100, 500, 1000, 2500};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Starting doloop pattern tests...\n");
    
    for (int t = 0; t < num_cases; t++) {
        volatile int N = test_cases[t];
        
        /* Call all test functions */
        test_for_neq_zero(N);
        test_while_postdec(N);
        test_for_gt_zero(N);
        test_unsigned_dec(N);
        test_dowhile_predec(N);
        test_nested_loops(N);
        test_array_access(N);
        
        /* Architecture-specific tests */
        #ifdef __ARM_ARCH
        test_arm_hardware_loop(N);
        #endif
        
        #ifdef __mips__
        test_mips_hardware_loop(N);
        #endif
        
        printf("Completed test case N=%d, global_sink=%d\n", N, global_sink);
    }
    
    /* Final output to prevent complete optimization */
    printf("Final checksum: %d\n", global_sink);
    
    return 0;
}
