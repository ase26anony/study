/* test_builtin_hooks.c - Comprehensive test for GCC built-in function hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void unreachable_helper(int x) __attribute__((nothrow));

/* Volatile variables to prevent optimization */
static volatile int global_seed = 42;
static volatile int result_accumulator = 0;

/* Helper with multiple attributes that may interact with hook */
static int attribute_helper(int x) 
    __attribute__((visibility("hidden"), nothrow, used, noinline, noclone));

/* Function with explicit hidden visibility */
int __attribute__((visibility("hidden"))) 
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Static function with used attribute */
static void __attribute__((used, noinline))
force_used_function(void) {
    /* This function must be emitted due to 'used' attribute */
    volatile int dummy = __builtin_clz(global_seed);
    (void)dummy;
}

/* Test arithmetic built-ins */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int sum = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent optimization */
    for (i = 0; i < 5; i++) {
        /* Use different arithmetic built-ins */
        int val = seed + i;
        sum += __builtin_abs(val);
        
        /* Use floating built-in with cast */
        if (val > 0) {
            float fval = (float)val;
            /* __builtin_sqrtf may trigger built-in declaration */
            sum += (int)__builtin_sqrtf(fval);
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return sum;
}

/* Test bit operation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Declare built-in prototype locally */
    int __builtin_popcount(unsigned int);
    
    /* Use the declared built-in */
    result += __builtin_popcount(seed);
    result += __builtin_popcount(seed ^ 0xAAAAAAAA);
    
    /* Count leading zeros */
    if (seed != 0) {
        result += __builtin_clz(seed);
    }
    
    /* Count trailing zeros */
    result += __builtin_ctz(seed | 1);  /* |1 to avoid undefined behavior */
    
    /* Store in volatile to ensure processing */
    volatile unsigned int vol_result = result;
    return (int)vol_result;
}

/* Test overflow built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    int overflow_result;
    bool overflow;
    
    /* Test addition overflow */
    overflow = __builtin_add_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow) {
        result |= 0x1000;
    }
    
    /* Test multiplication overflow */
    overflow = __builtin_mul_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow) {
        result |= 0x2000;
    }
    
    /* Test subtraction overflow */
    overflow = __builtin_sub_overflow(a, b, &overflow_result);
    result += overflow_result;
    if (overflow) {
        result |= 0x4000;
    }
    
    return result;
}

/* Attributed function that calls built-ins */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
attribute_helper(int x) {
    /* Mix of built-ins and barriers */
    int result = __builtin_abs(x);
    
    /* Use __builtin_expect with computation */
    if (__builtin_expect(x < 0, 0)) {
        result += __builtin_clz((unsigned int)(-x));
    }
    
    /* Optimization barrier */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Function with external linkage that uses built-ins */
int external_builtin_user(int x) {
    /* Use multiple built-ins */
    int result = __builtin_abs(x);
    
    /* Conditional built-in usage */
    if (x > 1000) {
        /* __builtin_unreachable under specific condition */
        if (x > INT_MAX) {
            __builtin_unreachable();
        }
        result += __builtin_popcount((unsigned int)x);
    }
    
    return result;
}

/* Function that may trigger unreachable */
void unreachable_helper(int x) __attribute__((nothrow)) {
    if (x < 0) {
        /* This should never happen in our test */
        __builtin_unreachable();
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    volatile int seed = global_seed + argc;
    int final_result = 0;
    
    /* Force emission of attributed function */
    force_used_function();
    
    /* Test 1: Arithmetic built-ins */
    int arith_result = test_builtin_arithmetic(seed);
    final_result ^= arith_result;
    
    /* Test 2: Bit operation built-ins */
    int bitops_result = test_builtin_bitops((unsigned int)seed);
    final_result ^= (bitops_result << 1);
    
    /* Test 3: Overflow built-ins */
    int overflow_result = test_builtin_overflow(seed, 17);
    final_result ^= overflow_result;
    
    /* Test 4: Hidden visibility function */
    int hidden_result = hidden_visibility_func(seed);
    final_result ^= hidden_result;
    
    /* Test 5: Attributed helper */
    int attr_result = attribute_helper(seed);
    final_result ^= attr_result;
    
    /* Test 6: External linkage function */
    int ext_result = external_builtin_user(seed);
    final_result ^= ext_result;
    
    /* Test 7: Unreachable helper */
    if (seed >= 0) {
        unreachable_helper(seed);
    }
    
    /* Additional built-in usage in main */
    volatile int main_builtin_result = 0;
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        main_builtin_result += __builtin_abs(seed);
    }
    
    /* Use __builtin_expect for control flow */
    if (__builtin_expect(final_result != 0, 1)) {
        main_builtin_result += final_result;
    }
    
    /* Use __builtin_assume_aligned */
    int array[4] = {0};
    int *aligned_ptr = (int*)__builtin_assume_aligned(array, 16);
    aligned_ptr[0] = main_builtin_result;
    
    /* Final result processing */
    final_result += main_builtin_result;
    
    /* Store to volatile to ensure all computation is preserved */
    result_accumulator = final_result;
    
    /* Return result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
