/* test_builtin_hooks.c - Comprehensive built-in function usage to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_wrapper(int x) __attribute__((used, noinline, noclone));

/* Global volatile variables to prevent optimization */
volatile int global_volatile_counter = 0;
volatile int global_volatile_result = 0;

/* Function with explicit hidden visibility and nothrow attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) * 2;
    }
    return 0;
}

/* Function with multiple built-ins and assembly barrier */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        /* Use math built-ins */
        int abs_val = __builtin_abs(seed + i - 10);
        result += abs_val;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Function focusing on bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* Use various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed | 1);
    
    /* Use built-in for endianness */
    result += __builtin_bswap32(seed) & 0xFF;
    
    return (int)result;
}

/* Function using overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow_result;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &overflow_result)) {
        result += 1000;
    } else {
        result += overflow_result;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &overflow_result)) {
        result += 2000;
    } else {
        result += overflow_result;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &overflow_result)) {
        result += 3000;
    } else {
        result += overflow_result;
    }
    
    return result;
}

/* Function using expectation and unreachable built-ins */
static int __attribute__((noinline, noclone))
test_builtin_control_flow(int x) {
    int result = x;
    
    /* Use __builtin_expect */
    if (__builtin_expect(x < 100, 0)) {
        result += 50;
    }
    
    /* Use __builtin_unreachable in impossible condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_trap for extreme cases */
    if (x > 10000) {
        __builtin_trap();
    }
    
    return result;
}

/* Static function with used attribute calling built-ins */
static int __attribute__((used))
static_builtin_wrapper(int x) {
    /* Mix of different built-ins */
    int a = __builtin_abs(x);
    int b = __builtin_clz(a | 1);
    int c = __builtin_popcount(a);
    
    /* Complex expression with built-in */
    return (__builtin_sadd_overflow(a, b, &c) ? c : a + b + c);
}

/* External function definition (simulating multi-file scope) */
int __attribute__((visibility("hidden")))
external_builtin_user(int x) {
    int result = 0;
    
    /* Use built-in with volatile memory access */
    volatile int temp = x;
    result += __builtin_abs(temp);
    
    /* Use built-in for synchronization */
    __builtin_ia32_mfence();
    
    /* Conditional unreachable */
    if (x == 0xDEADBEEF) {
        __builtin_unreachable();
    }
    
    return result;
}

/* Function using floating point built-ins */
static float __attribute__((noinline, noclone))
test_float_builtins(float seed) {
    float result = seed;
    
    /* Use floating point built-ins */
    result += __builtin_fabsf(seed);
    
    /* Note: sqrtf might be optimized, use with volatile */
    volatile float vseed = seed + 1.0f;
    result += __builtin_sqrtf(vseed);
    
    return result;
}

/* Main test orchestrator */
int main(int argc, char *argv[]) {
    int final_result = 0;
    
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)time(NULL);
    
    printf("Testing built-in function hooks...\n");
    
    /* Test 1: Arithmetic built-ins */
    int res1 = test_builtin_arithmetic(seed);
    final_result += res1;
    printf("Arithmetic test result: %d\n", res1);
    
    /* Test 2: Bit operation built-ins */
    int res2 = test_builtin_bitops(useed);
    final_result += res2;
    printf("Bitops test result: %d\n", res2);
    
    /* Test 3: Overflow built-ins */
    int res3 = test_builtin_overflow(seed, 100);
    final_result += res3;
    printf("Overflow test result: %d\n", res3);
    
    /* Test 4: Control flow built-ins */
    int res4 = test_builtin_control_flow(seed);
    final_result += res4;
    printf("Control flow test result: %d\n", res4);
    
    /* Test 5: Hidden visibility function with built-ins */
    int res5 = hidden_visibility_func(seed);
    final_result += res5;
    printf("Hidden visibility test result: %d\n", res5);
    
    /* Test 6: Static wrapper with used attribute */
    int res6 = static_builtin_wrapper(seed);
    final_result += res6;
    printf("Static wrapper test result: %d\n", res6);
    
    /* Test 7: External function simulation */
    int res7 = external_builtin_user(seed);
    final_result += res7;
    printf("External function test result: %d\n", res7);
    
    /* Test 8: Float built-ins */
    float res8 = test_float_builtins((float)seed);
    final_result += (int)res8;
    printf("Float builtins test result: %d\n", (int)res8);
    
    /* Additional direct built-in usage in main */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed);
    direct_result += __builtin_popcount(useed);
    
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(seed)) {
        direct_result += 100;
    }
    
    /* Use __builtin_expect in main */
    if (__builtin_expect(final_result > 0, 1)) {
        direct_result += final_result;
    }
    
    /* Store to volatile global to ensure side effects */
    global_volatile_result = direct_result;
    
    printf("Final checksum: %d\n", final_result + direct_result);
    
    /* Return non-zero result to ensure all code paths matter */
    return (final_result + direct_result) != 0 ? 0 : 1;
}

/* Additional function with assembly and built-ins */
void __attribute__((used))
additional_global_constructor(void) {
    /* This function should be kept due to 'used' attribute */
    volatile int x = 42;
    int y = __builtin_abs(x);
    
    /* Use built-in for memory clearing */
    char buffer[64];
    __builtin_memset(buffer, y & 0xFF, sizeof(buffer));
    
    /* Prevent optimization */
    asm volatile("" : : "r"(buffer) : "memory");
}
