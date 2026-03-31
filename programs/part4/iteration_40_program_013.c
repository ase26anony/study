/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_seed = 0;
static volatile int volatile_result = 0;

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
    
    /* Call built-in with volatile variable */
    int temp = __builtin_popcount(volatile_seed);
    volatile_result += temp;
    
    /* Another built-in call */
    if (__builtin_constant_p(volatile_seed)) {
        /* This path shouldn't be taken due to volatile */
        volatile_result = 0;
    }
}

/* ==================== BUILT-IN TEST FUNCTIONS ==================== */

static int __attribute__((noinline, noclone))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    
    /* Loop with volatile counter to prevent dead code elimination */
    for (volatile int i = 0; i < 3; i++) {
        /* Use math built-ins */
        result += __builtin_abs(seed + i);
        
        /* Use floating-point built-in with cast */
        float fval = (float)(seed + i);
        result += (int)__builtin_sqrtf(fval + 1.0f);
    }
    
    return result;
}

static int __attribute__((noinline, noclone))
test_builtin_bitops(int seed)
{
    int result = 0;
    
    /* Bit manipulation built-ins */
    result += __builtin_popcount(seed);
    result += __builtin_clz(seed | 1);  /* Ensure non-zero */
    result += __builtin_ctz(seed | 1);
    result += __builtin_ffs(seed);
    
    /* Built-in with side effects through volatile */
    volatile int v = seed;
    result += __builtin_parity(v);
    
    return result;
}

static int __attribute__((noinline, noclone))
test_builtin_overflow(int seed)
{
    int result = 0;
    int overflow;
    
    /* Overflow checking built-ins */
    if (__builtin_add_overflow(seed, 1000, &result)) {
        result = seed;
    }
    
    int mul_result;
    if (__builtin_mul_overflow(seed, 2, &mul_result)) {
        result += 1;
    } else {
        result += mul_result;
    }
    
    /* Sub and signed overflow */
    int sub_result;
    __builtin_sub_overflow(seed, 500, &sub_result);
    result += sub_result;
    
    return result;
}

static void __attribute__((noinline, noclone))
test_builtin_memory(void)
{
    /* Memory built-ins with optimization barriers */
    int arr[10];
    
    /* Initialize with built-in */
    __builtin_memset(arr, 0, sizeof(arr));
    
    /* Copy with built-in */
    int arr2[10];
    __builtin_memcpy(arr2, arr, sizeof(arr));
    
    /* Compare with built-in */
    if (__builtin_memcmp(arr, arr2, sizeof(arr)) == 0) {
        volatile_result += 1;
    }
}

/* ==================== EXTERNAL-LIKE FUNCTIONS ==================== */

/* Forward declaration to simulate external linkage */
static int external_like_function(int x);

/* Function that calls the external-like function */
static int __attribute__((noinline))
test_external_linkage(int seed)
{
    /* Call function declared as extern (but defined in same file) */
    return external_like_function(seed);
}

/* The "external" function definition */
static int __attribute__((used, noinline))
external_like_function(int x)
{
    int result = x;
    
    /* Use __builtin_unreachable under specific condition */
    if (x < 0) {
        __builtin_unreachable();  /* Hint to optimizer */
    }
    
    /* Use built-in with optimization barrier */
    asm volatile("" : "+r"(result) : : "memory");
    
    /* More built-ins */
    result += __builtin_bswap16(x & 0xFFFF);
    result += __builtin_bswap32(x);
    
    return result;
}

/* ==================== COMPLEX EXPRESSION INTEGRATION ==================== */

static int __attribute__((noinline, noclone))
test_complex_expressions(int seed)
{
    int result = 0;
    
    /* Complex expression mixing multiple built-ins */
    result = __builtin_abs(__builtin_popcount(seed) - 
                          __builtin_clz(seed | 1));
    
    /* Built-in in conditional */
    result += __builtin_expect(seed > 0, 1) ? 
              __builtin_ffs(seed) : __builtin_ctz(~seed);
    
    /* Chained built-in calls */
    int temp = __builtin_add_overflow_p(seed, 100, 0);
    result += temp ? __builtin_abs(seed) : __builtin_popcount(seed);
    
    return result;
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char *argv[])
{
    /* Initialize volatile seed from runtime sources */
    volatile_seed = argc;
    volatile_seed += (int)time(NULL) & 0xFF;
    
    int final_result = 0;
    
    /* Call all test functions to ensure they're used */
    final_result += test_builtin_arithmetic(volatile_seed);
    final_result += test_builtin_bitops(volatile_seed);
    final_result += test_builtin_overflow(volatile_seed);
    final_result += test_complex_expressions(volatile_seed);
    
    /* Test attributed function */
    final_result += test_attributed_function(volatile_seed);
    
    /* Test external linkage simulation */
    final_result += test_external_linkage(volatile_seed);
    
    /* Call hidden visibility function */
    hidden_visibility_func();
    
    /* Test memory built-ins */
    test_builtin_memory();
    
    /* Use result to prevent dead code elimination */
    volatile_result += final_result;
    
    /* Additional built-in calls in main */
    if (__builtin_constant_p(argc)) {
        /* Unlikely path */
        volatile_result = 0;
    }
    
    /* Use __builtin_assume_aligned */
    int *ptr = &final_result;
    ptr = (int*)__builtin_assume_aligned(ptr, 4);
    
    /* Final result checksum */
    printf("Result: %d (volatile: %d)\n", final_result, volatile_result);
    
    /* Use __builtin_trap in unreachable path */
    if (final_result < 0) {
        __builtin_trap();
    }
    
    return final_result & 0xFF;
}

/* ==================== ADDITIONAL DECLARATIONS ==================== */

/* Explicit built-in function prototype declaration */
int __builtin_popcount(unsigned int) 
    __attribute__((visibility("hidden"), nothrow));

/* Another built-in with attributes */
long __builtin_expect(long, long) 
    __attribute__((used));

/* Global constructor/destructor with built-ins */
void __attribute__((constructor))
init_func(void)
{
    volatile_seed += __builtin_abs(-42);
}

void __attribute__((destructor))
cleanup_func(void)
{
    volatile_result += __builtin_popcount(0xABCD);
}
