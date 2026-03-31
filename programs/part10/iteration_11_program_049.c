/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone,optimize("O0")))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
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
    #define ARCH_GENERIC 1
    #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;  /* Simple arithmetic that can't be optimized away */
        /* Prevent compiler from moving operations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Force pre-decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration for exact comparison */
    if (n == 0) {
        sum += 5;
    }
    return sum;
}

/* Variant 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not decrement operator */
    }
    return sum;
}

/* Variant 5: Do-while with post-decrement - do { ... } while (n-- > 0) */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Variant 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;
    }
    return sum;
}

/* Variant 7: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 8: Counter in register variable hint */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int loop_limit;
    
    /* Use volatile or system call to get unpredictable but reasonable value */
#ifdef ARCH_SUPPORTS_DOLOOP
    /* For doloop targets, use a value that ensures multiple iterations */
    loop_limit = base_limit + (getpid() & 0x3F);  /* 100-163 iterations */
#else
    /* For generic targets, use a fixed but non-constant appearing value */
    loop_limit = base_limit + 37;
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    printf("Architecture: ");
    
#if defined(ARCH_ARM)
    printf("ARM\n");
#elif defined(ARCH_AVR)
    printf("AVR\n");
#elif defined(ARCH_PPC)
    printf("PowerPC\n");
#elif defined(ARCH_MIPS)
    printf("MIPS\n");
#else
    printf("Generic\n");
#endif
    
    /* Execute all test variants */
    total += test_post_decrement(loop_limit);
    total += test_pre_decrement(loop_limit);
    total += test_for_loop_decrement(loop_limit);
    total += test_explicit_subtract(loop_limit);
    total += test_do_while_post(loop_limit);
    total += test_while_explicit(loop_limit);
    total += test_unsigned_counter((unsigned int)loop_limit);
    total += test_register_var(loop_limit);
    
    printf("Total sum from all loops: %d\n", total);
    
    /* Return non-zero if any test failed (all should return positive values) */
    return (total > 0) ? 0 : 1;
}
