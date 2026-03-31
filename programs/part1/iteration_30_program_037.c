/* test_fixed_overflow.c
 * 
 * This test triggers overflow checks in GCC's fixed-value.cc
 * Specifically targets lines 264-277 which check if a value exceeds
 * maximum bounds during fixed-point conversion.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimizations from eliminating overflow checks */
volatile int global_counter = 0;

/* Test function for different i_f_bits values */
void test_overflow_conditions(void) {
    printf("Testing fixed-point overflow conditions...\n");
    
    /* Test 1: Unsigned fract with 8 fractional bits (i_f_bits=8) */
    {
        /* For unsigned short _Fract: i_f_bits = 8, max_s = 2^8 - 1 = 255 */
        /* Condition: a_high == 0 && a_low > 255 */
        unsigned short _Fract f1 = 0.5ur;
        unsigned short _Fract f2 = 0.6ur;
        
        /* This addition might overflow if not saturated */
        unsigned short _Fract sum = f1 + f2;
        
        /* Force conversion that could trigger overflow check */
        unsigned short _Accum a1 = 256.0hk;  /* 256 in 8.8 format */
        unsigned short _Fract f3 = (unsigned short _Fract)a1;  /* Should overflow */
        
        global_counter += (int)(sum * 256);
        global_counter += (int)(f3 * 256);
    }
    
    /* Test 2: Signed fract with 7 fractional bits (i_f_bits=7 for signed) */
    {
        /* For signed short _Fract: i_f_bits = 7 (one sign bit) */
        /* max_s = 2^7 - 1 = 127 when zero-extended */
        short _Fract f1 = 0.9hr;
        short _Fract f2 = 0.8hr;
        
        /* Overflow in positive direction */
        short _Fract sum = f1 + f2;
        
        /* Explicit overflow by casting from larger type */
        short _Accum a1 = 128.0hk;  /* Exceeds max for 1.7 format */
        short _Fract f3 = (short _Fract)a1;
        
        global_counter += (int)(sum * 128);
        global_counter += (int)(f3 * 128);
    }
    
    /* Test 3: Unsigned accum with 16 fractional bits (i_f_bits=16) */
    {
        /* For unsigned _Accum: i_f_bits = 16 */
        /* max_s = 2^16 - 1 = 65535 */
        unsigned _Accum a1 = 65535.9999uk;  /* Very close to max */
        unsigned _Accum a2 = 0.0001uk;
        
        /* This should approach or exceed the bound */
        unsigned _Accum sum = a1 + a2;
        
        /* Cast to smaller type to force overflow check */
        unsigned short _Accum a3 = (unsigned short _Accum)sum;
        
        global_counter += (int)(sum * 65536);
        global_counter += (int)(a3 * 65536);
    }
    
    /* Test 4: Test with i_f_bits = 1 (minimum) */
    {
        /* Create a type with 1 fractional bit */
        typedef _Fract __attribute__((mode(HQ))) one_bit_fract;
        
        one_bit_fract f1 = 0.5r;
        one_bit_fract f2 = 0.5r;
        
        /* 0.5 + 0.5 = 1.0, which might overflow depending on representation */
        one_bit_fract sum = f1 + f2;
        
        global_counter += (int)(sum * 2);
    }
    
    /* Test 5: Mixed signedness conversions */
    {
        /* Signed to unsigned conversion that should overflow */
        short _Accum sa = 255.5hk;      /* Positive value */
        unsigned short _Accum usa = (unsigned short _Accum)sa;
        
        /* Unsigned to signed with high value */
        unsigned _Accum ua = 32767.9999uk;
        _Accum sa2 = (_Accum)ua;
        
        global_counter += (int)(usa * 256);
        global_counter += (int)(sa2 * 32768);
    }
    
    /* Test 6: Using builtins for overflow detection */
    {
        unsigned short _Fract f1 = 0.9ur;
        unsigned short _Fract f2 = 0.9ur;
        unsigned short _Fract result;
        
        /* This builtin might trigger the overflow checking code */
        int overflow = __builtin_add_overflow(f1, f2, &result);
        
        if (overflow) {
            global_counter += 1000;
        }
        
        /* Also test multiplication overflow */
        unsigned short _Accum a1 = 200.0hk;
        unsigned short _Accum a2 = 2.0hk;
        unsigned short _Accum mul_result;
        
        overflow = __builtin_mul_overflow(a1, a2, &mul_result);
        if (overflow) {
            global_counter += 2000;
        }
    }
    
    /* Test 7: Explicit boundary testing */
    {
        /* Test the exact condition: a_high == 0 && a_low > max_s */
        /* For 8 fractional bits, max_s = 255 */
        /* Create a value with high=0, low=256 */
        unsigned short _Accum boundary = 256.0hk;  /* 256 = 1 << 8 */
        unsigned short _Fract f = (unsigned short _Fract)boundary;
        
        /* Also test a_high > 0 case */
        unsigned _Accum large = 65536.0uk;  /* High part would be > 0 */
        unsigned short _Accum converted = (unsigned short _Accum)large;
        
        global_counter += (int)(f * 256);
        global_counter += (int)(converted * 256);
    }
    
    /* Test 8: Saturated arithmetic */
    {
        /* With saturation attribute, overflow checks are definitely performed */
        unsigned short _Fract __attribute__((saturated)) f1 = 0.9ur;
        unsigned short _Fract __attribute__((saturated)) f2 = 0.9ur;
        unsigned short _Fract __attribute__((saturated)) sum = f1 + f2;
        
        /* This should saturate to maximum value */
        global_counter += (int)(sum * 256);
    }
}

/* Additional test with volatile to prevent optimization */
void test_volatile_conversions(void) {
    volatile unsigned short _Accum v1 = 300.0hk;
    volatile unsigned short _Fract v2;
    
    /* This conversion should trigger overflow check */
    v2 = (unsigned short _Fract)v1;
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(v2));
    
    global_counter += 1;
}

/* Test with different optimization barriers */
int test_with_barrier(int x) {
    unsigned short _Accum a = (unsigned short _Accum)x;
    unsigned short _Fract f;
    
    /* Force conversion through function call */
    f = (unsigned short _Fract)a;
    
    /* Compiler can't optimize away due to side effect */
    return (int)(f * 256);
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_overflow_conditions();
        test_volatile_conversions();
        
        /* Test with different input values */
        int result = test_with_barrier(256 + i * 100);
        global_counter += result;
    }
    
    printf("Tests completed. Global counter: %d\n", global_counter);
    printf("If any conversions overflowed, the internal check in fixed-value.cc\n");
    printf("should have been triggered (lines 264-277).\n");
    
    return 0;
}
