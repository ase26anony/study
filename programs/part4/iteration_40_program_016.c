/* test_builtin_hooks.c - Comprehensive built-in function usage to trigger GCC hooks */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Helper function with attributes that may interact with built-in processing */
static int __attribute__((noinline, noclone, used))
helper_with_builtin(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function with explicit visibility and nothrow attributes */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    volatile int result = 0;
    /* Use multiple built-ins in sequence */
    result += __builtin_popcount(x);
    result += __builtin_clz(x);
    return result;
}

/* Function using overflow checking built-ins */
static int __attribute__((noinline))
test_overflow_builtins(int a, int b) {
    int sum, product;
    bool overflow1, overflow2;
    
    /* These built-ins return overflow status */
    overflow1 = __builtin_add_overflow(a, b, &sum);
    overflow2 = __builtin_mul_overflow(a, b, &product);
    
    /* Use results to prevent optimization */
    volatile int check = 0;
    if (overflow1) check |= 1;
    if (overflow2) check |= 2;
    
    return sum + product + check;
}

/* Function using math built-ins with volatile barriers */
static float __attribute__((noinline))
test_math_builtins(float x) {
    volatile float accumulator = 0.0f;
    
    /* Use sqrt built-in */
    accumulator += __builtin_sqrtf(x);
    
    /* Use fabs built-in */
    accumulator += __builtin_fabsf(x);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return accumulator;
}

/* Function that might trigger __builtin_unreachable */
static int __attribute__((noinline))
test_unreachable_builtin(int x) {
    switch (x & 3) {
        case 0: return x + 1;
        case 1: return x * 2;
        case 2: return x - 1;
        /* Compiler should know this is unreachable if x is properly constrained */
        default: __builtin_unreachable();
    }
}

/* External function definition (simulating another translation unit) */
int __attribute__((visibility("default")))
external_builtin_user(int x) {
    /* Use built-in with external linkage */
    int result = __builtin_ffs(x);  /* Find first set bit */
    
    /* Use __builtin_constant_p to check if argument is constant */
    if (__builtin_constant_p(x)) {
        result += 1000;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    volatile int seed = argc;  /* Prevent constant propagation */
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins with volatile context */
    for (volatile int i = 0; i < 3; i++) {
        checksum += helper_with_builtin(seed + i);
        checksum += hidden_visibility_func(seed + i * 7);
    }
    
    /* Test 2: Bit operation built-ins */
    checksum += __builtin_popcount(seed);
    checksum += __builtin_clz(seed | 1);  /* Ensure non-zero */
    checksum += __builtin_ctz(seed | 1);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_overflow_builtins(seed, seed * 2);
    checksum += test_overflow_builtins(seed, 1073741824);  /* Large number to potentially overflow */
    
    /* Test 4: Math built-ins */
    checksum += (int)test_math_builtins(seed * 1.5f);
    
    /* Test 5: Unreachable built-in */
    checksum += test_unreachable_builtin(seed & 3);  /* Ensure valid range */
    
    /* Test 6: External function with built-ins */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Built-in with assembly barrier */
    {
        volatile int a = seed;
        volatile int b = seed * 3;
        int sum;
        if (__builtin_add_overflow(a, b, &sum)) {
            checksum += 999;
        } else {
            checksum += sum;
        }
        asm volatile("" : : : "memory");
    }
    
    /* Test 8: Built-in for branch prediction */
    if (__builtin_expect(checksum > 0, 1)) {
        checksum += __builtin_abs(checksum);
    }
    
    /* Test 9: String built-in (different category) */
    {
        char buffer[32];
        int len = __builtin_snprintf(buffer, sizeof(buffer), "Seed: %d", seed);
        checksum += len;
    }
    
    /* Test 10: Type-generic built-in */
    checksum += __builtin_choose_expr(sizeof(int) == 4, 4, 0);
    
    /* Ensure result is used */
    printf("Checksum: %d\n", checksum);
    return checksum & 255;  /* Return non-zero to indicate execution */
}
