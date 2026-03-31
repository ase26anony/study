/* test_builtin_hooks.c - Comprehensive built-in function usage to trigger GCC target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int optimization_barrier = 0;

/* Helper function with attributes that may interact with built-in processing */
static int __attribute__((noinline, noclone, used))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int temp;
    
    /* Use math built-ins in a loop */
    for (volatile int i = 0; i < 5; i++) {
        /* __builtin_abs with volatile argument */
        temp = seed + i;
        result += __builtin_abs(temp);
        
        /* __builtin_sqrtf with type conversion */
        float fval = (float)(seed + i * 10);
        result += (int)__builtin_sqrtf(fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Function with explicit hidden visibility and nothrow attributes */
static void __attribute__((visibility("hidden"), nothrow, used))
test_attributed_function(volatile int seed) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(seed > 100, 0)) {
        global_counter += __builtin_popcount(seed);
    } else {
        global_counter += __builtin_clz(seed | 1);  /* Ensure non-zero */
    }
    
    /* Use __builtin_unreachable in a controlled manner */
    if (seed < 0) {
        __builtin_unreachable();  /* Should never happen with our inputs */
    }
}

/* Function using overflow checking built-ins */
static int __attribute__((noinline))
test_builtin_overflow(volatile int a, volatile int b) {
    int result = 0;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        result = __builtin_abs(a);
    }
    
    /* Test multiplication overflow */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result += __builtin_clz(mul_result);
    } else {
        result += mul_result;
    }
    
    /* Use __builtin_expect with the result */
    return __builtin_expect(result, 0) ? result : -result;
}

/* Function with bit manipulation built-ins */
static unsigned int __attribute__((noinline))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Chain multiple built-in operations */
    result = __builtin_popcount(seed);
    result += __builtin_ctz(seed | 1);  /* Avoid undefined behavior with 0 */
    result += __builtin_parity(seed);
    result += __builtin_ffs(seed);
    
    /* Use result in __builtin_expect */
    if (__builtin_expect(result > 32, 0)) {
        result = __builtin_rotateright32(result, 2);
    }
    
    return result;
}

/* External function definition (simulating cross-file reference) */
int external_builtin_user(int x) {
    /* Use various built-ins inside external function */
    int result = __builtin_abs(x);
    
    /* Conditional __builtin_unreachable */
    if (x == 0) {
        return result;
    } else if (x < 0) {
        result += __builtin_clz(-x);
    } else {
        /* This should always be true for our test */
        if (__builtin_expect(x > 0, 1)) {
            result += __builtin_popcount(x);
        }
    }
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden"), used))
hidden_visibility_func(void) {
    /* Use built-in with side effects */
    volatile int x = optimization_barrier;
    int y = __builtin_abs(x);
    
    /* Use __builtin_expect to influence code generation */
    if (__builtin_expect(y > 0, 1)) {
        global_counter += y;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int seed;
    
    /* Initialize seed from runtime sources */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFF;
    }
    
    /* Ensure seed is positive and non-zero */
    seed = (seed < 0) ? -seed : seed;
    seed = (seed == 0) ? 1 : seed;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, seed + 1);
    
    /* Test 4: Call attributed function */
    test_attributed_function(seed);
    checksum += global_counter;
    
    /* Test 5: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Hidden visibility function */
    hidden_visibility_func();
    checksum += global_counter;
    
    /* Additional direct built-in usage in main */
    checksum = __builtin_abs(checksum);
    
    /* Use __builtin_expect on final result */
    if (__builtin_expect(checksum > 1000, 0)) {
        checksum = checksum % 1000;
    }
    
    /* Prevent dead code elimination */
    optimization_barrier = checksum;
    
    /* Print result to ensure all code is live */
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional function to increase declaration complexity */
static int __attribute__((unused, noinline))
unused_builtin_wrapper(volatile int x) {
    /* Declare built-in prototype locally */
    int __builtin_popcount(unsigned int);
    
    /* Use the declared built-in */
    return __builtin_popcount((unsigned int)x) + 
           __builtin_abs(x) + 
           __builtin_expect(x > 0, 1);
}
