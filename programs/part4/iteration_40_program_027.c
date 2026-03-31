/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_test(void);

/* Volatile variables to prevent optimization */
static volatile int volatile_accumulator = 0;
static volatile int volatile_seed = 0;

/* Function with multiple attributes that may interact with built-in processing */
static int __attribute__((visibility("hidden"), nothrow, used, noinline, noclone))
attributed_builtin_wrapper(int x, int y) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        /* Use __builtin_abs to ensure built-in processing */
        return __builtin_abs(y) * 2;
    }
    return __builtin_abs(x) * 3;
}

/* Another attributed function calling different built-ins */
int __attribute__((visibility("hidden"), used, noinline))
hidden_visibility_func(unsigned int val) {
    /* Use population count built-in */
    int popcnt = __builtin_popcount(val);
    
    /* Use count leading zeros built-in */
    int clz = __builtin_clz(val | 1);  /* OR with 1 to avoid undefined behavior for 0 */
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return popcnt + clz;
}

/* Function using overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_overflow_operations(int a, int b) {
    int result = 0;
    int overflow_flag = 0;
    
    /* Test addition overflow */
    if (__builtin_add_overflow(a, b, &result)) {
        overflow_flag |= 1;
    }
    
    /* Test multiplication overflow */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        overflow_flag |= 2;
        result = b;  /* Use different value on overflow */
    } else {
        result += mul_result;
    }
    
    /* Use __builtin_expect with the overflow flag */
    if (__builtin_expect(overflow_flag != 0, 0)) {
        return __builtin_abs(result);
    }
    
    return result;
}

/* Function with complex built-in usage in loops */
static int __attribute__((noinline))
test_builtin_in_loop(int iterations) {
    volatile int sum = 0;
    volatile int counter = iterations;
    
    while (counter-- > 0) {
        /* Use different built-ins based on loop counter */
        if (counter % 3 == 0) {
            sum += __builtin_ffs(counter + 1);  /* Find first set bit */
        } else if (counter % 3 == 1) {
            /* Use __builtin_sqrtf with type conversion */
            float val = (float)(counter + 1);
            sum += (int)__builtin_sqrtf(val);
        } else {
            /* Use parity built-in */
            sum += __builtin_parity(counter);
        }
        
        /* Optimization barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Function that declares and uses built-in prototypes */
static int __attribute__((noinline))
explicit_builtin_declaration_test(int x) {
    /* Explicit declaration mimicking built-in prototype */
    int __builtin_popcount(unsigned int);
    int __builtin_ctz(unsigned int);
    
    /* Use the declared built-ins */
    unsigned int val = (unsigned int)(x ^ 0x12345678);
    int popcnt = __builtin_popcount(val);
    int ctz = __builtin_ctz(val | 1);  /* Avoid undefined for 0 */
    
    return popcnt * 100 + ctz;
}

/* External function definition (simulating another translation unit) */
int external_builtin_user(int x) {
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();  /* Mark path as unreachable */
    }
    
    /* Use __builtin_constant_p to check if value is constant */
    if (__builtin_constant_p(x)) {
        return __builtin_abs(x) * 2;
    } else {
        return __builtin_abs(x) * 3;
    }
}

/* Another external function with visibility attributes */
void __attribute__((visibility("hidden"), nothrow))
external_visibility_test(void) {
    /* Use __builtin_trap under specific condition */
    volatile int flag = volatile_seed % 100;
    if (flag == 0) {
        __builtin_trap();  /* Generate trap instruction */
    }
    
    /* Use memory built-in */
    void* ptr = &flag;
    __builtin_prefetch(ptr, 0, 3);  /* Prefetch for read, high temporal locality */
}

/* Function using synchronization built-ins */
static int __attribute__((noinline))
test_sync_builtins(int* shared) {
    /* Use atomic built-ins */
    int old = __sync_fetch_and_add(shared, 1);
    
    /* Use __builtin_expect with atomic result */
    if (__builtin_expect(old > 1000, 0)) {
        __sync_lock_release(shared);
        return 1;
    }
    
    return 0;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize volatile seed from argc and time */
    volatile_seed = argc + (int)time(NULL);
    srand(volatile_seed);
    
    int result = 0;
    
    /* Test 1: Arithmetic built-ins with volatile variables */
    for (volatile int i = 0; i < 10; i++) {
        int val = volatile_seed + i;
        result += __builtin_abs(val % 100 - 50);
        
        /* Use __builtin_expect in loop condition */
        if (__builtin_expect(i == 5, 0)) {
            result += 1000;
        }
    }
    
    /* Test 2: Call attributed function */
    result += attributed_builtin_wrapper(volatile_seed, -volatile_seed);
    
    /* Test 3: Test overflow operations */
    result += test_overflow_operations(volatile_seed, volatile_seed / 2 + 1);
    
    /* Test 4: Built-ins in loops */
    result += test_builtin_in_loop(20);
    
    /* Test 5: Explicit built-in declaration */
    result += explicit_builtin_declaration_test(volatile_seed);
    
    /* Test 6: External function calls */
    result += external_builtin_user(result);
    
    /* Test 7: Visibility function */
    result += hidden_visibility_func((unsigned int)result);
    
    /* Test 8: Call external visibility test */
    external_visibility_test();
    
    /* Test 9: Sync built-ins */
    int shared = 0;
    result += test_sync_builtins(&shared);
    
    /* Test 10: Various other built-ins */
    float fval = (float)result;
    result += (int)__builtin_sqrtf(fval + 100.0f);
    
    /* Use __builtin_choose_expr */
    int choice = __builtin_choose_expr(
        result > 1000,
        __builtin_clz(result),
        __builtin_ctz(result | 1)
    );
    result += choice;
    
    /* Final: Use result to prevent dead code elimination */
    volatile_accumulator = result;
    
    /* Print result to ensure all code is live */
    printf("Result: %d (Seed: %d)\n", result, volatile_seed);
    
    return result % 256;
}
