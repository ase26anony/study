/* test_targhooks.c - Comprehensive built-in function usage test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Prevent optimization */
static volatile int global_volatile = 0;
static int checksum = 0;

/* Barrier to prevent optimization */
#define OPTIMIZATION_BARRIER() asm volatile("" : : : "memory")

/* Helper with attributes that might interact with the hook */
static int __attribute__((used, noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    volatile int i;
    
    /* Use math built-ins in a loop */
    for (i = 0; i < 5; i++) {
        int val = seed + i * 100;
        
        /* Various arithmetic built-ins */
        result += __builtin_abs(val);
        result += (int)__builtin_sqrtf((float)(val > 0 ? val : -val));
        result += __builtin_ffs(val);  /* Find first set bit */
        
        /* Prevent dead code elimination */
        OPTIMIZATION_BARRIER();
    }
    
    return result;
}

/* Bit manipulation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed)
{
    unsigned int x = seed ^ 0xDEADBEEF;
    int result = 0;
    
    /* Bit operation built-ins */
    result += __builtin_popcount(x);
    result += __builtin_clz(x | 1);  /* Ensure non-zero */
    result += __builtin_ctz(x | 1);  /* Ensure non-zero */
    result += __builtin_parity(x);
    
    /* Store in volatile to prevent optimization */
    global_volatile = result;
    
    return result;
}

/* Overflow checking built-ins */
static int __attribute__((noinline, noclone))
test_builtin_overflow(int a, int b)
{
    int result = 0;
    int overflow;
    
    /* Overflow built-ins with branching */
    if (__builtin_add_overflow(a, b, &result)) {
        result = __builtin_abs(a);
    }
    
    if (__builtin_mul_overflow(a, b, &result)) {
        result = __builtin_abs(b);
    }
    
    if (__builtin_sub_overflow(a, b, &result)) {
        result = __builtin_abs(a - b);
    }
    
    /* Use __builtin_expect */
    if (__builtin_expect(result > 1000, 0)) {
        result = 1000;
    }
    
    return result;
}

/* Function with explicit visibility and other attributes */
static void __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(volatile int *ptr)
{
    /* Use built-in inside attributed function */
    int val = *ptr;
    
    if (__builtin_expect(val < 0, 0)) {
        *ptr = __builtin_abs(val);
    }
    
    /* Memory barrier */
    OPTIMIZATION_BARRIER();
}

/* Declare built-in function prototype (important for triggering the hook) */
int __builtin_popcount(unsigned int) __attribute__((visibility("hidden")));
int __builtin_clz(unsigned int) __attribute__((visibility("hidden")));
int __builtin_abs(int) __attribute__((visibility("hidden"), nothrow));

/* External function definition (simulating another translation unit) */
int __attribute__((visibility("hidden"), nothrow))
external_builtin_user(int x)
{
    /* Use multiple built-ins */
    if (x == 0) {
        __builtin_unreachable();
    }
    
    int result = __builtin_abs(x);
    result += __builtin_popcount((unsigned int)x);
    
    /* Complex expression with built-in */
    return __builtin_expect(result > 100, 0) ? 100 : result;
}

/* Another external-like function */
static void __attribute__((used, noinline))
hidden_visibility_func(void)
{
    volatile int x = global_volatile;
    
    /* Use built-in with volatile */
    int y = __builtin_abs(x);
    
    /* Store to global to prevent elimination */
    checksum += y;
    
    /* Assembly barrier */
    asm volatile("" : "+r"(y) : : "memory");
}

/* Function that might trigger unreachable */
static int __attribute__((noinline, noclone))
test_unreachable(int x)
{
    if (x < 0) {
        /* This should trigger builtin_unreachable processing */
        __builtin_unreachable();
    }
    
    return __builtin_abs(x) + __builtin_popcount((unsigned int)x);
}

/* Main test driver */
int main(int argc, char **argv)
{
    volatile int seed;
    
    /* Initialize seed from various sources */
    seed = argc;
    seed ^= (int)time(NULL);
    seed ^= (int)clock();
    
    /* Test 1: Arithmetic built-ins */
    int r1 = test_builtin_arithmetic(seed);
    checksum += r1;
    
    /* Test 2: Bit operation built-ins */
    int r2 = test_builtin_bitops((unsigned int)seed);
    checksum += r2;
    
    /* Test 3: Overflow built-ins */
    int r3 = test_builtin_overflow(seed, seed * 2);
    checksum += r3;
    
    /* Test 4: Call attributed function */
    volatile int attr_param = seed;
    test_attributed_function(&attr_param);
    checksum += attr_param;
    
    /* Test 5: External-like function */
    int r5 = external_builtin_user(seed);
    checksum += r5;
    
    /* Test 6: Hidden visibility function */
    hidden_visibility_func();
    
    /* Test 7: Unreachable built-in */
    int r7 = test_unreachable(seed > 0 ? seed : -seed);
    checksum += r7;
    
    /* Additional direct built-in usage */
    volatile int direct_result = 0;
    direct_result += __builtin_abs(seed);
    direct_result += __builtin_popcount((unsigned int)seed);
    direct_result += __builtin_clz((unsigned int)(seed | 1));
    
    /* Use __builtin_constant_p */
    if (!__builtin_constant_p(seed)) {
        direct_result += __builtin_ffs(seed | 1);
    }
    
    checksum += direct_result;
    
    /* Final result that can't be optimized away */
    printf("Checksum: %d\n", checksum);
    
    /* Use __builtin_trap in a conditional path */
    if (checksum < 0) {
        __builtin_trap();
    }
    
    return checksum & 0xFF;
}
