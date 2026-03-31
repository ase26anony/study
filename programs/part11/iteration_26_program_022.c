/* test_doloop.c - Test program for GCC loop-doloop.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Prevent inlining and cloning to preserve loop structure */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_loop_neq(volatile int n) {
    volatile int local_sink = 0;
    for (int i = n; i != 0; i--) {
        /* Simple side effect using loop counter */
        local_sink += i;
        /* Prevent loop removal with asm */
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

__attribute__((noinline, noclone, optimize("O2")))
void test_for_loop_gt(volatile int n) {
    volatile int local_sink = 0;
    for (int i = n; i > 0; i--) {
        local_sink ^= i;  /* Different operation */
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

__attribute__((noinline, noclone, optimize("O2")))
void test_while_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    while (i != 0) {
        local_sink |= i;
        asm volatile("" : : "r"(i));
        i--;
    }
    global_sink += local_sink;
}

__attribute__((noinline, noclone, optimize("O2")))
void test_unsigned_loop(volatile unsigned int n) {
    volatile unsigned int local_sink = 0;
    for (unsigned int i = n; i != 0; --i) {
        local_sink += i;
        asm volatile("" : : "r"(i));
    }
    global_sink += local_sink;
}

__attribute__((noinline, noclone, optimize("O2")))
void test_explicit_decrement(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    while (i) {
        local_sink += i;
        asm volatile("" : : "r"(i));
        i = i - 1;  /* Explicit subtraction instead of i-- */
    }
    global_sink += local_sink;
}

/* Nested loop to increase analysis complexity */
__attribute__((noinline, noclone, optimize("O2")))
void test_nested_loop(volatile int n) {
    volatile int local_sink = 0;
    volatile int outer = n / 2;
    
    for (int j = outer; j != 0; j--) {
        for (int i = n; i != 0; i--) {
            local_sink += i * j;
            asm volatile("" : : "r"(i), "r"(j));
        }
    }
    global_sink += local_sink;
}

/* Array access pattern */
__attribute__((noinline, noclone, optimize("O2")))
void test_array_loop(volatile int n) {
    volatile int local_sink = 0;
    int array[256];
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    for (int i = n; i != 0; i--) {
        /* Access with volatile-like pattern */
        int idx = i & 0xFF;
        local_sink += array[idx];
        asm volatile("" : : "r"(i), "r"(idx));
    }
    global_sink += local_sink;
}

/* Architecture-specific hardware loop hints */
#ifdef __ARM_ARCH
__attribute__((noinline, noclone, optimize("O2")))
void test_arm_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    /* Use builtin to guide branch prediction */
    while (__builtin_expect(i != 0, 1)) {
        local_sink += i;
        /* ARM-specific asm hint */
        asm volatile("" : : "r"(i) : "cc");
        i--;
    }
    global_sink += local_sink;
}
#endif

#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hardware_loop(volatile int n) {
    volatile int local_sink = 0;
    int i = n;
    
    while (i != 0) {
        local_sink += i;
        /* MIPS delay slot hint */
        asm volatile("" : : "r"(i));
        i--;
    }
    global_sink += local_sink;
}
#endif

/* Main test driver */
int main(void) {
    volatile int test_cases[] = {100, 500, 1000, 1, 10};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Starting doloop pattern tests...\n");
    
    for (int t = 0; t < num_cases; t++) {
        volatile int n = test_cases[t];
        global_counter++;
        
        /* Test various loop patterns */
        test_for_loop_neq(n);
        test_for_loop_gt(n);
        test_while_loop(n);
        test_unsigned_loop((unsigned int)n);
        test_explicit_decrement(n);
        test_nested_loop(n);
        test_array_loop(n);
        
        /* Architecture-specific tests */
#ifdef __ARM_ARCH
        test_arm_hardware_loop(n);
#endif
#ifdef __mips__
        test_mips_hardware_loop(n);
#endif
        
        /* Print progress to prevent optimization */
        printf("Completed test case %d with n=%d\n", t + 1, n);
    }
    
    /* Final result that must be computed */
    int result = global_sink + global_counter;
    printf("Final checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 0) {
        return 0;
    } else {
        return 1;
    }
}
