/* loop-doloop-test.c
 * Test program to trigger GCC's doloop optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

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

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* volatile prevents constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This should generate: (compare (plus reg -1) (const_int 0)) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3; /* Simple arithmetic that won't be optimized away */
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* This may also generate the PLUS -1 pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* Classic for loop decrement pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract - i = i - 1 */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Explicit subtraction by 1 */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1; /* Should generate (plus reg -1) */
    }
    return sum;
}

/* Loop variant 5: Do-while with post-decrement */
NO_OPT static int test_do_while_postdec(int limit) {
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

/* Loop variant 6: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Unsigned comparison to zero */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit; /* Suggest register allocation */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 8: Nested loops to test multiple patterns */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer = outer_limit;
    int total = 0;
    
    while (__builtin_expect(outer-- > 0, 1)) {
        int inner = inner_limit;
        int inner_sum = 0;
        
        /* Inner loop with different pattern */
        for (int i = inner; __builtin_expect(i > 0, 1); i--) {
            inner_sum += 23;
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        
        total += inner_sum;
        asm volatile("" : "+r"(total) : : "memory");
    }
    return total;
}

/* Architecture-specific test functions */
#ifdef ARCH_ARM
/* ARM-specific: Use register constraints that might trigger doloop */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subtract and compare instructions */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 29;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #29" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

#ifdef ARCH_AVR
/* AVR-specific: 8-bit counters often use doloop */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF); /* 8-bit counter */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 31;
        /* AVR doesn't like complex asm in C, use simple increment */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    int total_result = 0;
    int loop_limit;
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching on ");
    
#if ARCH_SUPPORTS_DOLOOP
    #ifdef ARCH_ARM
    printf("ARM architecture\n");
    #elif defined(ARCH_AVR)
    printf("AVR architecture\n");
    #elif defined(ARCH_PPC)
    printf("PowerPC architecture\n");
    #elif defined(ARCH_MIPS)
    printf("MIPS architecture\n");
    #endif
    
    /* Run all loop variants */
    total_result += test_post_decrement(loop_limit);
    total_result += test_pre_decrement(loop_limit);
    total_result += test_for_loop_decrement(loop_limit);
    total_result += test_explicit_subtract(loop_limit);
    total_result += test_do_while_postdec(loop_limit);
    total_result += test_unsigned_counter((unsigned int)loop_limit);
    total_result += test_register_counter(loop_limit);
    total_result += test_nested_loops(loop_limit / 10, 10);
    
    /* Architecture-specific tests */
    #ifdef ARCH_ARM
    total_result += test_arm_specific(loop_limit);
    #endif
    
    #ifdef ARCH_AVR
    total_result += test_avr_specific(loop_limit);
    #endif
    
#else
    printf("generic architecture (doloop may not be supported)\n");
    
    /* Run a subset that should still exercise the code */
    total_result += test_post_decrement(loop_limit);
    total_result += test_for_loop_decrement(loop_limit);
    total_result += test_explicit_subtract(loop_limit);
#endif
    
    printf("Total result: %d\n", total_result);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_result == 0) ? 1 : 0;
}
