/* loop-doloop-test.c - Test program for GCC's doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

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

/* Different loop patterns to trigger the PLUS with -1 RTL pattern */

/* Pattern 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern - should generate (plus reg -1) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration if needed */
    if (limit > 0) {
        sum += 5;
    }
    return sum;
}

/* Pattern 3: For loop with decrement */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    /* for (i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 4: Explicit subtract operation */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (i = limit; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Pattern 5: Do-while with post-decrement */
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

/* Pattern 6: While loop with explicit compare to zero */
NO_OPT static int test_while_compare_zero(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n) pattern with n-- inside */
    while (__builtin_expect(n, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        n--;
    }
    return sum;
}

/* Pattern 7: Unsigned counter (common in doloop optimizations) */
NO_OPT static unsigned int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Pattern 8: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int loop_limit;
    
    /* Use different methods to get non-constant values */
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use more aggressive patterns */
    
    #ifdef ARCH_ARM
        /* ARM-specific: ensure we're in a mode that supports doloop */
        asm volatile("" : : : "memory");
        loop_limit = base_limit + (getpid() & 0xF);  /* Small non-constant offset */
        printf("Testing on ARM architecture\n");
        
    #elif defined(ARCH_AVR)
        /* AVR often has specific doloop instructions */
        loop_limit = 255;  /* Common max for 8-bit */
        printf("Testing on AVR architecture\n");
        
    #elif defined(ARCH_PPC)
        /* PowerPC has strong doloop support */
        loop_limit = base_limit + (clock() & 0x7);
        printf("Testing on PowerPC architecture\n");
        
    #elif defined(ARCH_MIPS)
        /* MIPS also supports doloop */
        loop_limit = base_limit;
        printf("Testing on MIPS architecture\n");
        
    #endif
    
#else
    /* Generic fallback for architectures without doloop support */
    loop_limit = base_limit;
    printf("Testing on generic architecture (doloop may not be supported)\n");
#endif
    
    printf("Using loop limit: %d\n", loop_limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_postdec(loop_limit);
    total_sum += test_while_compare_zero(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return something based on the result to prevent dead code elimination */
    return (total_sum > 0) ? 0 : 1;
}

/* Additional test with nested loops for more complex patterns */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int total = 0;
    
    for (int i = outer_limit; i > 0; i--) {
        int inner = inner_limit;
        while (inner-- > 0) {
            total += 29;
            asm volatile("" : "+r"(total) : : "memory");
        }
    }
    return total;
}

/* Test with volatile counter to prevent certain optimizations */
NO_OPT static int test_volatile_counter(int limit) {
    volatile int counter = limit;
    int sum = 0;
    
    while (counter > 0) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
        counter--;
    }
    return sum;
}
