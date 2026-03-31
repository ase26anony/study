/* test_builtin_hooks.c - Comprehensive built-in function test to trigger targhooks.cc flags */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* Prevent optimization of helper functions */
#define NO_OPT __attribute__((noinline, noclone))

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));

/* Function with multiple attributes that may interact with built-in processing */
static int hidden_used_function(int x) 
    __attribute__((visibility("hidden"), used, nothrow, artificial));

/* Built-in function prototype declaration */
int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
int __builtin_clz(unsigned int);
int __builtin_ctz(unsigned int);
long __builtin_expect(long, long);
int __builtin_abs(int) __attribute__((nothrow));
float __builtin_sqrtf(float);

/* Global volatile to prevent constant folding */
volatile int global_seed = 42;

/* Barrier to prevent optimization */
static inline void optimization_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Test 1: Arithmetic built-ins with volatile context */
NO_OPT int test_builtin_arithmetic(volatile int seed) {
    volatile int result = 0;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (volatile int i = 0; i < 3; i++) {
        /* Use multiple arithmetic built-ins */
        int val = seed + i * 100;
        result += __builtin_abs(val);
        
        /* Use floating built-in with cast */
        float fval = (float)val;
        result += (int)__builtin_sqrtf(fval > 0 ? fval : 1.0f);
        
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect(val > 0, 1)) {
            result += 1;
        }
    }
    
    optimization_barrier();
    return result;
}

/* Test 2: Bit operation built-ins */
NO_OPT int test_builtin_bitops(unsigned int seed) {
    volatile unsigned int accum = 0;
    
    /* Process through several bit manipulation built-ins */
    accum += __builtin_popcount(seed);
    accum += __builtin_clz(seed | 1);  /* Ensure non-zero */
    accum += __builtin_ctz(seed | 1);
    
    /* Use built-in with attribute in declaration */
    unsigned int mask = 0xAAAAAAAA;
    accum += __builtin_popcount(seed & mask);
    
    optimization_barrier();
    return (int)accum;
}

/* Test 3: Overflow checking built-ins */
NO_OPT int test_builtin_overflow(int a, int b) {
    volatile int result = 0;
    int overflow_result;
    bool overflow;
    
    /* Test addition overflow */
    overflow = __builtin_add_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow) {
        result += 1000;
    }
    
    /* Test multiplication overflow */
    overflow = __builtin_mul_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow) {
        result += 2000;
    }
    
    /* Test subtraction overflow */
    overflow = __builtin_sub_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (__builtin_expect(overflow, 0)) {
        result += 3000;
    }
    
    optimization_barrier();
    return result;
}

/* Test 4: Attributed static function calling built-ins */
static int hidden_used_function(int x) {
    /* This function has multiple attributes and uses built-ins */
    volatile int r = 0;
    
    /* Use __builtin_expect with attributes */
    if (__builtin_expect(x > 0, 1)) {
        r = __builtin_abs(x);
    } else {
        r = __builtin_abs(x) * -1;
    }
    
    /* Use __builtin_unreachable for impossible path */
    if (x < INT_MIN) {
        __builtin_unreachable();
    }
    
    optimization_barrier();
    return r;
}

/* Test 5: External linkage simulation */
NO_OPT int test_external_linkage(int x) {
    /* Call externally declared function */
    return external_builtin_user(x);
}

/* Definition of the externally declared function */
int external_builtin_user(int x) __attribute__((visibility("hidden"))) {
    volatile int result = 0;
    
    /* Use various built-ins in external-like function */
    result += __builtin_popcount((unsigned int)x);
    
    /* Complex expression with built-in */
    int temp = __builtin_abs(x);
    if (__builtin_expect(temp > 100, 0)) {
        result += __builtin_clz((unsigned int)temp);
    } else {
        result += __builtin_ctz((unsigned int)(temp | 1));
    }
    
    /* Use __builtin_unreachable with condition */
    if (x == 0xDEADBEEF) {
        __builtin_unreachable();
    }
    
    optimization_barrier();
    return result;
}

/* Additional function with artificial attribute */
int __attribute__((artificial)) artificial_builtin_wrapper(int x) {
    return __builtin_abs(x) + __builtin_popcount((unsigned int)x);
}

/* Main function tying everything together */
int main(int argc, char *argv[]) {
    volatile int checksum = 0;
    
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc + global_seed;
    
    printf("Testing built-in function hooks...\n");
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, 17);
    checksum += test_builtin_overflow(INT_MAX / 2, 3);
    
    /* Test 4: Attributed function */
    checksum += hidden_used_function(seed);
    checksum += hidden_used_function(-seed);
    
    /* Test 5: External linkage */
    checksum += test_external_linkage(seed);
    
    /* Direct built-in calls with volatile storage */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed);
    direct_result += __builtin_popcount((unsigned int)seed);
    direct_result += __builtin_clz((unsigned int)(seed | 1));
    
    /* Use artificial attribute wrapper */
    checksum += artificial_builtin_wrapper(seed);
    
    /* Final optimization barrier and output */
    optimization_barrier();
    
    printf("Final checksum: %d\n", checksum + direct_result);
    
    /* Return checksum to ensure all code has effect */
    return (checksum + direct_result) & 0xFF;
}
