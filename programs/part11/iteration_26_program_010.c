/* loop-doloop-coverage.c
 * Target: Trigger specific RTL pattern matching in GCC's loop-doloop.cc
 * Compile with: -O2 -fdoloop -fdump-rtl-doloop
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent early loop transformations */
#define KEEP_LOOP(var) asm volatile("" : "+r"(var))

/* Different loop variants to trigger various paths */
__attribute__((noinline, noclone, optimize("O2")))
void test_for_loop(volatile int n) {
    volatile int sink = 0;
    for (int i = n; i != 0; i--) {
        sink += i;
        KEEP_LOOP(i);
    }
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sink));
}

__attribute__((noinline, noclone, optimize("O2")))
void test_while_loop(volatile int n) {
    volatile int sink = 0;
    int i = n;
    while (i != 0) {
        sink += i;
        KEEP_LOOP(i);
        i--;
    }
    asm volatile("" : : "r"(sink));
}

__attribute__((noinline, noclone, optimize("O2")))
void test_unsigned_loop(volatile unsigned n) {
    volatile unsigned sink = 0;
    for (unsigned i = n; i != 0; --i) {
        sink += i;
        KEEP_LOOP(i);
    }
    asm volatile("" : : "r"(sink));
}

__attribute__((noinline, noclone, optimize("O2")))
void test_explicit_decrement(volatile int n) {
    volatile int sink = 0;
    int i = n;
    while (i) {
        sink += i;
        KEEP_LOOP(i);
        i = i - 1;  /* Explicit subtraction instead of i-- */
    }
    asm volatile("" : : "r"(sink));
}

__attribute__((noinline, noclone, optimize("O2")))
void test_nested_loop(volatile int n) {
    volatile int sink = 0;
    volatile int outer = n / 10;
    
    for (int j = 0; j < outer; j++) {
        /* Inner loop with decrementing counter */
        for (int i = n; i != 0; i--) {
            sink += i * j;
            KEEP_LOOP(i);
        }
    }
    asm volatile("" : : "r"(sink));
}

__attribute__((noinline, noclone, optimize("O2")))
void test_array_access(volatile int n) {
    volatile int sink = 0;
    int array[7] = {0};  /* Small prime-sized array */
    
    for (int i = n; i > 0; i--) {
        /* Access with side effect, using modulo to stay in bounds */
        array[i % 7] = i;
        sink += array[i % 7];
        KEEP_LOOP(i);
    }
    asm volatile("" : : "r"(sink), "r"(array[0]));
}

/* Architecture-specific variants */
#ifdef __ARM_ARCH
__attribute__((noinline, noclone, optimize("O2")))
void test_arm_hint_loop(volatile int n) {
    volatile int sink = 0;
    int i = n;
    
    /* Use likely to guide branch prediction */
    while (__builtin_expect(i != 0, 1)) {
        sink += i;
        KEEP_LOOP(i);
        i--;
    }
    
    /* ARM-specific asm hint */
    asm volatile("" : : "r"(sink));
}
#endif

#ifdef __mips__
__attribute__((noinline, noclone, optimize("O2")))
void test_mips_hint_loop(volatile int n) {
    volatile int sink = 0;
    
    for (int i = n; i != 0; i--) {
        sink += i;
        KEEP_LOOP(i);
    }
    
    /* MIPS delay slot hint */
    asm volatile("" : : "r"(sink));
}
#endif

/* Main driver with multiple iterations */
int main(void) {
    volatile int checksum = 0;
    
    /* Test with different bounds to increase coverage chance */
    volatile int bounds[] = {100, 500, 1000, 1, 10};
    
    for (int b = 0; b < 5; b++) {
        volatile int n = bounds[b];
        
        test_for_loop(n);
        checksum += n;
        
        test_while_loop(n);
        checksum += n;
        
        test_unsigned_loop((unsigned)n);
        checksum += n;
        
        test_explicit_decrement(n);
        checksum += n;
        
        test_nested_loop(n);
        checksum += n;
        
        test_array_access(n);
        checksum += n;
        
        #ifdef __ARM_ARCH
        test_arm_hint_loop(n);
        checksum += n;
        #endif
        
        #ifdef __mips__
        test_mips_hint_loop(n);
        checksum += n;
        #endif
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
