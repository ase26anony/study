/* test_builtin_hooks.c - Comprehensive built-in function usage to trigger targhooks.cc flags */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;
static volatile int optimization_barrier = 0;

/* Helper with attributes that may interact with built-in processing */
static int __attribute__((used, noinline, noclone)) 
force_builtin_processing(int x) {
    /* Use multiple built-ins in sequence */
    int abs_val = __builtin_abs(x);
    int popcnt = __builtin_popcount((unsigned int)abs_val);
    int clz = __builtin_clz((unsigned int)popcnt);
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (abs_val), "+r" (popcnt), "+r" (clz));
    
    return abs_val + popcnt + clz;
}

/* Function with explicit hidden visibility and nothrow */
static void __attribute__((visibility("hidden"), nothrow, used))
attributed_builtin_user(int seed) {
    /* Use builtin_expect to influence branch prediction */
    if (__builtin_expect(seed > 100, 0)) {
        /* Use builtin_unreachable for impossible paths */
        __builtin_unreachable();
    }
    
    /* Use overflow checking built-ins */
    int result;
    if (__builtin_add_overflow(seed, 42, &result)) {
        /* Handle overflow */
        result = seed;
    }
    
    /* Use math built-in */
    float sqrt_val = __builtin_sqrtf((float)result);
    
    /* Store in volatile to prevent elimination */
    optimization_barrier = (int)sqrt_val;
}

/* Test arithmetic built-ins with loop barrier */
static int __attribute__((noinline))
test_builtin_arithmetic(volatile int seed) {
    int sum = 0;
    
    /* Loop with volatile counter to prevent unrolling */
    for (volatile int i = 0; i < 3; i++) {
        /* Use different math built-ins */
        int abs_val = __builtin_abs(seed + i);
        float sqrt_val = __builtin_sqrtf((float)abs_val);
        
        /* Use bit built-ins */
        int clz = __builtin_clz((unsigned int)abs_val);
        int ctz = __builtin_ctz((unsigned int)abs_val ^ 1);
        
        sum += abs_val + (int)sqrt_val + clz + ctz;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Test bit operation built-ins */
static int __attribute__((noinline))
test_builtin_bitops(volatile unsigned int val) {
    int result = 0;
    
    /* Chain multiple built-in calls */
    result += __builtin_popcount(val);
    result += __builtin_parity(val);
    result += __builtin_ffs(val);
    
    /* Use with non-constant arguments */
    volatile unsigned int shifted = val >> 3;
    result += __builtin_clz(shifted | 1);  /* Ensure non-zero */
    result += __builtin_ctz(shifted | 1);
    
    /* Use builtin_choose_expr */
    int choice = __builtin_choose_expr(
        (val & 1) == 0,
        __builtin_popcount(val),
        __builtin_parity(val)
    );
    
    result += choice;
    
    return result;
}

/* Test overflow built-ins with branching */
static int __attribute__((noinline))
test_builtin_overflow(volatile int a, volatile int b) {
    int res1, res2, res3;
    int overflow_flag = 0;
    
    /* Test different overflow operations */
    if (__builtin_add_overflow(a, b, &res1)) {
        overflow_flag |= 1;
    }
    
    if (__builtin_mul_overflow(a, b, &res2)) {
        overflow_flag |= 2;
    }
    
    if (__builtin_sub_overflow(a, b, &res3)) {
        overflow_flag |= 4;
    }
    
    /* Use builtin_constant_p */
    if (__builtin_constant_p(a)) {
        res1 = 0;
    }
    
    /* Combine results with memory barrier */
    asm volatile("" : "+r" (res1), "+r" (res2), "+r" (res3));
    
    return res1 + res2 + res3 + overflow_flag;
}

/* External function definition (simulating another TU) */
int external_builtin_user(int x) {
    /* Use builtin_expect with external linkage */
    if (__builtin_expect(x < 0, 0)) {
        return __builtin_abs(x);
    }
    
    /* Use builtin_unreachable for error case */
    if (x > 1000) {
        __builtin_unreachable();
    }
    
    /* Complex built-in usage */
    int result;
    if (__builtin_sadd_overflow(x, 100, &result)) {
        return x;
    }
    
    return result;
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden")))
hidden_visibility_func(void) {
    /* Use sync built-ins */
    int val = __sync_fetch_and_add(&global_counter, 1);
    
    /* Use builtin_trap for extreme cases */
    if (val > 10000) {
        __builtin_trap();
    }
    
    /* Use prefetch built-in */
    __builtin_prefetch(&global_counter, 0, 3);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    volatile unsigned int bit_seed = (unsigned int)time(NULL);
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(bit_seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Force built-in processing with attributes */
    checksum += force_builtin_processing(seed);
    
    /* Test 5: Call attributed function */
    attributed_builtin_user(seed);
    checksum += optimization_barrier;
    
    /* Test 6: External linkage simulation */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    checksum += global_counter;
    
    /* Test 8: Direct built-in calls in main */
    int direct_result = 0;
    direct_result += __builtin_abs(seed - 10);
    direct_result += __builtin_popcount((unsigned int)checksum);
    
    /* Use builtin_expect to influence control flow */
    if (__builtin_expect(checksum > 0, 1)) {
        direct_result += __builtin_clz((unsigned int)checksum);
    }
    
    /* Final result with optimization barrier */
    asm volatile("" : "+r" (direct_result));
    checksum += direct_result;
    
    /* Print to prevent dead code elimination */
    printf("Built-in test checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
