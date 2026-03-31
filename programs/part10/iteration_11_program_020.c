/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
  #define ARCH_ARM 1
  #define ARCH_NAME "ARM"
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__)
  #define ARCH_PPC 1
  #define ARCH_NAME "PowerPC"
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_NAME "MIPS"
#else
  #define ARCH_GENERIC 1
  #define ARCH_NAME "Generic"
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory"); /* Memory barrier */
    }
    
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    /* Add one more for the last iteration */
    sum += 5;
    return sum;
}

/* Loop variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPTIMIZE
static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i != 0; i = i - 1) */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NO_OPTIMIZE
static int test_do_while_post(int limit) {
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

/* Loop variant 6: While loop with explicit comparison */
NO_OPTIMIZE
static int test_while_explicit(int limit) {
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

/* Loop variant 7: Unsigned counter (common in doloop patterns) */
NO_OPTIMIZE
static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) with unsigned */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 8: Counter in register variable hint */
NO_OPTIMIZE
static int test_register_var(int limit) {
    register int n asm("r0") = limit;  /* Suggest register for counter */
    int sum = 0;
    
    /* Pattern: while (n-- > 0) with register variable */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop optimization patterns on %s architecture\n", ARCH_NAME);
    printf("Targeting GCC loop-doloop.cc lines 136-150 pattern matching\n\n");
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    
    /* Execute all test variants */
    printf("Running test variants...\n");
    
    total_sum += test_post_decrement(loop_limit);
    printf("  test_post_decrement completed\n");
    
    total_sum += test_pre_decrement(loop_limit);
    printf("  test_pre_decrement completed\n");
    
    total_sum += test_for_loop_decrement(loop_limit);
    printf("  test_for_loop_decrement completed\n");
    
    total_sum += test_explicit_subtract(loop_limit);
    printf("  test_explicit_subtract completed\n");
    
    total_sum += test_do_while_post(loop_limit);
    printf("  test_do_while_post completed\n");
    
    total_sum += test_while_explicit(loop_limit);
    printf("  test_while_explicit completed\n");
    
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    printf("  test_unsigned_counter completed\n");
    
    total_sum += test_register_var(loop_limit);
    printf("  test_register_var completed\n");
    
    printf("\nTotal sum: %d\n", total_sum);
    printf("Loop limit was: %d\n", loop_limit);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum == 0) ? 1 : 0;
}
