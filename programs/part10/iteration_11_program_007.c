/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could interfere with pattern matching */
#define NO_OPTIMIZE __attribute__((noinline, noipa, noclone, optimize("O0")))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM) || defined(__thumb__)
#define ARCH_ARM 1
#elif defined(__AVR__)
#define ARCH_AVR 1
#elif defined(__powerpc__) || defined(__PPC__) || defined(__PPC64__)
#define ARCH_PPC 1
#elif defined(__mips__)
#define ARCH_MIPS 1
#else
#define ARCH_GENERIC 1
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000; /* Use volatile to prevent constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition */
NO_OPTIMIZE
static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3; /* Prevent loop elimination */
        asm volatile("" : "+r"(sum) : : "memory"); /* Memory barrier */
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NO_OPTIMIZE
static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement */
NO_OPTIMIZE
static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NO_OPTIMIZE
static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (int i = limit; i != 0; i = i - 1) pattern */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1; /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Loop variant 5: Do-while with decrement */
NO_OPTIMIZE
static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(--n > 0, 1));
    }
    return sum;
}

/* Loop variant 6: Counter in register with complex exit */
NO_OPTIMIZE
static int test_register_counter(int limit) {
    register int counter asm("r0") = limit; /* Hint to use register */
    int sum = 0;
    
    while (__builtin_expect(counter-- > 0, 1)) {
        sum += 17;
        /* Use counter in asm to ensure it's in register */
        asm volatile("" : "+r"(counter), "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Unsigned counter (common in doloop) */
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

/* Loop variant 8: Counter with volatile to force memory ops */
NO_OPTIMIZE
static int test_volatile_counter(int limit) {
    volatile int n = limit; /* Force memory operations */
    int sum = 0;
    int local_n;
    
    while (1) {
        local_n = n; /* Read from volatile */
        if (!__builtin_expect(local_n > 0, 1)) break;
        n = local_n - 1; /* Write to volatile */
        sum += 23;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Architecture-specific test selection */
NO_OPTIMIZE
static int run_architecture_tests(int limit) {
    int total = 0;
    
#if defined(ARCH_ARM) || defined(ARCH_AVR) || defined(ARCH_PPC) || defined(ARCH_MIPS)
    /* Run all tests for doloop-supported architectures */
    total += test_post_decrement(limit);
    total += test_pre_decrement(limit);
    total += test_for_decrement(limit);
    total += test_explicit_subtract(limit);
    total += test_do_while(limit);
    total += test_register_counter(limit);
    total += test_unsigned_counter((unsigned int)limit);
    total += test_volatile_counter(limit);
#else
    /* For generic architectures, run a subset */
    total += test_post_decrement(limit);
    total += test_for_decrement(limit);
    total += test_explicit_subtract(limit);
#endif
    
    return total;
}

/* Main function */
int main(void) {
    int loop_limit;
    
    /* Get non-constant loop limit to prevent compile-time evaluation */
#ifdef ARCH_AVR
    /* AVR has limited resources, use smaller loop */
    loop_limit = 100;
#else
    loop_limit = get_loop_limit();
#endif
    
    printf("Testing doloop patterns with limit = %d\n", loop_limit);
    
    /* Run tests and accumulate results */
    int result = run_architecture_tests(loop_limit);
    
    printf("Total result: %d\n", result);
    
    /* Return non-zero to indicate execution */
    return result != 0 ? 0 : 1;
}
