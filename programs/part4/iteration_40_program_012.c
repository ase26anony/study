/* test_builtin_hooks.c - Comprehensive test for GCC built-in function hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* Prevent optimization of helper functions */
#define NO_OPT __attribute__((noinline, noclone))

/* External function declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Function with multiple attributes that may interact with built-in processing */
static int hidden_function(int x) __attribute__((visibility("hidden"), nothrow, used));
static int hidden_function(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Another static function with different attributes */
static int used_function(void) __attribute__((used, noinline));
static int used_function(void) {
    volatile int counter = 0;
    int result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (counter = 0; counter < 10; counter++) {
        /* Use multiple built-ins in sequence */
        result += __builtin_popcount(counter);
        result += __builtin_clz(counter | 1);  /* Ensure non-zero */
    }
    
    return result;
}

/* Function with overflow checking built-ins */
NO_OPT int test_builtin_overflow(volatile int seed) {
    int a = seed;
    int b = seed * 2;
    int sum, product;
    bool overflow;
    
    /* Test addition overflow */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        /* Use __builtin_unreachable in a controlled manner */
        if (sum < INT_MIN) {
            __builtin_unreachable();
        }
    }
    
    /* Test multiplication overflow */
    overflow = __builtin_mul_overflow(a, b, &product);
    
    /* Use asm volatile as optimization barrier */
    asm volatile("" : : "r"(sum), "r"(product) : "memory");
    
    return sum + product + (overflow ? 1 : 0);
}

/* Function with arithmetic built-ins */
NO_OPT int test_builtin_arithmetic(volatile float seed) {
    volatile float f = seed;
    int result = 0;
    int i;
    
    for (i = 0; i < 5; i++) {
        /* Use math built-ins with volatile operands */
        int abs_val = __builtin_abs((int)f + i);
        float sqrt_val = __builtin_sqrtf(f + i);
        
        /* Store results in volatile to prevent elimination */
        volatile int store_abs = abs_val;
        volatile float store_sqrt = sqrt_val;
        
        result += store_abs + (int)store_sqrt;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Function with bit operation built-ins */
NO_OPT int test_builtin_bitops(volatile unsigned int seed) {
    unsigned int x = seed;
    int result = 0;
    
    /* Chain multiple built-in calls */
    result += __builtin_popcount(x);
    result += __builtin_ctz(x | 1);  /* Avoid undefined behavior for 0 */
    result += __builtin_clz(x | 1);
    result += __builtin_ffs(x | 1);
    
    /* Use built-in with side effects through volatile */
    volatile unsigned int y = x;
    result += __builtin_parity(y);
    
    return result;
}

/* External function definition (simulating separate compilation unit) */
int external_builtin_user(int x) {
    /* This function is extern in declaration but defined here */
    if (x < 0) {
        /* Use __builtin_unreachable for negative values */
        __builtin_unreachable();
    }
    
    /* Use built-in with optimization barrier */
    int result = __builtin_bswap32(x);
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Function that combines multiple built-in patterns */
NO_OPT int test_combined_builtins(volatile int seed) {
    int result = 0;
    
    /* Use expectation built-in */
    if (__builtin_expect(seed != 0, 0)) {
        result += __builtin_abs(seed);
    }
    
    /* Use object size built-in */
    char buffer[64];
    result += __builtin_object_size(buffer, 0);
    
    /* Use constant propagation built-in */
    result += __builtin_constant_p(seed) ? 0 : 1;
    
    /* Use prefetch built-in (has no return value, but affects codegen) */
    __builtin_prefetch(&result, 0, 3);
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    volatile float fseed = argc * 1.5f;
    volatile unsigned int useed = (unsigned int)argc;
    
    int checksum = 0;
    
    /* Call all test functions to ensure they're not eliminated */
    checksum += hidden_function(seed);
    checksum += used_function();
    checksum += test_builtin_overflow(seed);
    checksum += test_builtin_arithmetic(fseed);
    checksum += test_builtin_bitops(useed);
    checksum += external_builtin_user(seed);
    checksum += test_combined_builtins(seed);
    
    /* Additional direct built-in usage in main */
    checksum += __builtin_abs(checksum);
    
    /* Use printf to ensure all code has observable effect */
    printf("Built-in test checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero result */
}

/* Additional global declarations with attributes */
int __attribute__((visibility("hidden"))) global_hidden = 0;
static int __attribute__((used)) global_used = 0;

/* Function pointer that might trigger built-in processing */
static int (* __attribute__((used)) builtin_func_ptr)(int) = __builtin_abs;

/* One more function using assembly with built-in */
void __attribute__((nothrow)) final_barrier(void) {
    /* Mixed assembly and built-in */
    asm volatile("" : : : "memory");
    volatile int x = __builtin_bswap32(0x12345678);
    (void)x;
}
