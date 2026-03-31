/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimization, inlining, and cloning of test functions */
#define NOOPT __attribute__((noinline, noipa, noclone))

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

/* Different loop patterns to trigger the PLUS with -1 RTL pattern */

/* Pattern 1: Post-decrement in condition */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but opaque loop body */
        sum += 3;
        /* Prevent loop unrolling or other optimizations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 2: Pre-decrement in condition */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 3: For loop with decrement */
NOOPT int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 4: Explicit subtract in loop */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;
    }
    return sum;
}

/* Pattern 5: Do-while with post-decrement */
NOOPT int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Pattern 6: While with explicit compare to zero */
NOOPT int test_while_compare_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Explicit compare to zero */
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n--;
    }
    return sum;
}

/* Pattern 7: Unsigned counter (common in doloop patterns) */
NOOPT unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 8: Counter in register variable */
NOOPT int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Hint to use register (architecture-specific) */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main function with architecture-specific setup */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int loop_limit;
    
    /* Use different methods to get non-constant values */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use system call or volatile */
    #ifdef ARCH_ARM
        /* ARM-specific: use system call or volatile read */
        asm volatile("mov %0, #100" : "=r"(loop_limit));
    #elif defined(ARCH_AVR)
        /* AVR: small loop counts */
        loop_limit = 50;
    #else
        /* Generic: use getpid or volatile */
        loop_limit = (getpid() % 100) + 50;
    #endif
#else
    /* For generic architectures, still try to exercise the code */
    loop_limit = base_limit;
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_while_compare_zero(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero to indicate execution */
    return total_sum != 0 ? 0 : 1;
}
