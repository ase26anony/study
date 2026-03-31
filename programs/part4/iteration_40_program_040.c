/* test_builtin_hooks.c - Comprehensive built-in function test to trigger targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Helper function with attributes that may interact with built-in processing */
static int __attribute__((noinline, noclone, used))
helper_with_builtins(int x) {
    volatile int result = 0;
    
    /* Use multiple built-ins in sequence */
    result += __builtin_abs(x);
    result += __builtin_popcount((unsigned int)x);
    
    /* Optimization barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Function with visibility and nothrow attributes */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_clz((unsigned int)x);
    }
    return 0;
}

/* Another static function using built-ins */
static int __attribute__((noinline))
test_overflow_builtins(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Test overflow built-ins */
    if (__builtin_add_overflow(a, b, &result)) {
        overflow = 1;
    } else {
        overflow = 0;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int vol_result = result;
    return vol_result + overflow;
}

/* Function using math built-ins */
static float __attribute__((noinline))
test_math_builtins(float x) {
    volatile float accum = 0.0f;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        /* Use math built-in */
        accum += __builtin_sqrtf(x + i);
    }
    
    return accum;
}

/* Function that might trigger unreachable built-in */
static void __attribute__((noinline))
test_unreachable(int x) {
    if (x < 0) {
        /* This should never happen with our inputs */
        __builtin_unreachable();
    }
}

/* External function definition (simulating separate compilation unit) */
int external_builtin_user(int x) {
    int result = 0;
    
    /* Use built-in with external linkage */
    result = __builtin_ffs(x);
    
    /* Call to hidden visibility function */
    result += hidden_visibility_func(x);
    
    return result;
}

/* Main test function with various built-in patterns */
int main(int argc, char *argv[]) {
    volatile int seed = argc;  /* Prevent constant folding */
    int checksum = 0;
    
    /* Initialize with time to add randomness */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Test 1: Arithmetic built-ins */
    checksum += helper_with_builtins(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += __builtin_popcount(seed * 2);
    checksum += __builtin_clz(seed > 0 ? seed : 1);
    
    /* Test 3: Overflow built-ins */
    checksum += test_overflow_builtins(seed, 100);
    
    /* Test 4: Math built-ins */
    checksum += (int)test_math_builtins((float)seed + 1.0f);
    
    /* Test 5: External function with built-ins */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Expect built-in for branch prediction */
    if (__builtin_expect(checksum > 0, 1)) {
        checksum += 1;
    }
    
    /* Test 7: Unreachable built-in (shouldn't execute) */
    test_unreachable(seed);
    
    /* Test 8: More built-ins with volatile barriers */
    {
        volatile int a = seed;
        volatile int b = checksum;
        int c;
        
        /* Use mul_overflow built-in */
        if (__builtin_mul_overflow(a, b, &c)) {
            checksum += 1000;
        } else {
            checksum += c % 100;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Test 9: Built-in with attribute combination */
    {
        /* Declare built-in prototype with attributes */
        int __attribute__((visibility("hidden"))) 
        (*builtin_ptr)(int) = (int (*)(int))__builtin_abs;
        
        checksum += builtin_ptr(seed);
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* Additional function to increase declaration count */
static void __attribute__((constructor, used))
init_function(void) {
    /* Use built-in in constructor */
    volatile int init_val = __builtin_abs(-42);
    (void)init_val;
}

/* Another static function with different attributes */
static int __attribute__((cold, noinline))
cold_function_with_builtin(int x) {
    /* Use builtin in cold function */
    return __builtin_ctz(x > 0 ? x : 1);
}
