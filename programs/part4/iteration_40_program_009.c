/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int optimization_barrier = 0;

/* Barrier to prevent optimization */
#define OPT_BARRIER() asm volatile("" : : : "memory")

/* Helper function with multiple attributes */
static int __attribute__((noinline, noclone, used))
test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    
    /* Use volatile loop counter */
    volatile int i;
    for (i = 0; i < 3; i++) {
        /* Math built-ins with volatile arguments */
        int val = seed + i + optimization_barrier;
        result += __builtin_abs(val);
        
        /* Floating point built-in */
        float fval = (float)val;
        result += (int)__builtin_sqrtf(fval);
        
        /* Built-in with side effects */
        OPT_BARRIER();
    }
    
    /* Store result in volatile to ensure it's used */
    global_counter += result;
    return result;
}

/* Bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* Various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Built-in with optimization hint */
    if (__builtin_expect((seed & 1) != 0, 0)) {
        result += __builtin_parity(seed);
    }
    
    OPT_BARRIER();
    return (int)result;
}

/* Overflow checking built-ins */
static int __attribute__((noinline, noclone, nothrow))
test_builtin_overflow(int seed) {
    int result = 0;
    int overflow;
    
    /* Addition overflow check */
    if (__builtin_add_overflow(seed, 1000, &result)) {
        result = seed - 1000;
    }
    
    /* Multiplication overflow check */
    int mul_result;
    if (__builtin_mul_overflow(seed, 2, &mul_result)) {
        result += seed;
    } else {
        result += mul_result;
    }
    
    /* Subtraction overflow check */
    int sub_result;
    __builtin_sub_overflow(seed, INT_MAX/2, &sub_result);
    result += sub_result;
    
    OPT_BARRIER();
    return result;
}

/* Function with explicit hidden visibility and other attributes */
static void __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(int seed) {
    /* Use built-in for branch prediction */
    if (__builtin_expect(seed > 100, 0)) {
        /* Use unreachable built-in under specific condition */
        if (seed > 1000) {
            __builtin_unreachable();
        }
    }
    
    /* Built-in for object size checking */
    char buffer[100];
    __builtin___memset_chk(buffer, seed % 256, sizeof(buffer),
                          __builtin_object_size(buffer, 0));
    
    OPT_BARRIER();
    global_counter += seed;
}

/* Static function calling built-ins */
static int __attribute__((noinline))
static_function_with_builtins(int x) {
    /* Declaration of built-in function prototype */
    int __builtin_popcount(unsigned int);
    
    /* Use the declared built-in */
    int result = __builtin_popcount((unsigned int)x);
    
    /* Additional built-in usage */
    result += __builtin_abs(x);
    
    /* Use built-in for synchronization */
    __builtin_ia32_mfence();
    
    return result;
}

/* External function definition (simulating another translation unit) */
int external_builtin_user(int x) {
    /* Use various built-ins */
    int result = 0;
    
    /* Built-in for constant propagation */
    result += __builtin_constant_p(x) ? 10 : 20;
    
    /* Built-in for prefetch */
    __builtin_prefetch(&global_counter, 0, 3);
    
    /* Use unreachable built-in */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    OPT_BARRIER();
    return result + x;
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden")))
hidden_visibility_func(void) {
    /* Use built-in for trap generation */
    if (global_counter > 1000000) {
        __builtin_trap();
    }
    
    /* Use sync built-in */
    __builtin_sync_fetch_and_add(&global_counter, 1);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize with volatile seed */
    volatile int seed = argc;
    seed += time(NULL) % 100;
    seed += optimization_barrier;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed);
    
    /* Test 4: Attributed function with built-ins */
    test_attributed_function(seed);
    checksum += global_counter;
    
    /* Test 5: Static function with built-in declaration */
    checksum += static_function_with_builtins(seed);
    
    /* Test 6: External function simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    checksum += global_counter;
    
    /* Additional direct built-in usage in main */
    
    /* Built-in for variable argument handling */
    checksum += __builtin_va_arg_pack_len();
    
    /* Built-in for offsetof */
    struct test_struct { int a; char b; long c; };
    checksum += __builtin_offsetof(struct test_struct, c);
    
    /* Built-in for types */
    checksum += __builtin_types_compatible_p(int, unsigned int) ? 0 : 1;
    
    /* Use result to prevent dead code elimination */
    OPT_BARRIER();
    
    /* Final output/return */
    printf("Checksum: %d\n", checksum);
    return checksum % 256;
}

/* Additional function with mixed attributes */
int __attribute__((used, noinline, visibility("hidden")))
unused_function_with_builtins(void) {
    /* This function is marked used but may not be called */
    volatile int x = 42;
    
    /* Multiple built-in calls */
    int r = __builtin_bswap32(x);
    r += __builtin_ceilf((float)x);
    r += __builtin_clrsb(x);
    
    /* Declaration style that might trigger the hook */
    void __builtin___clear_cache(void *begin, void *end);
    
    char buffer[64];
    __builtin___clear_cache(buffer, buffer + 64);
    
    return r;
}
