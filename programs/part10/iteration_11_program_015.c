/* test-doloop-pattern.c
 * Target: Trigger specific RTL pattern in GCC's doloop optimization
 * Pattern: SET of COMPARE where second operand is const0_rtx,
 *          first operand is PLUS with second operand -1,
 *          first operand of PLUS is a register.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent optimizations that could alter loop structure */
#define NO_OPT __attribute__((noinline,noipa,noclone))

/* Architecture detection and doloop support */
#ifdef __arm__
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "ARM"
#elif defined(__AVR__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "AVR"
#elif defined(__powerpc__) || defined(__PPC__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "PowerPC"
#elif defined(__mips__)
#define ARCH_SUPPORTS_DOLOOP 1
#define ARCH_NAME "MIPS"
#else
#define ARCH_SUPPORTS_DOLOOP 0
#define ARCH_NAME "Generic"
#endif

/* Function to get non-constant loop limit */
static int get_loop_limit(void) {
    volatile int limit = 1000;  /* Prevent constant propagation */
    return limit;
}

/* Loop variant 1: Post-decrement in condition */
NO_OPT static int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (n-- > 0) */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but simple loop body */
        sum += 3;  /* Prevent dead code elimination */
        asm volatile("" : "+r"(sum) : : "memory");  /* Opaque operation */
    }
    
    return sum;
}

/* Loop variant 2: Pre-decrement in condition */
NO_OPT static int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: while (--n > 0) */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    /* Handle last iteration for comparison with zero */
    if (n == 0) {
        sum += 5;
    }
    
    return sum;
}

/* Loop variant 3: For loop with decrement */
NO_OPT static int test_for_decrement(int limit) {
    int sum = 0;
    
    /* Pattern: for (int i = limit; i > 0; i--) */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Loop variant 4: Explicit subtract in loop */
NO_OPT static int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    /* Pattern: while (i != 0) { i = i - 1; } */
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, not i-- or --i */
    }
    
    return sum;
}

/* Loop variant 5: Do-while with decrement */
NO_OPT static int test_do_while_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pattern: do { ... } while (n-- > 0); */
    if (n > 0) {
        do {
            sum += 13;
            asm volatile("" : "+r"(sum) : : "memory");
        } while (__builtin_expect(n-- > 0, 1));
    }
    
    return sum;
}

/* Loop variant 6: Unsigned counter (common in doloop) */
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

/* Loop variant 7: Counter in register variable */
NO_OPT static int test_register_counter(int limit) {
    register int i asm("r0") = limit;  /* Suggest register allocation */
    int sum = 0;
    
    /* Pattern: for (; i > 0; i--) */
    for (; __builtin_expect(i > 0, 1); i--) {
        sum += 19;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* Architecture-specific test for ARM */
#ifdef __arm__
NO_OPT static int test_arm_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* ARM often uses subs instruction for decrement and compare */
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 23;
        /* ARM-specific asm to ensure certain register usage */
        asm volatile("add %0, %0, #23" : "+r"(sum) : : "cc");
    }
    
    return sum;
}
#endif

/* Architecture-specific test for AVR */
#ifdef __AVR__
NO_OPT static int test_avr_specific(int limit) {
    int n = limit;
    int sum = 0;
    
    /* AVR has specific decrement and branch instructions */
    for (int i = limit; __builtin_expect(i > 0, 1); i--) {
        sum += 29;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    printf("Testing doloop patterns on %s architecture\n", ARCH_NAME);
    
    /* Get non-constant loop limit to prevent constant propagation */
#if ARCH_SUPPORTS_DOLOOP
    /* Use volatile or system call for supported architectures */
    volatile int vol_limit = 1000;
    loop_limit = vol_limit;
#else
    /* For generic architectures, use function return value */
    loop_limit = get_loop_limit();
#endif
    
    printf("Loop limit: %d\n", loop_limit);
    
    /* Execute all loop variants */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_decrement(loop_limit);
    total_sum += test_unsigned_decrement((unsigned int)loop_limit);
    total_sum += test_register_counter(loop_limit);
    
    /* Architecture-specific tests */
#ifdef __arm__
    total_sum += test_arm_specific(loop_limit);
#endif
    
#ifdef __AVR__
    total_sum += test_avr_specific(loop_limit);
#endif
    
    printf("Total sum: %d\n", total_sum);
    
    /* Return value based on sum to ensure all loops executed */
    return (total_sum > 0) ? 0 : 1;
}
