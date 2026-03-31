/* loop-doloop-test.c - Test program for GCC's doloop optimization */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPT __attribute__((noinline,noipa,noclone))

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
  #define ARCH_SUPPORTS_DOLOOP 0
#endif

/* Test function 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory"); /* Memory barrier */
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

/* Test function 4: Explicit subtract operation */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: for (; i != 0; i = i - 1) */
    for (; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
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

/* Test function 6: Unsigned counter (common in doloop patterns) */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- != 0) */
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 7: Complex expression in compare */
NO_OPT static int test_complex_compare(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while ((n = n - 1) >= 0) */
    while (__builtin_expect((n = n - 1) >= 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Counter in register variable */
NO_OPT static int test_register_var(int limit) {
    register int n asm("r0") = limit; /* Suggest register for counter */
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
    
#if ARCH_SUPPORTS_DOLOOP
    /* For doloop-supported architectures, use a volatile read */
    loop_limit = base_limit;
    
    /* Architecture-specific hints */
    #ifdef ARCH_ARM
        asm volatile("" : "=r"(loop_limit) : "0"(loop_limit) : "memory");
    #elif defined(ARCH_AVR)
        /* AVR-specific: ensure 16-bit operations */
        loop_limit = (int)((uint16_t)base_limit);
    #endif
#else
    /* For non-doloop architectures, still test but with smaller limit */
    loop_limit = 10;
#endif
    
    /* Run all test functions */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_complex_compare(loop_limit);
    total_sum += test_register_var(loop_limit);
    
    /* Make result observable */
    printf("Total sum: %d\n", total_sum);
    
    /* Also return as exit code for scriptable testing */
    return total_sum & 0xFF; /* Return lower 8 bits as exit code */
}

/* Additional test with different optimization barriers */
NO_OPT static int test_volatile_counter(int limit) {
    volatile int n = limit; /* Volatile counter */
    int sum = 0;
    
    while (n > 0) {
        sum += 29;
        n = n - 1; /* Explicit decrement */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test with pointer-based counter (less likely but covers more cases) */
NO_OPT static int test_pointer_counter(int limit) {
    int counter = limit;
    int *p = &counter;
    int sum = 0;
    
    while (__builtin_expect(*p > 0, 1)) {
        sum += 31;
        (*p)--;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
