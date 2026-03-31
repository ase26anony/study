/* test_builtin_hooks.c - Comprehensive test for GCC built-in function hooks */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void unreachable_helper(int x) __attribute__((nothrow));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int result_accumulator = 0;

/* Helper function with multiple attributes that uses built-ins */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
hidden_attributed_function(int x, int y) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        /* Use __builtin_abs to ensure the built-in is processed */
        return __builtin_abs(y) * 2;
    }
    return __builtin_abs(x) * 3;
}

/* Function with static linkage that uses built-ins */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int sum = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent optimization */
    for (i = 0; i < 5; i++) {
        /* Use different arithmetic built-ins */
        int val = seed + i;
        sum += __builtin_abs(val);
        
        /* Use __builtin_sqrtf with type conversion */
        if (val > 0) {
            float fval = (float)val;
            /* Memory barrier to prevent optimization */
            asm volatile("" : : : "memory");
            sum += (int)__builtin_sqrtf(fval);
        }
    }
    
    /* Store result in volatile to ensure side effect */
    global_counter += sum;
    return sum;
}

/* Function using bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Use various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Use __builtin_parity */
    result += __builtin_parity(seed);
    
    /* Complex expression with built-in */
    result = __builtin_bswap32(result) ^ seed;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return (int)result;
}

/* Function using overflow checking built-ins */
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

/* Function that declares and uses built-in prototypes */
static int __attribute__((noinline, noclone))
test_builtin_declarations(volatile int x) {
    /* Explicit declaration of built-in function */
    int __builtin_popcount(unsigned int);
    int __builtin_clz(unsigned int);
    int __builtin_ctz(unsigned int);
    int __builtin_ffs(int);
    
    /* Use the declared built-ins */
    int r1 = __builtin_popcount((unsigned int)x);
    int r2 = __builtin_clz((unsigned int)x | 1);
    int r3 = __builtin_ctz((unsigned int)x | 1);
    int r4 = __builtin_ffs(x);
    
    /* Use __builtin_expect with the result */
    if (__builtin_expect((r1 + r2 + r3 + r4) > 10, 0)) {
        return 1;
    }
    return 0;
}

/* External function definition (simulating multi-file) */
int __attribute__((visibility("default"), nothrow))
external_builtin_user(int x) {
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return x * 2;
    }
    
    /* Use __builtin_classify_type */
    int type = __builtin_classify_type(x);
    
    /* Use __builtin_trap in a conditional */
    if (x < 0) {
        __builtin_trap();
    }
    
    return x + type;
}

/* Function that might use __builtin_unreachable */
void __attribute__((nothrow, noinline, noclone))
unreachable_helper(int x) {
    if (x < 0) {
        /* This should never happen in our test */
        __builtin_unreachable();
    }
    
    /* Use __builtin_prefetch */
    int array[100];
    __builtin_prefetch(&array[x % 100], 0, 0);
}

/* Main test driver */
int main(int argc, char **argv) {
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)argc;
    int checksum = 0;
    
    /* Initialize volatile accumulator */
    result_accumulator = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(useed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, 1000);
    
    /* Test 4: Built-in declarations */
    checksum += test_builtin_declarations(seed);
    
    /* Test 5: Hidden attributed function */
    checksum += hidden_attributed_function(seed, -seed);
    
    /* Test 6: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Unreachable helper */
    unreachable_helper(seed);
    
    /* Test 8: Direct built-in usage with volatile storage */
    volatile int direct_result;
    direct_result = __builtin_abs(seed);
    direct_result += __builtin_popcount((unsigned int)seed);
    direct_result += __builtin_clz((unsigned int)seed | 1);
    
    /* Use __builtin_expect in main */
    if (__builtin_expect(checksum > 10000, 0)) {
        direct_result *= 2;
    }
    
    /* Final result computation preventing dead code elimination */
    checksum += direct_result;
    checksum += global_counter;
    checksum += result_accumulator;
    
    /* Use __builtin_printf if available */
#ifdef __BUILTIN_PRINTF__
    __builtin_printf("Result: %d\n", checksum);
#else
    printf("Result: %d\n", checksum);
#endif
    
    /* Return checksum to prevent optimization */
    return checksum == 0 ? 1 : 0;
}

/* Additional function with assembly and built-ins */
static void __attribute__((used, noinline, noclone))
additional_builtin_test(void) {
    volatile int x = 42;
    volatile int y = 100;
    
    /* Mixed built-ins and assembly */
    asm volatile("nop" : : : "memory");
    int r1 = __builtin_abs(x);
    asm volatile("nop" : : : "memory");
    int r2 = __builtin_popcount((unsigned int)y);
    asm volatile("nop" : : : "memory");
    
    /* Use __builtin_constant_p with assembly barrier */
    if (!__builtin_constant_p(x)) {
        asm volatile("" : : "r"(r1), "r"(r2) : "memory");
    }
}

/* Force emission of additional test */
static void __attribute__((constructor))
init_additional_test(void) {
    additional_builtin_test();
}
