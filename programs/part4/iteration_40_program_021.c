/* test_builtin_hooks.c - Comprehensive test for GCC built-in function hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern int hidden_visibility_func(int x) __attribute__((visibility("hidden")));

/* Prevent optimization barriers */
static volatile int global_side_effect = 0;

/* ====== Pattern 1: Direct built-in usage with volatile variables ====== */

/* Function with multiple attributes that may interact with the hook */
static int __attribute__((used, noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    volatile int v = seed;
    
    /* Use various arithmetic built-ins */
    result += __builtin_abs(v);           /* Absolute value */
    result += __builtin_clz(v | 1);       /* Count leading zeros */
    result += __builtin_popcount(v);      /* Population count */
    
    /* Floating point built-in with cast */
    float f = (float)v;
    result += (int)__builtin_sqrtf(f * f);
    
    /* Store to volatile to prevent elimination */
    global_side_effect = result;
    return result;
}

/* ====== Pattern 2: Built-ins in loops with optimization barriers ====== */

static int __attribute__((noinline))
test_builtin_bitops(volatile int seed)
{
    int sum = 0;
    volatile int i;
    
    for (i = 0; i < 5; i++) {
        /* Mix built-ins with inline assembly barrier */
        int val = seed + i;
        sum += __builtin_ctz(val | 1);    /* Count trailing zeros */
        sum += __builtin_parity(val);     /* Parity */
        
        /* Optimization barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Use __builtin_expect to influence branching */
    if (__builtin_expect(sum > 100, 0)) {
        sum = 100;
    }
    
    return sum;
}

/* ====== Pattern 3: Overflow checking built-ins with branching ====== */

static int __attribute__((nothrow, noinline))
test_builtin_overflow(volatile int a, volatile int b)
{
    int result = 0;
    int overflow;
    
    /* Addition overflow check */
    if (__builtin_add_overflow(a, b, &result)) {
        result = a;
    }
    
    /* Multiplication overflow check */
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result += b;
    } else {
        result += mul_result;
    }
    
    /* Subtraction overflow check */
    int sub_result;
    __builtin_sub_overflow(a, b, &sub_result);
    result += sub_result;
    
    return result;
}

/* ====== Pattern 4: Function with explicit hidden visibility ====== */

/* This function has attributes matching the uncovered lines */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
hidden_builtin_user(volatile int x)
{
    /* Use __builtin_expect inside */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) + __builtin_clz(x | 1);
    }
    return __builtin_popcount(x);
}

/* ====== Pattern 5: External linkage simulation ====== */

/* Forward declaration to simulate external linkage */
static int internal_external_sim(volatile int x);

/* Function that might trigger unreachable built-in */
static int __attribute__((noinline))
test_external_linkage(volatile int x)
{
    /* Call the "external" function */
    int r1 = internal_external_sim(x);
    
    /* Call the truly external declaration */
    int r2 = external_builtin_user(x);
    
    /* Call hidden visibility function */
    int r3 = hidden_visibility_func(x);
    
    return r1 + r2 + r3;
}

/* The "external" function implementation */
static int
internal_external_sim(volatile int x)
{
    if (x < 0) {
        /* This should trigger unreachable in some paths */
        __builtin_unreachable();
        return 0;
    }
    
    /* Use built-in with side effects */
    return __builtin_sadd_overflow(x, 1, &x) ? x : x + 1;
}

/* ====== External function definitions (simulating another TU) ====== */

/* Definition of externally declared function */
int external_builtin_user(int x)
{
    volatile int v = x;
    /* Use multiple built-ins */
    int a = __builtin_ffs(v);      /* Find first set */
    int b = __builtin_clrsb(v);    /* Count leading redundant sign bits */
    return a + b;
}

/* Definition of hidden visibility function */
int __attribute__((visibility("hidden"), nothrow))
hidden_visibility_func(int x)
{
    /* Complex built-in usage */
    int result = 0;
    result += __builtin_bswap16(x & 0xFFFF);  /* Byte swap */
    result += __builtin_rotateleft32(x, 3);   /* Rotation */
    return result;
}

/* ====== Pattern 6: Built-ins in complex expressions ====== */

static int __attribute__((noinline))
test_complex_expressions(volatile int a, volatile int b)
{
    /* Nested built-in calls */
    int x = __builtin_abs(__builtin_abs(a) - __builtin_abs(b));
    
    /* Built-in in ternary expression */
    int y = __builtin_expect(a > b, 1) 
            ? __builtin_clz(a) 
            : __builtin_ctz(b | 1);
    
    /* Built-in with pointer dereference */
    volatile int arr[4] = {a, b, a+b, a-b};
    int z = __builtin_popcount(arr[__builtin_abs(a) % 4]);
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(a)) {
        z += __builtin_bswap32(x);
    }
    
    return x + y + z;
}

/* ====== Main function orchestrating all tests ====== */

int main(int argc, char *argv[])
{
    volatile int seed;
    
    /* Initialize seed from various sources to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFF;
    }
    
    /* Also use volatile to ensure the value can't be optimized away */
    volatile int v1 = seed * 3 + 1;
    volatile int v2 = seed * 7 - 3;
    
    int checksum = 0;
    
    /* Execute all test patterns */
    checksum += test_builtin_arithmetic(v1);
    checksum += test_builtin_bitops(v2);
    checksum += test_builtin_overflow(v1, v2);
    checksum += hidden_builtin_user(v1);
    checksum += test_external_linkage(v2);
    checksum += test_complex_expressions(v1, v2);
    
    /* Use built-in in main as well */
    checksum += __builtin_popcount(checksum);
    
    /* Final result depends on all computations */
    printf("Result: %d (Side effect: %d)\n", checksum, global_side_effect);
    
    return checksum & 0xFF;
}

/* ====== Additional static functions with different attributes ====== */

/* Static function calling built-in - may trigger static declaration handling */
static void __attribute__((constructor))
init_function(void)
{
    volatile int init_val = 42;
    global_side_effect += __builtin_abs(init_val);
}

/* Function with artificial-like usage */
static int __attribute__((used, noinline))
artificial_builtin_user(void)
{
    /* Use synchronization built-in */
    int val = 1;
    __atomic_store_n(&global_side_effect, val, __ATOMIC_RELAXED);
    
    /* Use __builtin_trap under condition */
    if (global_side_effect > 1000) {
        __builtin_trap();
    }
    
    return __builtin_ffs(global_side_effect);
}
