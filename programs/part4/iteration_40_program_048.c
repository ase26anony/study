/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_wrapper(int x) __attribute__((used, nothrow));

/* Volatile variables to prevent optimization */
volatile int global_volatile_counter = 0;
volatile int optimization_barrier = 0;

/* Function with explicit hidden visibility and multiple attributes */
__attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
int hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return __builtin_clz((unsigned int)x);
}

/* Static function with used attribute to force emission */
static int static_builtin_wrapper(int x) __attribute__((used, nothrow));
static int static_builtin_wrapper(int x) {
    int result = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (i = 0; i < 3; i++) {
        /* Mix different built-ins */
        result += __builtin_popcount((unsigned int)(x + i));
        result -= __builtin_ctz((unsigned int)(x + i + 1));
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Function with external linkage defined later */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable for unreachable code */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Overflow checking built-in */
    int overflow_result;
    if (__builtin_add_overflow(x, 1000, &overflow_result)) {
        return __builtin_abs(x);
    }
    
    return overflow_result;
}

/* Another function with complex built-in usage */
__attribute__((noinline, noclone))
int test_builtin_arithmetic(volatile int seed) {
    int acc = 0;
    volatile int i;
    
    for (i = 0; i < 4; i++) {
        /* Use math built-ins */
        int val = seed + i;
        acc += __builtin_abs(val);
        
        /* Use sqrt built-in with float */
        float fval = (float)val;
        acc += (int)__builtin_sqrtf(fval * fval);
    }
    
    /* Store to volatile to ensure side effects */
    optimization_barrier = acc;
    return acc;
}

__attribute__((noinline, noclone))
int test_builtin_bitops(int seed) {
    unsigned int uval = (unsigned int)seed;
    int result = 0;
    
    /* Chain multiple bit operation built-ins */
    result = __builtin_popcount(uval);
    result += __builtin_clz(uval | 1);  /* Ensure non-zero */
    result += __builtin_ctz(uval | 1);  /* Ensure non-zero */
    result += __builtin_ffs(seed);
    
    /* Use built-in in conditional */
    if (__builtin_parity(uval)) {
        result += 1000;
    }
    
    return result;
}

__attribute__((noinline, noclone))
int test_builtin_overflow(int seed) {
    int result = 0;
    int overflow_flag;
    
    /* Test various overflow built-ins */
    if (__builtin_add_overflow(seed, 0x7FFFFFFF, &result)) {
        result = __builtin_abs(seed);
    }
    
    if (__builtin_mul_overflow(seed, 2, &result)) {
        result = __builtin_clz((unsigned int)seed);
    }
    
    if (__builtin_sub_overflow(0, seed, &result)) {
        result = __builtin_popcount((unsigned int)seed);
    }
    
    /* Use __builtin_expect with overflow check */
    overflow_flag = __builtin_expect(__builtin_add_overflow(seed, 100, &result), 0);
    if (overflow_flag) {
        result = 0;
    }
    
    return result;
}

/* Function that declares built-in prototype locally */
__attribute__((noinline))
int test_local_builtin_declaration(int x) {
    /* Declare built-in function prototype */
    int __builtin_popcount(unsigned int);
    
    volatile unsigned int v = (unsigned int)x;
    int count = __builtin_popcount(v);
    
    /* Declare another built-in */
    int __builtin_abs(int);
    count += __builtin_abs(x);
    
    return count;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int final_result = 0;
    volatile int seed;
    
    /* Initialize seed from various sources */
    seed = argc;
    seed += (int)time(NULL) & 0xFF;
    seed |= 1;  /* Ensure non-zero */
    
    printf("Starting built-in function coverage test...\n");
    printf("Seed value: %d\n", seed);
    
    /* Test 1: Arithmetic built-ins */
    int arith_result = test_builtin_arithmetic(seed);
    final_result += arith_result;
    printf("Arithmetic test result: %d\n", arith_result);
    
    /* Test 2: Bit operation built-ins */
    int bitop_result = test_builtin_bitops(seed);
    final_result += bitop_result;
    printf("Bit operation test result: %d\n", bitop_result);
    
    /* Test 3: Overflow built-ins */
    int overflow_result = test_builtin_overflow(seed);
    final_result += overflow_result;
    printf("Overflow test result: %d\n", overflow_result);
    
    /* Test 4: Hidden visibility function */
    int hidden_result = hidden_visibility_func(seed);
    final_result += hidden_result;
    printf("Hidden visibility test result: %d\n", hidden_result);
    
    /* Test 5: Static wrapper function */
    int static_result = static_builtin_wrapper(seed);
    final_result += static_result;
    printf("Static wrapper test result: %d\n", static_result);
    
    /* Test 6: External function */
    int external_result = external_builtin_user(seed);
    final_result += external_result;
    printf("External function test result: %d\n", external_result);
    
    /* Test 7: Local built-in declaration */
    int local_decl_result = test_local_builtin_declaration(seed);
    final_result += local_decl_result;
    printf("Local declaration test result: %d\n", local_decl_result);
    
    /* Additional built-in usage in main */
    int temp;
    if (__builtin_expect(final_result > 0, 1)) {
        temp = __builtin_abs(final_result);
    } else {
        temp = __builtin_clz((unsigned int)final_result);
    }
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        temp += __builtin_popcount((unsigned int)temp);
    }
    
    final_result += temp;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(final_result) : "memory");
    
    printf("Final checksum: %d\n", final_result);
    
    /* Use __builtin_return_address */
    void *ra = __builtin_return_address(0);
    printf("Return address: %p\n", ra);
    
    return final_result & 0xFF;  /* Return non-zero result */
}

/* Additional global declaration with attributes */
int __attribute__((visibility("hidden"), used)) 
global_builtin_helper(int x) {
    return __builtin_ffs(x) + __builtin_parity((unsigned int)x);
}
