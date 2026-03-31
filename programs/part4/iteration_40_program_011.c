/* test_targhooks.c - Comprehensive built-in function test for targhooks.cc coverage */

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/* External function declarations to simulate multi-file scope */
extern int external_builtin_user(int x);
extern void external_visibility_func(void) __attribute__((visibility("hidden")));

/* Prevent optimization of helper functions */
#define NO_OPT __attribute__((noinline, noclone))

/* ========== Pattern 1: Basic built-in usage with volatile barriers ========== */

NO_OPT static int test_builtin_arithmetic(volatile int seed)
{
    volatile int result = 0;
    
    /* Use various arithmetic built-ins */
    for (volatile int i = 0; i < 5; i++) {
        int val = seed + i * 7;
        
        /* __builtin_abs - commonly used built-in */
        result += __builtin_abs(val);
        
        /* __builtin_sqrtf with float conversion */
        float fval = (float)val;
        result += (int)__builtin_sqrtf(fval < 0 ? -fval : fval);
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* ========== Pattern 2: Bit operation built-ins ========== */

NO_OPT static int test_builtin_bitops(unsigned int seed)
{
    volatile unsigned int accum = 0;
    
    /* __builtin_popcount - population count */
    accum += __builtin_popcount(seed);
    
    /* __builtin_clz - count leading zeros */
    if (seed != 0) {
        accum += __builtin_clz(seed);
    }
    
    /* __builtin_ctz - count trailing zeros */
    if (seed != 0) {
        accum += __builtin_ctz(seed);
    }
    
    /* __builtin_ffs - find first set bit */
    accum += __builtin_ffs(seed);
    
    /* __builtin_parity - parity of number of 1-bits */
    accum += __builtin_parity(seed);
    
    return (int)accum;
}

/* ========== Pattern 3: Overflow checking built-ins ========== */

NO_OPT static int test_builtin_overflow(int a, int b)
{
    volatile int result = 0;
    int overflow_result;
    
    /* __builtin_add_overflow */
    if (__builtin_add_overflow(a, b, &overflow_result)) {
        result += 1;
    } else {
        result += overflow_result;
    }
    
    /* __builtin_sub_overflow */
    if (__builtin_sub_overflow(a, b, &overflow_result)) {
        result += 2;
    } else {
        result += overflow_result;
    }
    
    /* __builtin_mul_overflow */
    if (__builtin_mul_overflow(a, b, &overflow_result)) {
        result += 3;
    } else {
        result += overflow_result;
    }
    
    return result;
}

/* ========== Pattern 4: Function with explicit attributes ========== */

/* Function with ALL the attributes that might interact with the hook */
static void __attribute__((visibility("hidden"), nothrow, used, artificial))
attributed_builtin_user(int x) 
{
    /* Use __builtin_expect to guide branch prediction */
    if (__builtin_expect(x > 100, 0)) {
        volatile int y = __builtin_abs(x);
        (void)y;
    }
    
    /* Use __builtin_unreachable in dead code path */
    if (x < 0) {
        __builtin_unreachable();
    }
}

NO_OPT static int test_attributed_function(int seed)
{
    /* Call the heavily attributed function */
    attributed_builtin_user(seed);
    
    /* Also declare and use built-in directly with attributes */
    int (*fn_ptr)(int) __attribute__((nothrow)) = &__builtin_abs;
    return fn_ptr(seed);
}

/* ========== Pattern 5: External linkage simulation ========== */

/* External function defined later in same file */
extern int external_builtin_helper(int x);

/* Function that will be called externally */
NO_OPT int external_builtin_user(int x)
{
    /* Use __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return x * 2;
    }
    
    /* Use __builtin_unreachable in conditional */
    if (x < INT_MIN) {
        __builtin_unreachable();
    }
    
    return external_builtin_helper(x);
}

/* The actual implementation */
NO_OPT static int external_builtin_helper(int x)
{
    /* Use multiple built-ins */
    int a = __builtin_abs(x);
    int b = __builtin_popcount((unsigned int)a);
    
    /* __builtin_expect with loop */
    for (int i = 0; __builtin_expect(i < 3, 1); i++) {
        a += __builtin_clz((unsigned int)(b + i));
    }
    
    return a + b;
}

/* ========== Pattern 6: Visibility-specific function ========== */

/* Function with explicit hidden visibility */
void __attribute__((visibility("hidden"), used))
external_visibility_func(void)
{
    /* Use __builtin_trap under condition */
    volatile int x = 42;
    if (x == 0) {
        __builtin_trap();
    }
    
    /* Use __builtin_prefetch */
    int array[10];
    __builtin_prefetch(&array[0], 0, 0);
}

/* ========== Pattern 7: Complex expression with built-ins ========== */

NO_OPT static int test_complex_expressions(int seed)
{
    volatile int result = 0;
    
    /* Chain built-ins together */
    int val = __builtin_abs(seed);
    val = __builtin_popcount((unsigned int)val) + 
          __builtin_ctz((unsigned int)(val | 1));
    
    /* Use __builtin_choose_expr */
    result += __builtin_choose_expr(
        sizeof(int) == 4,
        __builtin_bswap32((unsigned int)val),
        val
    );
    
    /* Use __builtin_offsetof with a struct */
    struct TestStruct {
        int a;
        char b;
        long c;
    };
    result += __builtin_offsetof(struct TestStruct, c);
    
    return result;
}

/* ========== Pattern 8: Built-ins with assembly barriers ========== */

NO_OPT static int test_asm_barrier_builtins(int seed)
{
    volatile int result = seed;
    
    /* Memory barrier before built-in */
    asm volatile("" : : : "memory");
    
    result = __builtin_abs(result);
    
    /* Memory barrier after built-in */
    asm volatile("" : : : "memory");
    
    /* Use __builtin_expect with barrier */
    if (__builtin_expect(result > 1000, 0)) {
        asm volatile("" : : : "memory");
        result = __builtin_clz((unsigned int)result);
    }
    
    return result;
}

/* ========== Main function tying everything together ========== */

int main(int argc, char *argv[])
{
    /* Use argc as volatile seed to prevent constant folding */
    volatile int seed = argc;
    volatile unsigned int useed = (unsigned int)argc;
    
    int checksum = 0;
    
    /* Test 1: Arithmetic built-ins */
    checksum += test_builtin_arithmetic(seed);
    
    /* Test 2: Bit operation built-ins */
    checksum += test_builtin_bitops(useed);
    
    /* Test 3: Overflow checking built-ins */
    checksum += test_builtin_overflow(seed, 12345);
    
    /* Test 4: Attributed function */
    checksum += test_attributed_function(seed);
    
    /* Test 5: External linkage */
    checksum += external_builtin_user(seed);
    
    /* Test 6: Visibility function call */
    external_visibility_func();
    
    /* Test 7: Complex expressions */
    checksum += test_complex_expressions(seed);
    
    /* Test 8: Assembly barrier built-ins */
    checksum += test_asm_barrier_builtins(checksum);
    
    /* Prevent dead code elimination of checksum */
    volatile int final_result = checksum;
    
    /* Use __builtin_printf if available, else regular printf */
    #ifdef __builtin_printf
    __builtin_printf("Result: %d\n", final_result);
    #else
    printf("Result: %d\n", final_result);
    #endif
    
    /* Use __builtin_return_address */
    void *ra = __builtin_return_address(0);
    (void)ra;
    
    return final_result & 0xFF;  /* Return non-zero result */
}
