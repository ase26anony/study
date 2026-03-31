/* test_doloop_coverage.c
 * Compile with: gcc -O2 -fdoloop -fdump-rtl-doloop test_doloop_coverage.c -o test_doloop
 * Also try: gcc -O3 -funroll-loops -fno-peel-loops test_doloop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop transformations */
#define KEEP_LOOP asm volatile("" : : "r"(i) : "memory")

/* Test function 1: Basic for loop with decrement */
__attribute__((noinline, noclone, optimize("O2")))
int test_for_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    /* Pattern: for (i = n; i != 0; i--) */
    for (int i = n; i != 0; i--) {
        KEEP_LOOP;
        sink += i;          /* Side effect */
        result ^= i;        /* Prevent elimination */
    }
    return result + sink;
}

/* Test function 2: While loop with decrement */
__attribute__((noinline, noclone, optimize("O2")))
int test_while_loop(volatile int n) {
    volatile int sink = 0;
    int i = n;
    int result = 0;
    
    /* Pattern: while (i != 0) { ... i--; } */
    while (i != 0) {
        KEEP_LOOP;
        sink += i;
        result ^= i;
        i--;                /* Decrement at end */
    }
    return result + sink;
}

/* Test function 3: Unsigned counter */
__attribute__((noinline, noclone, optimize("Os")))  /* Size optimization */
unsigned test_unsigned_loop(volatile unsigned n) {
    volatile unsigned sink = 0;
    unsigned result = 0;
    
    /* Pattern: for (unsigned i = n; i != 0; --i) */
    for (unsigned i = n; i != 0; --i) {
        asm volatile("" : : "r"(i) : "memory");
        sink += i;
        result ^= i;
    }
    return result + sink;
}

/* Test function 4: Explicit decrement with subtraction */
__attribute__((noinline, noclone))
int test_explicit_decr(volatile int n) {
    volatile int sink = 0;
    int i = n;
    int result = 0;
    
    /* Pattern: while (i) { ... i = i - 1; } */
    while (i) {
        KEEP_LOOP;
        sink += i;
        result ^= i;
        i = i - 1;          /* Explicit subtraction */
    }
    return result + sink;
}

/* Test function 5: Do-while style (should still match) */
__attribute__((noinline, noclone, optimize("O3")))
int test_dowhile_loop(volatile int n) {
    volatile int sink = 0;
    int i = n;
    int result = 0;
    
    if (i > 0) {
        do {
            KEEP_LOOP;
            sink += i;
            result ^= i;
            i--;
        } while (i != 0);
    }
    return result + sink;
}

/* Test function 6: Nested loops to increase analysis complexity */
__attribute__((noinline, noclone))
int test_nested_loops(volatile int outer_n, volatile int inner_n) {
    volatile int sink = 0;
    int result = 0;
    
    for (int o = outer_n; o != 0; o--) {
        /* Inner loop with decrement pattern */
        for (int i = inner_n; i != 0; i--) {
            asm volatile("" : : "r"(i), "r"(o) : "memory");
            sink += i * o;
            result ^= i;
        }
    }
    return result + sink;
}

/* Test function 7: Array access with volatile index */
__attribute__((noinline, noclone))
int test_array_loop(volatile int n) {
    volatile int sink = 0;
    int array[1024];
    int result = 0;
    
    /* Initialize array */
    for (int j = 0; j < 1024; j++) {
        array[j] = j;
    }
    
    /* Target loop with decrement */
    for (int i = n; i > 0; i--) {
        volatile int idx = i % 1024;
        KEEP_LOOP;
        array[idx] += i;    /* Memory side effect */
        result ^= array[idx];
    }
    
    /* Use result to prevent elimination */
    for (int j = 0; j < 1024; j++) {
        sink += array[j];
    }
    return result + sink;
}

#ifdef __ARM_ARCH
/* ARM-specific test with hardware loop hint */
__attribute__((noinline, noclone, target("thumb")))
int test_arm_hardware_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    for (int i = n; i != 0; i--) {
        asm volatile("" : : "r"(i) : "memory");
        sink += i;
        result ^= i;
        
        /* Hint for hardware loop */
        if (__builtin_expect(i > 1, 1)) {
            /* Continue */
        }
    }
    return result + sink;
}
#endif

#ifdef __mips__
/* MIPS-specific test */
__attribute__((noinline, noclone))
int test_mips_hardware_loop(volatile int n) {
    volatile int sink = 0;
    int result = 0;
    
    for (int i = n; i != 0; --i) {
        asm volatile("" : : "r"(i) : "memory");
        sink += i;
        result ^= i;
    }
    return result + sink;
}
#endif

/* Main driver with multiple iterations */
int main(void) {
    volatile int checksum = 0;
    
    /* Test with different bounds to trigger various optimizations */
    volatile int bounds[] = {100, 500, 1000, 1, 10};
    int num_bounds = sizeof(bounds) / sizeof(bounds[0]);
    
    for (int b = 0; b < num_bounds; b++) {
        volatile int n = bounds[b];
        
        checksum += test_for_loop(n);
        checksum += test_while_loop(n);
        checksum += test_unsigned_loop(n);
        checksum += test_explicit_decr(n);
        checksum += test_dowhile_loop(n);
        checksum += test_nested_loops(n / 2, n);
        checksum += test_array_loop(n);
        
#ifdef __ARM_ARCH
        checksum += test_arm_hardware_loop(n);
#endif
        
#ifdef __mips__
        checksum += test_mips_hardware_loop(n);
#endif
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile use to ensure loops execute */
    volatile int final = checksum;
    return final != 0 ? 0 : 1;
}
