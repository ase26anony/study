/* test_builtin_hooks.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimization barriers */
static volatile int global_barrier = 0;

/* External function declaration with attributes that may trigger the hook */
extern int external_builtin_user(int x) 
    __attribute__((visibility("hidden"), nothrow, used));

/* Function with multiple attributes to trigger flag setting */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
attributed_builtin_wrapper(int x) {
    /* Use __builtin_expect to potentially trigger the hook */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Another static function with different builtins */
static int __attribute__((noinline, noclone))
test_arithmetic_builtins(volatile int seed) {
    int result = 0;
    volatile int i;
    
    for (i = 0; i < 5; i++) {
        /* Use multiple arithmetic builtins */
        int val = seed + i;
        result += __builtin_abs(val);
        
        /* Use bit manipulation builtins */
        if (val != 0) {
            result += __builtin_popcount((unsigned int)val);
            result += __builtin_clz((unsigned int)val);
        }
    }
    
    return result;
}

/* Function using overflow builtins */
static int __attribute__((noinline, noclone))
test_overflow_builtins(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Test add overflow */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1;
    }
    result += overflow;
    
    /* Test mul overflow */
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 2;
    }
    result += overflow;
    
    return result;
}

/* Function with unreachable builtin */
static int __attribute__((noinline, noclone))
test_unreachable_builtin(int x) {
    if (x < 0) {
        /* This should trigger unreachable path */
        __builtin_unreachable();
    }
    return x * 2;
}

/* External function definition (simulating multi-file scope) */
int external_builtin_user(int x) {
    /* Use various builtins in external function */
    int result = __builtin_abs(x);
    
    /* Use expect builtin with volatile to prevent optimization */
    volatile int v = x;
    if (__builtin_expect(v > 100, 0)) {
        result += __builtin_popcount((unsigned int)v);
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Main test function with complex builtin usage */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int checksum = 0;
    
    /* Initialize with time to add randomness */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Test 1: Arithmetic builtins in loop */
    checksum += test_arithmetic_builtins(seed);
    
    /* Test 2: Overflow builtins */
    checksum += test_overflow_builtins(seed, seed + 1);
    
    /* Test 3: Attributed function with builtins */
    checksum += attributed_builtin_wrapper(seed);
    
    /* Test 4: External function with builtins */
    checksum += external_builtin_user(seed);
    
    /* Test 5: Direct builtin usage with volatile */
    volatile int direct_val = seed * 2;
    checksum += __builtin_abs(direct_val);
    checksum += __builtin_popcount((unsigned int)direct_val);
    
    /* Test 6: Unreachable builtin */
    if (seed > 0) {
        checksum += test_unreachable_builtin(seed);
    }
    
    /* Test 7: Builtin with assembly barrier */
    int tmp = seed;
    asm volatile("" : "+r"(tmp) : : "memory");
    checksum += __builtin_clz((unsigned int)tmp);
    
    /* Test 8: Multiple builtins in complex expression */
    checksum += __builtin_sqrtf((float)(seed + 1)) + 
                __builtin_ffs(seed | 1) +
                __builtin_parity((unsigned int)seed);
    
    /* Ensure result is used */
    global_barrier = checksum;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Additional function with visibility attribute */
static int __attribute__((visibility("hidden"), used))
hidden_visibility_func(int x) {
    /* Use builtin that might trigger the hook */
    return __builtin_ctz(x | 1);
}

/* Force emission of unused but attributed function */
static void __attribute__((used))
force_emission(void) {
    /* Call hidden function to ensure it's not eliminated */
    volatile int dummy = hidden_visibility_func(42);
    (void)dummy;
}

/* Constructor to force early initialization */
static void __attribute__((constructor))
init_constructor(void) {
    /* Use builtin in constructor */
    volatile int init_val = __builtin_bswap32(0x12345678);
    global_barrier += init_val;
}
