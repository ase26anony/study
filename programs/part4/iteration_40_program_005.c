/* test_builtin_hooks.c - Comprehensive test for built-in function declaration hooks */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void hidden_visibility_func(void) __attribute__((visibility("hidden")));

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;
static volatile int volatile_result = 0;

/* Helper function with multiple attributes */
static int __attribute__((noinline, noclone, used))
test_builtin_arithmetic(volatile int seed)
{
    int result = 0;
    volatile int i;
    
    /* Use math built-ins in a loop with volatile counter */
    for (i = 0; i < (seed & 0xF); i++) {
        int val = seed + i * 37;
        
        /* Multiple arithmetic built-ins */
        result += __builtin_abs(val);
        result += (int)__builtin_sqrtf((float)(val > 0 ? val : -val));
        
        /* Built-in with expectation */
        if (__builtin_expect(val > 100, 0)) {
            result -= 50;
        }
    }
    
    /* Optimization barrier */
    asm volatile("" : : : "memory");
    
    volatile_result = result;
    return result;
}

/* Function with bit operation built-ins */
static int __attribute__((noinline, noclone))
test_builtin_bitops(unsigned int seed)
{
    unsigned int x = seed ^ 0xDEADBEEF;
    int result = 0;
    
    /* Various bit manipulation built-ins */
    result += __builtin_popcount(x);
    result += __builtin_clz(x | 1);  /* Ensure non-zero */
    result += __builtin_ctz(x | 1);  /* Ensure non-zero */
    result += __builtin_ffs(x | 1);  /* Ensure non-zero */
    
    /* Built-in parity */
    result += __builtin_parity(x);
    
    /* Store in volatile to prevent elimination */
    volatile_counter = result;
    
    return result;
}

/* Function with overflow checking built-ins */
static int __attribute__((noinline, noclone, nothrow))
test_builtin_overflow(int a, int b)
{
    int result = 0;
    int overflow;
    
    /* Overflow checking built-ins */
    if (__builtin_add_overflow(a, b, &overflow)) {
        result = -1;
    } else {
        result = overflow;
    }
    
    int mul_result;
    if (__builtin_mul_overflow(a, b, &mul_result)) {
        result += 1000;
    } else {
        result += mul_result;
    }
    
    /* Sub overflow */
    int sub_result;
    if (__builtin_sub_overflow(a, b, &sub_result)) {
        result += 2000;
    }
    
    return result;
}

/* Function with hidden visibility and used attribute */
static void __attribute__((visibility("hidden"), nothrow, used, noinline))
test_attributed_function(int x)
{
    /* Use built-in inside attributed function */
    if (__builtin_expect(x > 100, 1)) {
        volatile_result = __builtin_abs(x);
    } else {
        volatile_result = __builtin_clz(x | 1);
    }
    
    /* Built-in constant detection */
    if (__builtin_constant_p(x)) {
        volatile_counter = 1;
    } else {
        volatile_counter = 0;
    }
}

/* Function that might use unreachable built-in */
static int __attribute__((noinline, noclone))
test_unreachable_builtin(int x)
{
    if (x < 0) {
        /* This should never happen with our inputs */
        __builtin_unreachable();
        return -1;
    }
    
    /* Use built-in for return value optimization */
    return __builtin_abs(x) + __builtin_popcount((unsigned int)x);
}

/* External function definition (simulating another translation unit) */
int external_builtin_user(int x)
{
    /* Use multiple built-ins in external function */
    int result = __builtin_bswap32(x);
    result += __builtin_ffs(x | 1);
    
    /* Built-in classification */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        result += 100;
    }
    
    return result;
}

/* Hidden visibility function definition */
void __attribute__((visibility("hidden")))
hidden_visibility_func(void)
{
    /* Use built-in in hidden visibility function */
    volatile_result = __builtin_clz(0x80000000);
}

/* Main test driver */
int main(int argc, char *argv[])
{
    int final_result = 0;
    volatile int seed;
    
    /* Initialize seed from argc and time */
    seed = argc + (int)time(NULL);
    
    /* Test 1: Arithmetic built-ins */
    final_result += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    final_result += test_builtin_bitops((unsigned int)seed);
    
    /* Test 3: Overflow built-ins */
    final_result += test_builtin_overflow(seed, 12345);
    
    /* Test 4: Call attributed function */
    test_attributed_function(seed);
    final_result += volatile_result;
    
    /* Test 5: Unreachable built-in */
    final_result += test_unreachable_builtin(seed > 0 ? seed : -seed);
    
    /* Test 6: External function with built-ins */
    final_result += external_builtin_user(seed);
    
    /* Test 7: Hidden visibility function */
    hidden_visibility_func();
    final_result += volatile_result;
    
    /* Additional direct built-in usage in main */
    int temp = 0;
    if (__builtin_add_overflow_p(final_result, seed, int)) {
        temp = 10000;
    }
    
    /* Built-in for alignment */
    temp += __builtin_align_up(final_result, 16);
    
    /* Final result computation with optimization barrier */
    asm volatile("" : "+r"(final_result) : : "memory");
    final_result += temp;
    
    /* Use built-in for return */
    if (__builtin_expect(final_result > 0, 1)) {
        printf("Result: %d\n", final_result);
        return final_result & 0xFF;
    }
    
    return 0;
}

/* Additional function with assembly and built-in mix */
static int __attribute__((noinline))
mixed_assembly_builtin(int x)
{
    int result;
    
    /* Inline assembly with memory clobber */
    asm volatile(
        "movl %1, %%eax\n\t"
        "addl $100, %%eax"
        : "=a"(result)
        : "r"(x)
        : "cc"
    );
    
    /* Built-in after assembly */
    result = __builtin_abs(result);
    
    /* Another assembly barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Unused function with built-in, marked used to force emission */
static int __attribute__((used, visibility("hidden")))
unused_but_present(void)
{
    return __builtin_popcount(0xAAAAAAAA) + 
           __builtin_bswap16(0x1234);
}
