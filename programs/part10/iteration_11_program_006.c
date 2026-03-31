/* loop-doloop-test.c - Test program for GCC doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Prevent inlining and IPA transformations */
#define NOOPT __attribute__((noinline,noipa,noclone))

/* Architecture detection */
#if defined(__arm__) || defined(__ARM_ARCH_ISA_ARM)
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

/* Test 1: Post-decrement in condition - while (n-- > 0) */
NOOPT int test_post_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Use __builtin_expect to hint loop behavior without breaking pattern */
    while (__builtin_expect(n-- > 0, 1)) {
        /* Non-trivial but safe loop body */
        sum += 3;  /* Simple arithmetic that won't be optimized away */
        /* Prevent compiler from moving operations across iterations */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 2: Pre-decrement in condition - while (--n > 0) */
NOOPT int test_pre_decrement(int limit) {
    int n = limit;
    int sum = 0;
    
    /* Pre-decrement pattern */
    while (__builtin_expect(--n > 0, 1)) {
        sum += 5;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    /* Handle last iteration for n == limit case */
    if (limit > 0) {
        sum += 5;
    }
    return sum;
}

/* Test 3: For loop with decrement - for (i = limit; i > 0; i--) */
NOOPT int test_for_loop_decrement(int limit) {
    int sum = 0;
    
    for (int i = limit; i > 0; i--) {
        sum += 7;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 4: Explicit subtract pattern - i = i - 1 */
NOOPT int test_explicit_subtract(int limit) {
    int i = limit;
    int sum = 0;
    
    while (__builtin_expect(i != 0, 1)) {
        sum += 11;
        asm volatile("" : "+r"(sum) : : "memory");
        i = i - 1;  /* Explicit subtract, should generate PLUS with -1 */
    }
    return sum;
}

/* Test 5: Do-while with post-decrement */
NOOPT int test_do_while_postdec(int limit) {
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

/* Test 6: Unsigned counter pattern */
NOOPT unsigned int test_unsigned_counter(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    
    while (__builtin_expect(n-- > 0, 1)) {
        sum += 17;
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Test 7: Complex pattern with multiple decrements (should not match doloop) */
NOOPT int test_complex_pattern(int limit) {
    int n = limit;
    int m = limit / 2;
    int sum = 0;
    
    while (n > 0 && m > 0) {
        sum += n;
        asm volatile("" : "+r"(sum) : : "memory");
        n--;
        m -= 2;
    }
    return sum;
}

/* Get a non-constant loop limit to prevent constant propagation */
static int get_loop_limit(void) {
    volatile int external_limit = 1000;  /* Volatile to prevent constant folding */
    
    #if ARCH_SUPPORTS_DOLOOP
        /* For doloop targets, use a value that ensures multiple iterations */
        return external_limit;
    #else
        /* For generic targets, smaller limit but still multiple iterations */
        return 100;
    #endif
}

int main(void) {
    int total_sum = 0;
    int loop_limit;
    
    /* Get non-constant loop limit */
    loop_limit = get_loop_limit();
    
    printf("Testing doloop pattern matching on ");
    
    #ifdef ARCH_ARM
        printf("ARM architecture\n");
        printf("Compile with: -O2 -march=armv7-a -fdump-rtl-loop2 -fdump-rtl-doloop2\n");
    #elif defined(ARCH_AVR)
        printf("AVR architecture\n");
        printf("Compile with: -O2 -mmcu=atmega328p -S -o- | grep -i doloop\n");
    #elif defined(ARCH_PPC)
        printf("PowerPC architecture\n");
        printf("Compile with: -O2 -fdump-rtl-loop2 -fdump-rtl-doloop2\n");
    #elif defined(ARCH_MIPS)
        printf("MIPS architecture\n");
        printf("Compile with: -O2 -fdump-rtl-loop2 -fdump-rtl-doloop2\n");
    #else
        printf("generic architecture (doloop may not be enabled)\n");
        printf("Compile with: -O2 -fdump-rtl-all -da\n");
    #endif
    
    printf("Loop limit: %d\n\n", loop_limit);
    
    /* Execute all test patterns */
    total_sum += test_post_decrement(loop_limit);
    total_sum += test_pre_decrement(loop_limit);
    total_sum += test_for_loop_decrement(loop_limit);
    total_sum += test_explicit_subtract(loop_limit);
    total_sum += test_do_while_postdec(loop_limit);
    total_sum += test_unsigned_counter((unsigned int)loop_limit);
    total_sum += test_complex_pattern(loop_limit);
    
    printf("Total sum from all loops: %d\n", total_sum);
    
    /* Return non-zero if any test failed (simplified check) */
    if (total_sum == 0) {
        return 1;
    }
    
    return 0;
}
