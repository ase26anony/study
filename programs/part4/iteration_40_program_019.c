/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_helper(unsigned int val) __attribute__((used, nothrow));

/* Prevent optimization barriers */
#define OPT_BARRIER() asm volatile("" : : : "memory")

/* Function with explicit hidden visibility and multiple attributes */
__attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
static int hidden_visibility_func(int x) {
    volatile int result = 0;
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        result = __builtin_abs(x);
    } else {
        result = __builtin_abs(x) * -1;
    }
    OPT_BARRIER();
    return result;
}

/* Test arithmetic built-ins with volatile prevention */
__attribute__((noinline, noclone))
static int test_builtin_arithmetic(volatile int seed) {
    volatile int acc = 0;
    
    for (volatile int i = 0; i < 5; i++) {
        /* Use various math built-ins */
        int val = seed + i;
        acc += __builtin_abs(val);
        
        /* Use floating-point built-in */
        float fval = (float)val;
        int sqrt_approx = (int)__builtin_sqrtf(fval);
        acc += sqrt_approx;
        
        /* Use count leading zeros built-in */
        if (val > 0) {
            acc += __builtin_clz(val);
        }
    }
    
    OPT_BARRIER();
    return acc;
}

/* Test bit operation built-ins */
__attribute__((noinline, noclone))
static int test_builtin_bitops(unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Declare built-in prototype to potentially trigger hook */
    int __builtin_popcount(unsigned int);
    
    /* Use popcount built-in */
    result += __builtin_popcount(seed);
    result += __builtin_popcount(seed ^ 0xAAAAAAAA);
    
    /* Use parity built-in */
    result += __builtin_parity(seed);
    
    /* Use byte swap built-in */
    result += __builtin_bswap32(seed) & 0xFF;
    
    OPT_BARRIER();
    return (int)result;
}

/* Test overflow checking built-ins */
__attribute__((noinline, noclone))
static int test_builtin_overflow(int a, int b) {
    volatile int result = 0;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result = __builtin_abs(a);
    } else {
        result = overflow;
    }
    
    /* Test multiplication overflow */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result += __builtin_abs(b);
    } else {
        result += mul_result;
    }
    
    /* Test subtraction overflow */
    int sub_result;
    if (__builtin_sub_overflow(a, b, &sub_result)) {
        result += 1000;
    } else {
        result += sub_result;
    }
    
    OPT_BARRIER();
    return result;
}

/* Static function with used attribute calling built-ins */
__attribute__((used, nothrow, noinline))
static int static_builtin_helper(unsigned int val) {
    volatile int result = 0;
    
    /* Mix of built-ins */
    result = __builtin_ffs(val);          /* Find first set */
    result += __builtin_ctz(val);         /* Count trailing zeros */
    
    /* Use expect with probability */
    if (__builtin_expect((val & 1) == 0, 0)) {
        result += __builtin_clz(val);
    }
    
    OPT_BARRIER();
    return result;
}

/* External function definition (simulating multi-file scope) */
__attribute__((visibility("hidden"), nothrow))
int external_builtin_user(int x) {
    volatile int result = 0;
    
    /* Complex expression with built-in */
    result = __builtin_abs(x) * 2;
    
    /* Use unreachable built-in under condition */
    if (x < 0) {
        __builtin_unreachable();
        /* The following code should never be reached */
        result = -1;
    }
    
    /* Use prefetch built-in */
    __builtin_prefetch(&result, 0, 3);
    
    OPT_BARRIER();
    return result;
}

/* Function with assembly and built-in mixing */
__attribute__((noinline, noclone))
static int test_mixed_assembly(volatile int seed) {
    int result = seed;
    
    /* Assembly barrier followed by built-in */
    asm volatile("nop" : : : "memory");
    result = __builtin_abs(result);
    
    /* More assembly */
    asm volatile("" : : : "memory");
    result += __builtin_ffs(result);
    
    /* Inline assembly that uses built-in result */
    int temp;
    asm volatile("mov %1, %0\n\t"
                 "add $1, %0"
                 : "=r"(temp)
                 : "r"(__builtin_clz(result)));
    result += temp;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int checksum = 0;
    
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)time(NULL);
    
    if (seed == 0) seed = 1;
    
    printf("Starting built-in function coverage test...\n");
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(useed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Hidden visibility function */
    checksum += hidden_visibility_func(seed);
    
    /* Test 5: Static helper with used attribute */
    checksum += static_builtin_helper(useed);
    
    /* Test 6: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Mixed assembly and built-ins */
    checksum += test_mixed_assembly(seed);
    
    /* Additional direct built-in usage in main */
    checksum += __builtin_abs(checksum);
    
    /* Use expect to influence control flow */
    if (__builtin_expect(checksum > 0, 1)) {
        checksum += __builtin_clz(checksum);
    }
    
    /* Final result output */
    printf("Final checksum: %d\n", checksum);
    
    /* Use built-in for return value adjustment */
    return __builtin_abs(checksum) % 255;
}
