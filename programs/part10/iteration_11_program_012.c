/* loop-doloop-test.c - Test program for GCC doloop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimization of individual test functions */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
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

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body that can't be optimized away */
        sum += 3;
        /* Prevent loop unrolling with a small asm barrier */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pre-decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle the last iteration for comparison with zero */
    if (n == 0) {
        sum += 5;
    }
    return sum;
}

/* Test 3: For loop with decrement - for (int i = limit; i > 0; i--) */
NOOPT int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract - for (int i = limit; i != 0; i = i - 1) */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtraction, not i-- */
    }
    return sum;
}

/* Test 5: Do-while with post-decrement */
NOOPT int test_do_while_post(int limit) {
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

/* Test 6: Unsigned counter - while (count-- != 0) */
NOOPT int test_unsigned_decrement(int limit) {
    unsigned int count = (unsigned int)limit;
    int sum = 0;
    
    while (__builtin_expect(count-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 7: Complex decrement pattern with multiple uses */
NOOPT int test_complex_pattern(int limit) {
    int n = limit;
    int sum = 0;
    int temp;
    
    while (__builtin_expect(n > 0, 1)) {
        sum += 19;
        temp = n;  /* Additional use of n */
        asm volatile("" : "+r"(sum), "+r"(temp) : : "memory");
        n = n - 1;  /* Decrement at end */
    }
    return sum;
}

/* Main function with architecture-specific handling */
int main(int argc, char *argv[]) {
    int loop_limit;
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use volatile or system call */
    volatile int vol_limit = 1000;
    loop_limit = vol_limit;
    
    /* Also try with a function return value */
    loop_limit += getpid() & 0x3F;  /* Add some randomness 0-63 */
#else
    /* For generic architectures, still use non-constant */
    if (argc > 1) {
        loop_limit = atoi(argv[1]);
    } else {
        loop_limit = 1000;
    }
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
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_unsigned_decrement(loop_limit);
    total_sum += test_complex_pattern(loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero if any test failed (all should succeed) */
    return (total_sum > 0) ? 0 : 1;
}
