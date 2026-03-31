/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that would interfere with pattern matching */
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

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint at loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Note: This pattern might generate different RTL but still valid */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 3: For loop with decrement - for (i = limit; i > 0; i--) */
NO_OPT static int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract pattern - for (i = limit; i != 0; i = i - 1) */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- */
    }
    return sum;
}

/* Test 5: Do-while with post-decrement */
NO_OPT static int test_do_while_post(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += 13;
        asm volatile("" : "+r"(sum) : : "memory");
    } while (__builtin_expect(n-- > 1, 1));
    
    return sum;
}

/* Test 6: Unsigned counter (common in embedded systems) */
NO_OPT static unsigned int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 7: Counter in register variable (hint to keep in register) */
NO_OPT static int test_register_counter(int limit) {
    register int n asm("r0") = limit;  /* Architecture-specific register hint */
    int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 8: Nested loops to stress the pattern matcher */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer_sum = 0;
    
    for (int i = outer_limit; __builtin_expect(i > 0, 1); i--) {
        int inner_sum = 0;
        int j = inner_limit;
        
        while (__builtin_expect(j-- > 0, 1)) {
            inner_sum += 2;
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        
        outer_sum += inner_sum;
        asm volatile("" : "+r"(outer_sum) : : "memory");
    }
    
    return outer_sum;
}

/* Get a non-constant loop limit to prevent constant propagation */
static int get_loop_limit(void) {
    volatile int base_limit = 1000;  /* Prevent constant folding */
    
    /* Use system call or volatile to get unpredictable but bounded value */
    #if ARCH_SUPPORTS_DOLOOP
        /* For doloop targets, use a value that ensures multiple iterations */
        int pid = getpid();
        return (pid & 0xFF) + 100;  /* 100-355 iterations */
    #else
        /* For generic targets, just use a fixed but non-constant value */
        return base_limit;
    #endif
}

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get a non-constant loop limit */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop optimization patterns\n");
    printf("Architecture: ");
    
    #ifdef ARCH_ARM
        printf("ARM");
    #elif defined(ARCH_AVR)
        printf("AVR");
    #elif defined(ARCH_PPC)
        printf("PowerPC");
    #elif defined(ARCH_MIPS)
        printf("MIPS");
    #else
        printf("Generic");
    #endif
    
    printf(" (Doloop support: %s)\n", ARCH_SUPPORTS_DOLOOP ? "yes" : "no");
    printf("Loop limit: %d\n\n", loop_limit);
    
    /* Run all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_post(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    total_sum += test_nested_loops(loop_limit / 10, 10);
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero to ensure all code paths are considered */
    return total_sum != 0 ? 0 : 1;
}
