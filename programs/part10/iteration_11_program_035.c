/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection for doloop support */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "ARM"
#elif defined(__AVR__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "PowerPC"
#elif defined(__mips__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "MIPS"
#else
#define ARCH_SUPPORTS_DOLOOP 0
#define ARCH_NAME "Generic"
#endif

/* Test function 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) - should generate (plus n -1) in RTL */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory");  /* Memory barrier */
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) - should generate (plus n -1) in RTL */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Pattern: for (i = limit; i != 0; i = i - 1) */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: do { ... } while (n-- > 0) */
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Test function 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n != 0) { n = n - 1; ... } */
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Test function 7: Unsigned counter (common in doloop optimizations) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) - unsigned version */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Counter in register variable hint */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Hint to use register */
    int sum = 0;
    
    /* Pattern: while (n-- > 0) with register variable */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main function with architecture-specific setup */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop pattern matching on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use volatile or system call */
    volatile int volatile_limit = 100;
    loop_limit = volatile_limit;
    
    /* Also try with function return value */
    loop_limit += getpid() & 0x3F;  /* Add some randomness 0-63 */
#else
    /* For generic architectures, still use non-constant */
    loop_limit = 100 + (getpid() & 0x1F);
#endif
    
    printf("Using loop limit: %d\n", loop_limit);
    
    /* Execute all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_register_var(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to indicate execution */
    return total_sum != 0 ? 0 : 1;
}
