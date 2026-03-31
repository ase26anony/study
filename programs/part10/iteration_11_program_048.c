/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
    #define ARCH_ARM 1
    #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__AVR__)
    #define ARCH_AVR 1
    #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
    #define ARCH_PPC 1
    #define ARCH_SUPPORTS_DOLOOP 1
#elif defined(__mips__)
    #define ARCH_MIPS 1
    #define ARCH_SUPPORTS_DOLOOP 1
#else
    #define ARCH_GENERIC 1
    #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test function 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body that can't be optimized away */
        sum += 3;
        /* Prevent loop unrolling or other transformations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    for (int i = limit; i != 0; i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 1, 1));
    }
    return sum;
}

/* Test function 6: While loop with explicit comparison to zero */
NO_OPT static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;
    }
    return sum;
}

/* Test function 7: Count down to zero with unsigned counter */
NO_OPT static int test_unsigned_decrement(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Complex decrement pattern that might generate PLUS with -1 */
NO_OPT static int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    int temp;
    
    while (__builtin_expect(n > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
        /* Force separate decrement that might be combined into compare */
        temp = n;
        temp = temp - 1;
        n = temp;
    }
    return sum;
}

/* Main function with architecture-specific adaptations */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* Use volatile or system call to get non-constant value */
    volatile int vol_limit = 1000;
    loop_limit = vol_limit;
    
    /* Also try with function return value */
    loop_limit += getpid() & 0x3F; /* Small non-constant addition */
#else
    /* For generic architectures, still use non-constant but smaller loops */
    loop_limit = 100 + (getpid() & 0xF);
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
    
    /* Execute all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_decrement(loop_limit);
    total_sum += test_complex_decrement(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum == 0) ? 1 : 0;
}
