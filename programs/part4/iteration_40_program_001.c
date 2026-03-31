/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_volatile_counter = 0;
static volatile int optimization_barrier = 0;

/* ============================================
   Pattern 1: Direct built-in usage with volatile
   ============================================ */
int test_builtin_arithmetic(volatile int seed) {
    int result = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (i = 0; i < 5; i++) {
        /* Use various arithmetic built-ins */
        int abs_val = __builtin_abs(seed + i - 3);
        result += abs_val;
        
        /* Use sqrt built-in (requires math library link, but declaration exists) */
        float fval = (float)(seed + i);
        /* __builtin_sqrtf is a valid GCC built-in */
        result += (int)__builtin_sqrtf(fval > 0 ? fval : 1.0f);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    global_volatile_counter += result;
    return result;
}

/* ============================================
   Pattern 2: Bit manipulation built-ins
   ============================================ */
int test_builtin_bitops(unsigned int seed) {
    unsigned int val = seed ^ 0xDEADBEEF;
    int result = 0;
    
    /* Multiple bit operation built-ins */
    result += __builtin_popcount(val);
    result += __builtin_clz(val | 1);  /* Ensure non-zero */
    result += __builtin_ctz(val | 1);  /* Ensure non-zero */
    result += __builtin_ffs(val | 1);  /* Ensure non-zero */
    
    /* Use parity built-in */
    result += __builtin_parity(val);
    
    /* Store in volatile to prevent elimination */
    optimization_barrier = result;
    return result;
}

/* ============================================
   Pattern 3: Overflow checking built-ins
   ============================================ */
int test_builtin_overflow(int a, int b) {
    int result = 0;
    int overflow;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result += 1;
    } else {
        result += overflow;
    }
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += 2;
    } else {
        result += overflow;
    }
    
    /* Test subtraction overflow */
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 4;
    } else {
        result += overflow;
    }
    
    return result;
}

/* ============================================
   Pattern 4: Function with explicit attributes
   ============================================ */
/* Static function with multiple attributes */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
attributed_builtin_user(int x) {
    /* Use expect built-in */
    if (__builtin_expect(x > 100, 0)) {
        return __builtin_abs(x) * 2;
    }
    
    /* Use prefetch built-in */
    __builtin_prefetch(&global_volatile_counter, 0, 3);
    
    return x;
}

int test_attributed_function(int seed) {
    /* Call the heavily attributed function */
    return attributed_builtin_user(seed);
}

/* ============================================
   Pattern 5: External linkage simulation
   ============================================ */
/* Forward declaration with extern */
extern int external_builtin_helper(int x);

/* Function that will be called externally */
int external_builtin_user(int x) {
    int result = 0;
    
    /* Use built-in in external function */
    result += __builtin_abs(x);
    
    /* Conditional unreachable */
    if (x < 0) {
        __builtin_unreachable();  /* Hint to optimizer */
    }
    
    return result + external_builtin_helper(x);
}

/* Another external-like function */
int external_builtin_helper(int x) {
    /* Use byte swap built-ins */
    unsigned int val = (unsigned int)x;
    return __builtin_bswap32(val) & 0xFF;
}

/* ============================================
   Pattern 6: Visibility-specific function
   ============================================ */
/* Function with hidden visibility attribute */
void __attribute__((visibility("hidden"), used))
hidden_visibility_func(void) {
    /* Use synchronization built-in */
    int val = __atomic_load_n(&global_volatile_counter, __ATOMIC_RELAXED);
    
    /* Use built-in for type traits */
    if (__builtin_types_compatible_p(int, unsigned int)) {
        val += 1;
    } else {
        val += __builtin_choose_expr(sizeof(int) == 4, 2, 3);
    }
    
    __atomic_store_n(&global_volatile_counter, val, __ATOMIC_RELAXED);
}

/* ============================================
   Pattern 7: Complex expression with built-ins
   ============================================ */
int __attribute__((noinline, noclone))
complex_builtin_expression(int a, int b, int c) {
    volatile int barrier = 0;
    int result = 0;
    
    /* Nested built-in calls in complex expressions */
    result = __builtin_abs(a) + 
             (__builtin_popcount(b) * 2) -
             (__builtin_clz(c | 1) / 4);
    
    /* Built-in in conditional */
    if (__builtin_expect(result > 100, 0)) {
        result = __builtin_abs(result - 50);
    }
    
    /* Overflow check in loop */
    for (int i = 0; i < 3; i++) {
        int tmp;
        if (__builtin_add_overflow(result, i, &tmp)) {
            result = 0;
            break;
        }
        result = tmp;
    }
    
    barrier = result;  /* Volatile write */
    return result;
}

/* ============================================
   Main function - orchestrates all tests
   ============================================ */
int main(int argc, char *argv[]) {
    int final_result = 0;
    volatile int seed;
    
    /* Initialize seed from various sources */
    seed = argc;
    seed ^= (int)time(NULL);
    seed ^= (int)clock();
    
    printf("Starting built-in function coverage test...\n");
    printf("Seed: %d\n", seed);
    
    /* Test 1: Arithmetic built-ins */
    final_result += test_builtin_arithmetic(seed);
    printf("Test 1 complete: %d\n", final_result);
    
    /* Test 2: Bit operation built-ins */
    final_result += test_builtin_bitops((unsigned int)seed);
    printf("Test 2 complete: %d\n", final_result);
    
    /* Test 3: Overflow built-ins */
    final_result += test_builtin_overflow(seed, seed + 1);
    printf("Test 3 complete: %d\n", final_result);
    
    /* Test 4: Attributed function */
    final_result += test_attributed_function(seed);
    printf("Test 4 complete: %d\n", final_result);
    
    /* Test 5: External linkage functions */
    final_result += external_builtin_user(seed);
    printf("Test 5 complete: %d\n", final_result);
    
    /* Test 6: Visibility function */
    hidden_visibility_func();
    final_result += global_volatile_counter;
    printf("Test 6 complete: %d\n", final_result);
    
    /* Test 7: Complex expression */
    final_result += complex_builtin_expression(seed, seed ^ 0x1234, seed ^ 0x5678);
    printf("Test 7 complete: %d\n", final_result);
    
    /* Additional built-in usage in main */
    /* Use built-in for constant detection */
    if (__builtin_constant_p(argc)) {
        final_result += 1000;
    }
    
    /* Use built-in for object size */
    final_result += __builtin_object_size(argv, 0) > 0 ? 1 : 0;
    
    printf("Final result: %d\n", final_result);
    
    /* Return checksum to ensure all code is live */
    return final_result & 0xFF;
}
