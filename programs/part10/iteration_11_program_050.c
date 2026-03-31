/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone, optimize("O0")))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This should generate: (plus (reg) (const_int -1)) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory"); /* Memory barrier */
    }
    return sum;
}

/* Variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This may generate different RTL but still decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPTIMIZE
static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Classic for loop decrement pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 4: Explicit subtract - for (i = limit; i != 0; i = i - 1) */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int sum = 0;
    
    /* Explicit subtraction by 1 */
    for (int i = limit; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 5: Do-while with post-decrement */
NO_OPTIMIZE
static int test_do_while_post(int limit) {
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

/* Variant 6: While loop with explicit comparison to zero */
NO_OPTIMIZE
static int test_while_explicit(int limit) {
    int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n = n - 1;  /* Explicit decrement separate from comparison */
    }
    return sum;
}

/* Variant 7: Unsigned counter (common in doloop patterns) */
NO_OPTIMIZE
static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Variant 8: Count down to zero with different comparison */
NO_OPTIMIZE
static int test_countdown(int limit) {
    int i = limit;
    int sum = 0;
    
    for (; __builtin_expect(i > 0, 1); --i) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent compile-time optimization */
#if ARCH_SUPPORTS_DOLOOP
    /* Use volatile or system call for architectures supporting doloop */
    volatile int vol_limit = 1000;
    loop_limit = vol_limit;
    
    /* Architecture-specific hints */
    #ifdef ARCH_ARM
        asm volatile("" ::: "memory");
        /* ARM-specific: ensure we're not in Thumb mode for certain patterns */
        #ifndef __thumb__
            asm volatile(".arm");
        #endif
    #elif defined(ARCH_AVR)
        /* AVR often uses special doloop instructions */
        asm volatile("" ::: "memory");
    #elif defined(ARCH_PPC)
        /* PowerPC often has doloop support */
        asm volatile("" ::: "memory");
    #endif
#else
    /* Generic fallback - still try to generate the pattern */
    loop_limit = get_loop_limit();
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    
    /* Execute all test variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_while_explicit(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_countdown(loop_limit);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to ensure all code paths are executed */
    return total_sum != 0 ? 0 : 1;
}
