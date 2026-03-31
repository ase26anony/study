/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
#define ARCH_ARM 1
#define DOLOOP_SUPPORTED 1
#elif defined(__AVR__)
#define ARCH_AVR 1
#define DOLOOP_SUPPORTED 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
#define ARCH_PPC 1
#define DOLOOP_SUPPORTED 1
#elif defined(__mips__)
#define ARCH_MIPS 1
#define DOLOOP_SUPPORTED 1
#else
#define DOLOOP_SUPPORTED 0
#endif

/* Test function 1: Post-decrement in condition */
/* while (n-- > 0) pattern */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that can't be optimized away */
        /* Prevent compiler from optimizing out the loop */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 2: Pre-decrement in condition */
/* while (--n > 0) pattern */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Copy to local variable to ensure we don't modify the parameter directly */
    int counter = n;
    
    while (__builtin_expect(--counter > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 3: For loop with decrement */
/* for (int i = limit; i > 0; i--) pattern */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 4: Explicit subtract pattern */
/* for (int i = limit; i != 0; i = i - 1) pattern */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, should generate PLUS with -1 */
    }
    return sum;
}

/* Test function 5: Do-while with decrement */
/* do { ... } while (n-- > 0) pattern */
NO_OPT static int test_do_while_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    return sum;
}

/* Test function 6: Complex decrement pattern with multiple uses */
NO_OPT static int test_complex_decrement(int limit) {
    int n = limit;
    int sum = 0;
    int temp;
    
    while (__builtin_expect(n > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
        
        /* Complex decrement that might still generate PLUS -1 */
        temp = n;
        n = temp - 1;
    }
    return sum;
}

/* Test function 7: Unsigned counter pattern */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test function 8: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test functions */
#ifdef ARCH_ARM
/* ARM-specific: Use register constraints that might help pattern matching */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use inline asm to force specific register usage if needed */
    asm volatile("" : "+r"(n) : : "memory");
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 29;
        /* ARM-specific asm to prevent optimization */
        asm volatile("add %0, %0, #29" : "+r"(sum) : : "cc");
    }
    return sum;
}
#endif

#ifdef ARCH_AVR
/* AVR-specific: 8-bit counters might trigger different patterns */
NO_OPT static int test_avr_specific(int limit) {
    uint8_t n = (uint8_t)(limit & 0xFF);
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 31;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

/* Main function with architecture-aware execution */
int main(void) {
    int total_sum = 0;
    
    /* Get a non-constant loop limit to prevent constant propagation */
    volatile int base_limit = 100;
    int limit;
    
    /* Use system call or volatile to get unpredictable but reasonable value */
#if DOLOOP_SUPPORTED
    /* For doloop-supported architectures, use a more dynamic limit */
    limit = base_limit + (getpid() & 0x3F);  /* 0-63 variation */
#else
    /* For other architectures, still use non-constant but simpler */
    limit = base_limit + 10;
#endif
    
    printf("Testing doloop patterns with limit = %d\n", limit);
    printf("Doloop supported: %s\n", DOLOOP_SUPPORTED ? "YES" : "NO");
    
    /* Execute all test functions */
    total_sum += test_post_decrement(limit);
    total_sum += test_pre_decrement(limit);
    total_sum += test_for_loop_decrement(limit);
    total_sum += test_explicit_subtract(limit);
    total_sum += test_do_while_decrement(limit);
    total_sum += test_complex_decrement(limit);
    total_sum += test_unsigned_decrement((unsigned int)limit);
    total_sum += test_register_counter(limit);
    
#if defined(ARCH_ARM)
    total_sum += test_arm_specific(limit);
    printf("ARM-specific test executed\n");
#elif defined(ARCH_AVR)
    total_sum += test_avr_specific(limit);
    printf("AVR-specific test executed\n");
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Use the result to prevent dead code elimination */
    if (total_sum > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
