/* test_targhooks.c - Comprehensive built-in function test to trigger flag-setting hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declaration to simulate multi-file scope */
extern int external_builtin_user(int x);

/* Volatile variables to prevent optimization */
volatile int global_volatile_counter = 0;
volatile int global_volatile_result = 0;

/* ============================================
   Pattern 1: Direct built-in usage with volatile
   ============================================ */
int test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (volatile int i = 0; i < 5; i++) {
        /* Use various arithmetic built-ins */
        int abs_val = __builtin_abs(seed + i - 10);
        result += abs_val;
        
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect(abs_val > 5, 0)) {
            result += 100;
        }
        
        /* Use __builtin_choose_expr */
        result += __builtin_choose_expr(abs_val > 0, 10, -10);
    }
    
    /* Store result in volatile to ensure side effects */
    global_volatile_counter += result;
    return result;
}

/* ============================================
   Pattern 2: Bit manipulation built-ins
   ============================================ */
int test_builtin_bitops(unsigned int seed) {
    unsigned int result = 0;
    
    /* Use bit counting built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed | 1);
    
    /* Use parity built-in */
    result += __builtin_parity(seed);
    
    /* Use byte swap built-in */
    result += __builtin_bswap32(seed) & 0xFF;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    return (int)result;
}

/* ============================================
   Pattern 3: Overflow checking built-ins
   ============================================ */
int test_builtin_overflow(int seed) {
    int result = 0;
    int overflow_result;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(seed, 1000000000, &overflow_result)) {
        result += 1;
    } else {
        result += overflow_result & 0xFF;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(seed, 3, &overflow_result)) {
        result += 2;
    } else {
        result += overflow_result & 0xFF;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(seed, -1000000000, &overflow_result)) {
        result += 4;
    } else {
        result += overflow_result & 0xFF;
    }
    
    return result;
}

/* ============================================
   Pattern 4: Function with explicit attributes
   ============================================ */
/* Static function with multiple attributes that may interact with the hook */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
attributed_builtin_user(int x) {
    /* Use __builtin_expect inside attributed function */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) + __builtin_clz(x | 1);
    }
    return 0;
}

int test_attributed_function(int seed) {
    /* Call the attributed function */
    int result = attributed_builtin_user(seed);
    
    /* Use __builtin_assume_aligned */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    int* aligned_ptr = (int*)__builtin_assume_aligned(array, 16);
    result += aligned_ptr[0];
    
    return result;
}

/* ============================================
   Pattern 5: External linkage simulation
   ============================================ */
/* Forward declaration with attributes */
extern int __attribute__((visibility("hidden"))) 
external_helper(int x) __attribute__((nothrow));

/* Definition of external helper */
int __attribute__((visibility("hidden")))
external_helper(int x) {
    /* Use __builtin_unreachable for impossible paths */
    if (x < 0) {
        __builtin_unreachable();  /* We ensure x is non-negative */
    }
    
    /* Use __builtin_trap for error conditions */
    if (x > 1000000) {
        __builtin_trap();
    }
    
    return __builtin_popcount(x);
}

/* External built-in user function definition */
int external_builtin_user(int x) {
    int result = 0;
    
    /* Use frame address built-in */
    void* frame_addr = __builtin_frame_address(0);
    result += (long)frame_addr & 0xFF;
    
    /* Call external helper */
    result += external_helper(x);
    
    /* Use __builtin_return_address */
    void* ret_addr = __builtin_return_address(0);
    result += (long)ret_addr & 0xFF;
    
    return result;
}

int test_external_linkage(int seed) {
    return external_builtin_user(seed);
}

/* ============================================
   Pattern 6: Complex expression integration
   ============================================ */
int __attribute__((noinline, noclone))
complex_builtin_expression(volatile int seed) {
    int result = seed;
    
    /* Complex expression mixing multiple built-ins */
    result = __builtin_abs(
        __builtin_add_overflow_p(result, 42, 0) ? 
        __builtin_popcount(result) : 
        __builtin_clz(result | 1)
    );
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        result += __builtin_ffs(result | 1) * 10;
    }
    
    /* Inline assembly barrier */
    __asm__ volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* ============================================
   Pattern 7: Built-ins with floating point
   ============================================ */
float __attribute__((noinline))
test_float_builtins(float seed) {
    float result = seed;
    
    /* Use floating point built-ins */
    result = __builtin_fabsf(result);
    result = __builtin_sqrtf(result + 1.0f);
    
    /* Use classification built-ins */
    if (__builtin_isnan(result)) {
        result = 1.0f;
    }
    
    if (__builtin_isinf(result)) {
        result = 2.0f;
    }
    
    /* Use sign built-in */
    if (__builtin_signbitf(result)) {
        result = -result;
    }
    
    return result;
}

/* ============================================
   Main function - orchestrates all tests
   ============================================ */
int main(int argc, char** argv) {
    /* Initialize seed from runtime sources */
    volatile int seed = argc;
    seed += time(NULL) & 0xFF;
    seed += (int)(long)argv & 0xFF;
    
    int checksum = 0;
    
    /* Run all test patterns */
    checksum += test_builtin_arithmetic(seed);
    checksum += test_builtin_bitops((unsigned int)seed);
    checksum += test_builtin_overflow(seed);
    checksum += test_attributed_function(seed);
    checksum += test_external_linkage(seed);
    checksum += complex_builtin_expression(seed);
    
    /* Test float built-ins */
    float float_result = test_float_builtins((float)seed);
    checksum += (int)float_result;
    
    /* Use synchronization built-ins */
    __sync_fetch_and_add(&global_volatile_result, checksum);
    
    /* Use atomic built-ins */
    int atomic_val = __atomic_load_n(&global_volatile_result, __ATOMIC_ACQUIRE);
    
    /* Final result depends on all computations */
    printf("Result: %d (atomic: %d)\n", checksum, atomic_val);
    
    /* Use __builtin_return_address at main exit */
    void* main_ret_addr = __builtin_return_address(0);
    checksum += (long)main_ret_addr & 0xFF;
    
    return checksum & 0xFF;
}

/* Additional global declarations with attributes */
int __attribute__((visibility("hidden"), used))
global_builtin_user = 0;

static void __attribute__((constructor))
init_globals(void) {
    /* Use __builtin_cpu_init */
    __builtin_cpu_init();
    
    /* Initialize with built-in */
    global_builtin_user = __builtin_popcount(0xABCDEF);
}
