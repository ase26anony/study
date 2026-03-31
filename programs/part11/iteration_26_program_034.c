/* test_doloop.cc - Test program for GCC loop-doloop.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent compile-time optimization */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Prevent inlining and cloning to preserve loop structure */
__attribute__((noinline, noclone))
void test_for_loop_signed(volatile int n) {
    volatile int local_sink = 0;
    
    /* Pattern: for loop with decrement and compare to zero */
    for (int i = n; i != 0; i--) {
        /* Simple side effect using asm to prevent removal */
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}

__attribute__((noinline, noclone))
void test_for_loop_unsigned(volatile unsigned int n) {
    volatile unsigned int local_sink = 0;
    
    /* Pattern: unsigned counter with explicit decrement */
    for (unsigned int i = n; i != 0; --i) {
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}

__attribute__((noinline, noclone))
void test_while_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Pattern: while loop with decrement in condition */
    while (i--) {
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}

__attribute__((noinline, noclone))
void test_do_while_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Pattern: do-while with explicit decrement */
    if (i > 0) {
        do {
            asm volatile ("" : : "r"(i));
            local_sink += i;
        } while (--i != 0);
    }
    
    global_sink += local_sink;
}

__attribute__((noinline, noclone))
void test_explicit_decrement(volatile int n) {
    volatile int local_sink = 0;
    
    /* Pattern: explicit i = i - 1 decrement */
    for (int i = n; i != 0; i = i - 1) {
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}

__attribute__((noinline, noclone))
void test_nested_loop(volatile int outer_n) {
    volatile int local_sink = 0;
    
    /* Nested loop to increase analysis complexity */
    for (int outer = outer_n; outer != 0; outer--) {
        int inner = outer % 10;
        for (int i = inner; i != 0; i--) {
            asm volatile ("" : : "r"(i));
            local_sink += i * outer;
        }
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
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}
#endif

#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    
    for (int i = n; i != 0; i--) {
        /* MIPS-specific asm to hint at loop usage */
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}
#endif

/* Function with pragma to ensure optimization level */
#pragma GCC push_options
#pragma GCC optimize ("O2")
__attribute__((noinline, noclone))
void test_pragma_optimized_loop(volatile int n) {
    volatile int local_sink = 0;
    
    /* Simple decrementing loop with size optimization hint */
    for (int i = n; i > 0; --i) {
        asm volatile ("" : : "r"(i));
        local_sink += i;
    }
    
    global_sink += local_sink;
}
#pragma GCC pop_options

/* Size-optimized version */
__attribute__((noinline, noclone, optimize("Os")))
void test_size_optimized_loop(volatile int n) {
    volatile int local_sink = 0;
    
    for (int i = n; i != 0; i--) {
        /* Minimal side effect */
        local_sink ^= i;
    }
    
    global_sink += local_sink;
}

/* Main test driver */
int main() {
    volatile int test_cases[] = {100, 500, 1000, 42, 256};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Starting doloop pattern tests...\n");
    
    for (int tc = 0; tc < num_cases; tc++) {
        volatile int n = test_cases[tc];
        
        /* Call all test functions with varying bounds */
        test_for_loop_signed(n);
        test_for_loop_unsigned((unsigned int)n);
        test_while_loop(n);
        test_do_while_loop(n);
        test_explicit_decrement(n);
        test_nested_loop(n % 50 + 1); /* Smaller bounds for nested loops */
        test_pragma_optimized_loop(n);
        test_size_optimized_loop(n);
        
        /* Architecture-specific tests */
        #ifdef __ARM_ARCH
        test_arm_hardware_loop(n);
        #endif
        
        #ifdef __mips__
        test_mips_hardware_loop(n);
        #endif
        
        /* Increment global counter to prevent dead code elimination */
        global_counter++;
    }
    
    /* Print results to prevent optimization */
    printf("Tests completed. Global sink: %d, Counter: %d\n", 
           global_sink, global_counter);
    
    /* Use result in a way that can't be optimized away */
    if (global_sink > 0) {
        return 0;
    } else {
        return 1;
    }
}
