/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone, optimize(0)))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
  #define ARCH_ARM 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
  #define ARCH_PPC 1
  #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_SUPPORTS_DOLOOP 1
#else
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This should generate: (plus reg -1) compare with 0 */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This may also generate the pattern depending on RTL expansion */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPTIMIZE
static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Classic for loop pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Explicit subtraction that should generate PLUS with -1 */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 5: Do-while with post-decrement */
NO_OPTIMIZE
static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test 6: Unsigned counter (common in embedded systems) */
NO_OPTIMIZE
static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 7: Counter in register variable */
NO_OPTIMIZE
static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 8: Nested loops - outer loop should still match pattern */
NO_OPTIMIZE
static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer_sum = 0;
    
    for (int i = outer_limit; __builtin_expect(i > 0, 1); i--) {
        int inner_sum = 0;
        int j = inner_limit;
        
        while (__builtin_expect(j-- > 0, 1)) {
            inner_sum += 2;
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        
        outer_sum += inner_sum;
    }
    
    return outer_sum;
}

/* Architecture-specific optimizations */
#if ARCH_SUPPORTS_DOLOOP
/* Function to get non-constant loop limit */
NO_OPTIMIZE
static int get_loop_limit(void) {
    volatile int limit = 100;  /* Prevent constant propagation */
    /* Mix with system call to add variability */
    limit += (getpid() & 0xF);  /* Add some low bits from PID */
    return limit;
}

/* ARM-specific: Use register constraints that might help */
#ifdef ARCH_ARM
NO_OPTIMIZE
static int test_arm_specific(int limit) {
    register int n asm("r4") = limit;
    int sum = 0;
    
    asm volatile("" : "+r"(n));  /* Ensure n is in register */
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        /* ARM-specific asm to prevent optimizations */
        asm volatile("add %0, %0, #23" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

/* AVR-specific: Small loops common on AVR */
#ifdef ARCH_AVR
NO_OPTIMIZE
static int test_avr_specific(int limit) {
    uint8_t n = (limit > 255) ? 255 : (uint8_t)limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        /* AVR doesn't like complex asm here, use simple barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif
#endif /* ARCH_SUPPORTS_DOLOOP */

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
#if ARCH_SUPPORTS_DOLOOP
    printf("Testing on doloop-supported architecture\n");
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    printf("Using loop limit: %d\n", loop_limit);
    
    /* Run all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    total_sum += test_nested_loops(loop_limit / 10, 10);
    
    /* Architecture-specific tests */
    #ifdef ARCH_ARM
    total_sum += test_arm_specific(loop_limit);
    #endif
    
    #ifdef ARCH_AVR
    total_sum += test_avr_specific(loop_limit);
    #endif
    
#else
    printf("Testing on generic architecture (doloop may not be enabled)\n");
    /* Use a simple volatile to get non-constant limit */
    volatile int base_limit = 50;
    loop_limit = base_limit + (getpid() & 0x7);
    
    /* Run basic tests anyway - pattern might still be generated */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to ensure all code paths are considered */
    return (total_sum > 0) ? 0 : 1;
}
