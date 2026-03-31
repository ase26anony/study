/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;
static volatile int volatile_result = 0;

/* Function with multiple attributes that may interact with built-in processing */
static int __attribute__((used, noinline, noclone))
test_attributed_function(int x) __attribute__((nothrow, visibility("hidden")));

/* Built-in function prototype declaration (simulating user declaration) */
int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
int __builtin_clz(unsigned int) __attribute__((used));
int __builtin_ctz(unsigned int);

/* Helper function with optimization barrier */
static void __attribute__((noinline, noclone))
optimization_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Test 1: Arithmetic built-ins with volatile context */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int i;
    
    for (i = 0; i < 5; i++) {
        /* Use various math built-ins */
        int val = seed + i;
        result += __builtin_abs(val);
        
        /* Use floating-point built-in */
        float fval = (float)val;
        result += (int)__builtin_sqrtf(fval);
        
        /* Use expect built-in to influence branching */
        if (__builtin_expect(val > 100, 0)) {
            result += 100;
        }
        
        optimization_barrier();
    }
    
    volatile_result = result;
    return result;
}

/* Test 2: Bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int x = seed ^ 0xDEADBEEF;
    int result = 0;
    
    /* Direct built-in calls with volatile intermediate */
    volatile unsigned int vx = x;
    result += __builtin_popcount(vx);
    result += __builtin_clz(vx | 1);  /* Ensure non-zero */
    result += __builtin_ctz(vx | 1);  /* Ensure non-zero */
    
    /* Built-in in conditional expression */
    result += (__builtin_parity(x) ? 1 : 0);
    
    /* Built-in with side effects */
    volatile_counter += __builtin_ffs(x);
    
    optimization_barrier();
    return result;
}

/* Test 3: Overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Add overflow check */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1000;
    } else {
        result += overflow;
    }
    
    /* Mul overflow check */
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 2000;
    } else {
        result += overflow;
    }
    
    /* Sub overflow check */
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 3000;
    } else {
        result += overflow;
    }
    
    optimization_barrier();
    return result;
}

/* Test 4: Attributed function definition */
static int __attribute__((used, noinline, noclone))
test_attributed_function(int x) {
    /* This function has hidden visibility and nothrow attributes */
    int result = 0;
    
    /* Use built-in inside attributed function */
    result = __builtin_expect(x > 0, 1) ? x : -x;
    
    /* Use unreachable built-in under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    optimization_barrier();
    return result;
}

/* Test 5: External linkage simulation */
extern int __attribute__((visibility("default")))
external_builtin_user(int x) {
    /* This function is declared extern earlier but defined here */
    int result = x;
    
    /* Use multiple built-ins */
    result += __builtin_bswap16(x & 0xFFFF);
    result += __builtin_bswap32(x);
    
    /* Use prefetch built-in */
    __builtin_prefetch(&result, 0, 3);
    
    /* Complex expression with built-in */
    result = __builtin_constant_p(x) ? 42 : result;
    
    optimization_barrier();
    return result;
}

/* Another externally declared function */
void __attribute__((visibility("hidden"), nothrow))
hidden_visibility_func(void) {
    /* Function with explicit hidden visibility */
    volatile int x = volatile_counter;
    
    /* Use built-in with volatile */
    int bits = __builtin_popcount(x);
    
    /* Use sync built-in */
    __sync_fetch_and_add(&volatile_counter, bits);
    
    optimization_barrier();
}

/* Test 6: Frame address and return address built-ins */
static int __attribute__((noinline, noclone))
test_frame_address(void) {
    void* frame_addr = __builtin_frame_address(0);
    void* return_addr = __builtin_return_address(0);
    
    /* Use addresses in computation (non-portable but valid) */
    int result = ((long)frame_addr ^ (long)return_addr) & 0xFF;
    
    optimization_barrier();
    return result;
}

/* Test 7: Type-generic built-ins */
static int __attribute__((noinline, noclone))
test_type_generic(volatile int seed) {
    int result = 0;
    
    /* Use type-generic math built-ins */
    double d = (double)seed;
    result += (int)__builtin_ceil(d);
    result += (int)__builtin_floor(d);
    result += (int)__builtin_trunc(d);
    
    /* Use fabs built-in */
    result += (int)__builtin_fabs(d);
    
    optimization_barrier();
    return result;
}

/* Main function that ties everything together */
int main(int argc, char* argv[]) {
    int final_result = 0;
    
    /* Initialize seed from runtime sources */
    volatile int seed = argc;
    seed ^= (int)time(NULL);
    seed ^= (int)clock();
    
    /* Test 1: Arithmetic built-ins */
    final_result += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    final_result += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    final_result += test_builtin_overflow(seed, 100);
    
    /* Test 4: Attributed function */
    final_result += test_attributed_function(seed);
    
    /* Test 5: External linkage functions */
    final_result += external_builtin_user(seed);
    hidden_visibility_func();
    final_result += volatile_counter;
    
    /* Test 6: Frame address */
    final_result += test_frame_address();
    
    /* Test 7: Type-generic */
    final_result += test_type_generic(seed);
    
    /* Use printf to ensure all results are used */
    printf("Final checksum: %d\n", final_result);
    
    /* Additional built-in at end */
    if (__builtin_expect(final_result > 1000000, 0)) {
        __builtin_trap();
    }
    
    return final_result & 0xFF;
}

/* Additional global declaration with attributes */
int __attribute__((used, visibility("hidden")))
global_builtin_user = __builtin_abs(-42);

/* Force emission of all functions */
void __attribute__((used))
force_emission(void) {
    test_builtin_arithmetic(0);
    test_builtin_bitops(0);
    test_builtin_overflow(0, 0);
    test_attributed_function(0);
    external_builtin_user(0);
    hidden_visibility_func();
    test_frame_address();
    test_type_generic(0);
}
