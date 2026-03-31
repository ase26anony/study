/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent function inlining to preserve loop structure */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__thumb__)
  #define ARCH_ARM 1
  #define ARCH_NAME "ARM"
#elif defined(__AVR__)
  #define ARCH_AVR 1
  #define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__) || defined(__ppc__)
  #define ARCH_PPC 1
  #define ARCH_NAME "PowerPC"
#elif defined(__mips__)
  #define ARCH_MIPS 1
  #define ARCH_NAME "MIPS"
#else
  #define ARCH_GENERIC 1
  #define ARCH_NAME "Generic"
#endif

/* Use volatile to prevent constant propagation */
static volatile int g_volatile_seed = 0;

/* Get a non-constant loop limit */
static int get_loop_limit(void) {
    /* Mix volatile read with system call for unpredictability */
    int base = g_volatile_seed;
    int pid = getpid();
    return (base ^ pid) & 0xFFF;  /* Limit to reasonable size */
}

/* Loop variant 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (n-- > 0) pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 0x1234;
        /* Prevent counter aliasing */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* while (--n > 0) pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 0x5678;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* for (int i = limit; i > 0; i--) pattern */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 0x9ABC;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* for (; i != 0; i = i - 1) pattern */
    for (; __builtin_expect(i != 0, 1); i = i - 1) {
        sum += 0xDEF0;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 5: Do-while with decrement */
NO_OPT static int test_do_while(int limit) {
    int n = limit;
    int sum = 0;
    
    if (n > 0) {
        do {
            sum += 0x2468;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(--n > 0, 1));
    }
    return sum;
}

/* Loop variant 6: Unsigned counter (common in doloop) */
NO_OPT static int test_unsigned_decrement(unsigned int limit) {
    unsigned int n = limit;
    int sum = 0;
    
    while (__builtin_expect(n-- != 0, 1)) {
        sum += 0x1357;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int i asm("r0") = limit;  /* Suggest register, but compiler may ignore */
    int sum = 0;
    
    for (; i > 0; i--) {
        sum += 0xFACE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Loop variant 8: Nested loops to test multiple patterns */
NO_OPT static int test_nested_loops(int outer_limit, int inner_limit) {
    int outer_sum = 0;
    
    for (int o = outer_limit; o > 0; o--) {
        int inner_sum = 0;
        int i = inner_limit;
        
        /* Inner loop with different pattern */
        while (i-- > 0) {
            inner_sum += 0xBEEF;
            asm volatile("" : "+r"(inner_sum) : : "memory");
        }
        outer_sum += inner_sum;
    }
    return outer_sum;
}

/* Architecture-specific optimizations */
#if defined(ARCH_ARM)
/* ARM-specific: Use register constraints that might trigger doloop */
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often optimizes count-down-to-zero loops */
    asm volatile("" : "+r"(n) : : "memory");
    
    while (n--) {
        sum += 0xCAFE;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

#if defined(ARCH_AVR)
/* AVR-specific: Small loops are common targets for doloop */
NO_OPT static int test_avr_specific(uint8_t limit) {
    uint8_t n = limit;
    int sum = 0;
    
    for (; n != 0; n--) {
        sum += 0xDEAD;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}
#endif

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop pattern matching on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent constant propagation */
    loop_limit = get_loop_limit();
    if (loop_limit <= 0) loop_limit = 100;  /* Ensure positive limit */
    
    printf("Loop limit: %d\n", loop_limit);
    
    /* Execute all loop variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    total_sum += test_nested_loops(loop_limit / 10 + 1, 10);
    
    /* Architecture-specific tests */
#if defined(ARCH_ARM)
    total_sum += test_arm_specific(loop_limit);
    printf("ARM-specific test executed\n");
#endif
    
#if defined(ARCH_AVR)
    total_sum += test_avr_specific((uint8_t)(loop_limit & 0xFF));
    printf("AVR-specific test executed\n");
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total_sum == 0) ? 1 : 0;
}
