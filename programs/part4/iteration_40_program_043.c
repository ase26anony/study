/* test_targhooks.c - Comprehensive built-in function test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations with various attributes */
extern int external_builtin_user(int x) __attribute__((visibility("hidden")));
static int static_builtin_wrapper(int x) __attribute__((used, nothrow));

/* Built-in function prototypes - these trigger declaration processing */
int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
int __builtin_clz(unsigned int) __attribute__((used));
int __builtin_ctz(unsigned int);
long __builtin_expect(long, long) __attribute__((nothrow));
int __builtin_abs(int) __attribute__((visibility("hidden"), nothrow));

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;
static volatile int volatile_accumulator = 0;

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Helper function with noinline to prevent optimization */
static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    volatile int local_volatile = seed;
    
    /* Use math built-ins in a loop */
    for (int i = 0; i < 10; i++) {
        COMPILER_BARRIER();
        int val = local_volatile + i;
        
        /* Built-in with explicit visibility attribute */
        int abs_val = __builtin_abs(val);
        
        /* Another built-in call */
        float sqrt_val = __builtin_sqrtf((float)abs_val);
        
        result += (int)sqrt_val;
        COMPILER_BARRIER();
    }
    
    volatile_accumulator = result;
    return result;
}

/* Bit operation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed)
{
    unsigned int val = seed;
    int result = 0;
    
    /* Chain multiple built-in calls */
    result += __builtin_popcount(val);
    result += __builtin_clz(val);
    result += __builtin_ctz(val | 1); /* Avoid undefined behavior */
    
    /* Built-in with expect */
    if (__builtin_expect((val & 1), 1)) {
        result += __builtin_popcount(val ^ 0xAAAAAAAA);
    }
    
    /* Store to volatile to ensure side effects */
    volatile_counter = result;
    return result;
}

/* Overflow checking built-ins */
static int __attribute__((noinline, noclone, visibility("hidden")))
test_builtin_overflow(int a, int b)
{
    int result = 0;
    int overflow;
    
    /* Multiple overflow built-in calls */
    if (__builtin_add_overflow(a, b, &result)) {
        COMPILER_BARRIER();
        result = __builtin_abs(a);
    }
    
    if (__builtin_mul_overflow(a, b, &overflow)) {
        result += __builtin_clz((unsigned int)overflow);
    }
    
    /* Sub overflow */
    if (__builtin_sub_overflow(a, b, &overflow)) {
        result += 1;
    }
    
    return result;
}

/* Function with multiple attributes that should trigger flag setting */
static int __attribute__((visibility("hidden"), nothrow, used, noinline))
static_builtin_wrapper(int x)
{
    /* Use builtin_expect with attributes */
    if (__builtin_expect(x > 0, 1)) {
        return __builtin_abs(x) + __builtin_popcount((unsigned int)x);
    }
    return 0;
}

/* External function definition */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int x)
{
    /* Complex expression with built-in */
    int result = __builtin_popcount((unsigned int)x) * 2;
    
    /* Conditional unreachable built-in */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    /* Memory clobber to prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Another static function calling built-ins */
static int __attribute__((used))
another_static_func(volatile int *ptr)
{
    int val = *ptr;
    int result = 0;
    
    /* Mix of different built-ins */
    result = __builtin_ffs(val);
    result += __builtin_parity(val);
    
    /* Built-in with assembly barrier */
    asm volatile("" : : "r"(result) : "memory");
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[])
{
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    int checksum = 0;
    
    /* Initialize with time for more randomness */
    srand(time(NULL));
    seed += rand();
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    checksum += test_builtin_overflow(seed, seed * 2);
    
    /* Test 4: Static function with attributes */
    checksum += static_builtin_wrapper(seed);
    
    /* Test 5: External function */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Direct built-in calls with volatile storage */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed);
    direct_result += __builtin_clz((unsigned int)seed | 1);
    direct_result += __builtin_popcount((unsigned int)seed);
    
    /* Use assembly to ensure built-in isn't optimized away */
    asm volatile("" : "+r"(direct_result));
    checksum += direct_result;
    
    /* Test 7: Another static function */
    checksum += another_static_func(&seed);
    
    /* Test 8: Built-in in conditional with volatile */
    volatile int cond_test = 0;
    for (int i = 0; i < 5; i++) {
        cond_test += i;
        if (__builtin_expect(cond_test > 2, 0)) {
            cond_test += __builtin_ctz((unsigned int)cond_test);
        }
    }
    checksum += cond_test;
    
    /* Test 9: Nested built-in calls */
    int nested = __builtin_abs(__builtin_popcount(seed) - 10);
    checksum += nested;
    
    /* Test 10: Built-in with pointer arguments */
    int overflow_flag;
    __builtin_add_overflow(checksum, seed, &checksum);
    __builtin_mul_overflow(checksum, 3, &checksum);
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum & 0xFF; /* Return non-zero to indicate execution */
}
