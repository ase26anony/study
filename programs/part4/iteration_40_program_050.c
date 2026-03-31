/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_test(void);

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int result_accumulator = 0;

/* ==================== HELPER FUNCTIONS WITH ATTRIBUTES ==================== */

/* Function with explicit hidden visibility and nothrow attribute */
static int __attribute__((visibility("hidden"), nothrow, used))
hidden_visibility_func(int x) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function marked as used to force emission */
static void __attribute__((used, noinline, noclone))
force_used_function(void) {
    volatile int temp = 0;
    /* Use multiple built-ins in sequence */
    temp += __builtin_popcount(0xABCDEF);
    temp += __builtin_clz(0x1000);
    result_accumulator += temp;
}

/* ==================== BUILT-IN TEST FUNCTIONS ==================== */

/* Test arithmetic built-ins with volatile barriers */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed) {
    int sum = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent optimization */
    for (i = 0; i < 5; i++) {
        /* Use different arithmetic built-ins */
        sum += __builtin_abs(seed + i - 10);
        
        /* Use __builtin_sqrtf with type conversion */
        float fval = (float)(seed + i);
        sum += (int)__builtin_sqrtf(fval);
        
        /* Optimization barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Test bit operation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(volatile unsigned int seed) {
    unsigned int result = 0;
    
    /* Chain built-in calls with volatile intermediate */
    volatile unsigned int temp = seed;
    
    result = __builtin_popcount(temp);
    temp = result * 0x1234;
    result += __builtin_clz(temp);
    temp = result | 0xF0F0F0F0;
    result += __builtin_ctz(temp);
    
    /* Use __builtin_ffs (find first set) */
    result += __builtin_ffs((int)seed);
    
    return (int)result;
}

/* Test overflow checking built-ins with branching */
static int __attribute__((noinline, noclone))
test_builtin_overflow(volatile int a, volatile int b) {
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

/* Test expectation and unreachable built-ins */
static int __attribute__((noinline, noclone))
test_builtin_control(volatile int x) {
    int result = 0;
    
    /* Use __builtin_expect in condition */
    if (__builtin_expect(x % 2 == 0, 0)) {
        result = x / 2;
    } else {
        result = x * 3 + 1;
    }
    
    /* Use __builtin_unreachable for impossible case */
    if (result < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_assume for optimization hint */
    if (result > 1000) {
        __builtin_assume(result < 10000);
    }
    
    return result;
}

/* ==================== EXTERNALLY DECLARED FUNCTIONS ==================== */

/* Function with external linkage that uses built-ins */
int __attribute__((visibility("default")))
external_builtin_user(int x) {
    int result = 0;
    
    /* Use __builtin_constant_p to check for constants */
    if (__builtin_constant_p(x)) {
        result = __builtin_abs(x);
    } else {
        result = __builtin_popcount((unsigned int)x);
    }
    
    /* Use __builtin_trap for extreme cases */
    if (result > 1000000) {
        __builtin_trap();
    }
    
    return result;
}

/* Function testing visibility attributes with built-ins */
void __attribute__((visibility("hidden"), nothrow))
external_visibility_test(void) {
    volatile int x = 42;
    
    /* Use __builtin_prefetch */
    int array[100];
    __builtin_prefetch(&array[0], 0, 0);
    
    /* Use __builtin_frame_address */
    void *frame_addr = __builtin_frame_address(0);
    x += (int)((long)frame_addr & 0xFF);
    
    /* Store to volatile to ensure side effect */
    result_accumulator += x;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char *argv[]) {
    volatile int seed;
    int final_result = 0;
    
    /* Initialize seed from various sources */
    seed = argc;
    seed += (int)time(NULL) & 0xFF;
    seed += global_counter;
    
    printf("Starting built-in function tests with seed=%d\n", seed);
    
    /* Test 1: Arithmetic built-ins */
    final_result += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    final_result += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow checking built-ins */
    final_result += test_builtin_overflow(seed, seed + 7);
    
    /* Test 4: Control flow built-ins */
    final_result += test_builtin_control(seed);
    
    /* Test 5: Hidden visibility function with built-ins */
    final_result += hidden_visibility_func(seed);
    
    /* Test 6: Force used function */
    force_used_function();
    
    /* Test 7: External linkage functions */
    final_result += external_builtin_user(seed);
    external_visibility_test();
    
    /* Test 8: Inline built-in usage with volatile storage */
    volatile int inline_result = 0;
    inline_result += __builtin_abs(seed - 100);
    inline_result += __builtin_popcount(0xDEADBEEF);
    inline_result += __builtin_clz(1 << 20);
    
    /* Use __builtin_choose_expr */
    int chosen = __builtin_choose_expr(seed > 0, 
                                      __builtin_abs(seed), 
                                      __builtin_popcount((unsigned int)-seed));
    inline_result += chosen;
    
    /* Final accumulation with optimization barrier */
    asm volatile("" : : : "memory");
    final_result += inline_result + result_accumulator;
    
    printf("Final result: %d\n", final_result);
    
    /* Return non-zero result to indicate execution */
    return (final_result != 0) ? 0 : 1;
}

/* Additional static function with complex built-in usage */
static int __attribute__((unused))
unused_function_with_builtins(void) {
    /* This function may be eliminated but its declaration should be processed */
    volatile int x = 0;
    
    /* Use __builtin_nan and __builtin_nans */
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nans("");
    x += (int)(nan1 != nan2);
    
    /* Use __builtin_inf */
    float inf = __builtin_inff();
    x += (inf > 0);
    
    return x;
}
