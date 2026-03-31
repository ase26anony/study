/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone,optimize("O0")))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
  #define ARCH_PPC 1
  #define DOLOOP_SUPPORTED 1
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define DOLOOP_SUPPORTED 1
#else
  #define DOLOOP_SUPPORTED 0
#endif

/* Test function 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- or --i */
    }
    return sum;
}

/* Test function 5: Do-while with decrement */
NO_OPT static int test_do_while(int limit) {
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

/* Test function 6: Count down to zero with unsigned */
NO_OPT static int test_unsigned_decrement(int limit) {
    unsigned int n = (unsigned int)limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Complex decrement pattern */
NO_OPT static int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while ((n = n - 1) >= 0) */
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Nested loops to stress pattern matching */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer = outer_limit;
    int sum = 0;
    
    while (__builtin_expect(outer-- > 0, 1)) {
        int inner = inner_limit;
        while (__builtin_expect(inner-- > 0, 1)) {
            sum += 23;
            asm volatile("" : "+r"(sum) : : "memory");
        }
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    /* Use volatile or external function to prevent constant propagation */
    volatile int base_limit = 100;
    int limit = base_limit;
    
    /* Alternative: use system call for non-constant value */
    #if DOLOOP_SUPPORTED
    int pid_limit = getpid() & 0xFF;  /* Non-constant but bounded */
    if (pid_limit < 10) pid_limit = 100;  /* Ensure reasonable minimum */
    limit = pid_limit;
    #endif
    
    printf("Testing doloop patterns with limit = %d\n", limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(limit);
    total_sum += test_pre_decrement(limit);
    total_sum += test_for_decrement(limit);
    total_sum += test_explicit_subtract(limit);
    total_sum += test_do_while(limit);
    total_sum += test_unsigned_decrement(limit);
    total_sum += test_complex_decrement(limit);
    total_sum += test_nested_loops(limit / 10, 10);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to indicate execution */
    return total_sum != 0 ? 0 : 1;
}

/* Additional architecture-specific variants */
#if defined(ARCH_ARM)
/* ARM-specific loop patterns that might generate different RTL */
NO_OPT static int test_arm_specific(int limit) {
    register int n asm("r4") = limit;  /* Suggest register */
    int sum = 0;
    
    asm volatile("" : "+r"(n));  /* Force register usage */
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        /* ARM-specific inline asm to prevent optimizations */
        asm volatile("add %0, %0, #29" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific - often benefits from doloop optimization */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);
    int sum = 0;
    
    /* AVR often uses 8-bit counters */
    while (n--) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif
