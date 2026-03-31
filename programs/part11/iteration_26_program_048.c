/* test_doloop_coverage.c
 * Compile with: gcc -O2 -fdoloop -fdump-rtl-doloop -c test_doloop_coverage.c
 * Also try: gcc -O3 -funroll-loops -fno-peel-loops -fno-move-loop-invariants -c test_doloop_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop transformations */
#define KEEP_LOOP_INTACT asm volatile("" : : "r"(i) : "memory")

/* Different loop variants to trigger various RTL patterns */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_loop(volatile int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    asm volatile("" : : "r"(sink) : "memory");
}

__attribute__((noinline, noclone, optimize("O2")))
void test_while_loop(volatile int n) {
    volatile int sink = 0;
    int i = n;
    while (i != 0) {
        sink += i;
        KEEP_LOOP_INTACT;
        i--;
    }
    asm volatile("" : : "r"(sink) : "memory");
}

__attribute__((noinline, noclone, optimize("O2")))
void test_unsigned_loop(volatile unsigned int n) {
    volatile unsigned int sink = 0;
    for (unsigned int i = n; i != 0; --i) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    asm volatile("" : : "r"(sink) : "memory");
}

__attribute__((noinline, noclone, optimize("O2")))
void test_explicit_decrement(volatile int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i = i - 1) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    asm volatile("" : : "r"(sink) : "memory");
}

__attribute__((noinline, noclone, optimize("O2")))
void test_gt_zero_loop(volatile int n) {
    volatile int sink = 0;
    for (int i = n; i > 0; i--) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    asm volatile("" : : "r"(sink) : "memory");
}

/* Nested loop to increase analysis complexity */
__attribute__((noinline, noclone, optimize("O2")))
void test_nested_loop(volatile int n) {
    volatile int sink = 0;
    volatile int outer = n / 10;
    
    for (int j = 0; j < outer; j++) {
        for (int i = n; i != 0; i--) {
            sink += i ^ j;
            KEEP_LOOP_INTACT;
        }
    }
    asm volatile("" : : "r"(sink) : "memory");
}

/* Array access with volatile index */
__attribute__((noinline, noclone, optimize("O2")))
void test_array_loop(volatile int n) {
    volatile int sink = 0;
    int array[1024];
    
    for (int i = n; i != 0; i--) {
        volatile int idx = i % 1024;
        array[idx] = i;
        sink += array[idx];
        KEEP_LOOP_INTACT;
    }
    asm volatile("" : : "r"(sink), "r"(array[0]) : "memory");
}

/* Architecture-specific optimizations */
#ifdef __ARM_ARCH
__attribute__((noinline, noclone, optimize("O2")))
void test_arm_hardware_loop(volatile int n) {
    volatile int sink = 0;
    
    /* Hint for hardware loop support */
    asm volatile("" : : : "memory");
    
    for (int i = n; i != 0; i--) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    
    /* Prevent tail call optimization */
    asm volatile("" : : "r"(sink) : "memory");
}
#endif

#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hardware_loop(volatile int n) {
    volatile int sink = 0;
    
    for (int i = n; i != 0; i--) {
        sink += i;
        KEEP_LOOP_INTACT;
    }
    
    asm volatile("" : : "r"(sink) : "memory");
}
#endif

/* Main test driver */
int main(void) {
    volatile int checksum = 0;
    
    /* Test with different bounds to trigger various optimizations */
    volatile int bounds[] = {100, 500, 1000, 5000};
    
    for (int b = 0; b < 4; b++) {
        volatile int n = bounds[b];
        
        test_for_loop(n);
        checksum += n;
        
        test_while_loop(n);
        checksum += n;
        
        test_unsigned_loop((unsigned int)n);
        checksum += n;
        
        test_explicit_decrement(n);
        checksum += n;
        
        test_gt_zero_loop(n);
        checksum += n;
        
        test_nested_loop(n);
        checksum += n;
        
        test_array_loop(n);
        checksum += n;
        
        #ifdef __ARM_ARCH
        test_arm_hardware_loop(n);
        checksum += n;
        #endif
        
        #ifdef __mips__
        test_mips_hardware_loop(n);
        checksum += n;
        #endif
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
