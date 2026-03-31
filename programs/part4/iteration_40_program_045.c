/* test_builtin_hooks.c - Comprehensive built-in function test to trigger targhooks.cc flags */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int global_volatile_counter = 0;
static volatile int optimization_barrier = 0;

/* ==================== HELPER FUNCTIONS WITH ATTRIBUTES ==================== */

/* Function with multiple attributes that may interact with the hook */
static int __attribute__((used, noinline, noclone)) 
test_attributed_function(int x) 
{
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x);
    }
    return 0;
}

/* Function with hidden visibility and nothrow attributes */
static void __attribute__((visibility("hidden"), nothrow, used, noinline))
hidden_visibility_func(void)
{
    /* Use memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Use built-in with volatile variable */
    int result = __builtin_popcount(global_volatile_counter);
    optimization_barrier = result;
}

/* ==================== BUILT-IN TEST FUNCTIONS ==================== */

/* Test arithmetic built-ins in a loop */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    volatile int accumulator = 0;
    
    for (volatile int i = 0; i < 5; i++) {
        /* Use different arithmetic built-ins */
        int val = seed + i;
        accumulator += __builtin_abs(val);
        
        /* Use floating-point built-in with cast */
        if (val != 0) {
            float fval = (float)val;
            /* __builtin_sqrtf might be recognized */
            accumulator += (int)(fval * 10);
        }
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    return accumulator;
}

/* Test bit operation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed)
{
    volatile unsigned int result = 0;
    
    /* Use various bit operation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Use built-in with side effects */
    unsigned int rotated = __builtin_rotateleft32(seed, 3);
    result += rotated;
    
    return result;
}

/* Test overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b)
{
    volatile int result = 0;
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

/* Test control flow built-ins */
static int __attribute__((noinline, noclone))
test_builtin_control_flow(int x)
{
    volatile int result = 0;
    
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect in loop */
    for (int i = 0; i < 10; i++) {
        if (__builtin_expect(i < x, 0)) {
            result += i;
        }
    }
    
    /* Use __builtin_prefetch */
    int array[100];
    __builtin_prefetch(&array[x % 100], 0, 0);
    
    return result;
}

/* ==================== EXTERNALLY DECLARED FUNCTIONS ==================== */

/* This function is declared extern earlier but defined here */
int __attribute__((used, noinline))
external_builtin_user(int x)
{
    /* Use multiple built-ins */
    int result = __builtin_abs(x);
    result += __builtin_popcount((unsigned int)x);
    
    /* Conditional __builtin_unreachable */
    if (x == 0xDEADBEEF) {
        __builtin_unreachable();
    }
    
    return result;
}

/* Function with assembly and built-ins */
static void __attribute__((noinline))
test_builtin_with_asm(volatile int *output)
{
    int temp;
    
    /* Inline assembly with built-in */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp)
        : "r" (global_volatile_counter)
    );
    
    temp = __builtin_bswap32(temp);
    *output = temp;
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char *argv[])
{
    /* Initialize volatile seed from argc and time */
    volatile int seed = argc;
    seed += (int)time(NULL) & 0xFF;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, 1000);
    
    /* Test 4: Control flow built-ins */
    checksum += test_builtin_control_flow(seed % 20);
    
    /* Test 5: Attributed function */
    checksum += test_attributed_function(seed);
    
    /* Test 6: External linkage function */
    checksum += external_builtin_user(seed);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    checksum += optimization_barrier;
    
    /* Test 8: Built-in with inline assembly */
    int asm_result;
    test_builtin_with_asm(&asm_result);
    checksum += asm_result;
    
    /* Test 9: Direct built-in calls with volatile results */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed - 50);
    direct_result += __builtin_clz(seed | 1);
    direct_result += __builtin_ffs(seed | 1);
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        direct_result += 100;
    }
    
    checksum += direct_result;
    
    /* Test 10: Complex expression with multiple built-ins */
    volatile int complex_expr = 0;
    for (volatile int i = 0; i < 3; i++) {
        int val = seed + i;
        complex_expr += __builtin_popcount(__builtin_abs(val));
        
        /* Nested built-in calls */
        complex_expr += __builtin_ctz(__builtin_ffs(val | 1));
    }
    checksum += complex_expr;
    
    /* Final output to prevent dead code elimination */
    printf("Built-in test checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional function to increase declaration count */
static int __attribute__((unused, noinline))
unused_builtin_wrapper(int x)
{
    /* Declare built-in prototype locally */
    int __builtin_dummy(int) __attribute__((used));
    
    return __builtin_abs(x) + __builtin_popcount(x);
}
