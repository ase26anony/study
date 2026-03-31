/* test_builtin_hooks.c - Comprehensive built-in function usage to trigger targhooks.cc flags */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_wrapper(int x) __attribute__((used, noinline, noclone));

/* Volatile variables to prevent optimization */
volatile int vol_seed = 42;
volatile int vol_result = 0;
volatile int vol_counter = 0;

/* Function with explicit hidden visibility and nothrow attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function with artificial control flow using builtins */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(int seed) {
    volatile int acc = 0;
    
    for (volatile int i = 0; i < 5; i++) {
        /* Use math builtins with volatile iteration */
        int val = seed + i;
        acc += __builtin_abs(val);
        
        /* Use sqrt builtin with float conversion */
        float fval = (float)val;
        acc += (int)__builtin_sqrtf(fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return acc;
}

/* Function using bit manipulation builtins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    volatile unsigned int result = 0;
    
    /* Use various bit operation builtins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Use parity builtin */
    result += __builtin_parity(seed);
    
    /* Store to volatile to ensure side effect */
    vol_result = (int)result;
    
    return (int)result;
}

/* Function using overflow checking builtins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    volatile int overflow_detected = 0;
    int result;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        overflow_detected = 1;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &result)) {
        overflow_detected |= 2;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &result)) {
        overflow_detected |= 4;
    }
    
    /* Use builtin to select value based on overflow */
    return __builtin_choose_expr(overflow_detected == 0, a + b, INT_MAX);
}

/* Function using control flow builtins */
static void __attribute__((noinline, noclone))
test_builtin_control_flow(int x) {
    /* Use unreachable builtin in conditional path */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use prefetch builtin */
    __builtin_prefetch(&vol_seed, 0, 3);
    
    /* Use assume builtin */
    __builtin_assume(x >= 0);
}

/* Static wrapper that uses builtins */
static int __attribute__((used, noinline, noclone))
static_builtin_wrapper(int x) {
    /* Mix of builtins in complex expression */
    int result = __builtin_abs(x) + 
                 __builtin_popcount((unsigned int)x) +
                 (__builtin_expect(x != 0, 1) ? 1 : 0);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* External function definition (simulating multi-file scope) */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int x) {
    /* Use builtin in external visible function */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_clz((unsigned int)x);
    }
    
    /* Potentially unreachable code with builtin */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    return x;
}

/* Function using synchronization builtins */
static int __attribute__((noinline, noclone))
test_builtin_sync(int seed) {
    volatile int sync_var = seed;
    
    /* Use atomic builtins */
    __sync_fetch_and_add(&sync_var, 1);
    int old = __sync_lock_test_and_set(&sync_var, seed * 2);
    __sync_synchronize();
    
    return old + sync_var;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int checksum = 0;
    
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc + vol_seed;
    
    /* Test 1: Arithmetic builtins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation builtins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow builtins */
    checksum += test_builtin_overflow(seed, 1000);
    
    /* Test 4: Hidden visibility function with builtins */
    checksum += hidden_visibility_func(seed);
    
    /* Test 5: Static wrapper with builtins */
    checksum += static_builtin_wrapper(seed);
    
    /* Test 6: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Control flow builtins */
    test_builtin_control_flow(seed);
    
    /* Test 8: Synchronization builtins */
    checksum += test_builtin_sync(seed);
    
    /* Test 9: Direct builtin calls in main */
    checksum += __builtin_abs(seed);
    checksum += __builtin_popcount((unsigned int)checksum);
    
    /* Use expect builtin for final condition */
    if (__builtin_expect(checksum != 0, 1)) {
        /* Use printf to ensure side effects */
        printf("Checksum: %d\n", checksum);
    } else {
        /* Unreachable with builtin */
        __builtin_unreachable();
    }
    
    /* Additional builtin usage in return path */
    return __builtin_ffs(checksum | 1) - 1;
}

/* Additional global declaration with builtin prototype */
int __attribute__((used))
declare_builtin_prototype(void) {
    /* Declare builtin function prototype */
    extern int __builtin_ctz(unsigned int);
    extern int __builtin_popcountll(unsigned long long);
    
    /* Use them */
    return __builtin_ctz(vol_seed) + __builtin_popcountll(vol_seed);
}
