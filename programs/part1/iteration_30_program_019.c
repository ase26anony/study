/* Test program to trigger uncovered lines in fixed-value.cc
   Lines 264-277: overflow checking for fixed-point conversions */

#include <stdio.h>

/* Use volatile to prevent constant folding optimizations */
volatile double source_values[] = {
    1.0,           /* Will overflow unsigned fract types */
    256.0,         /* Will overflow 8-bit fractional types */
    65536.0,       /* Will overflow 16-bit fractional types */
    4294967296.0,  /* Will overflow 32-bit fractional types */
    -1.0,          /* Test signed overflow cases */
    -256.0
};

/* Test various fixed-point types with different i_f_bits values */
void test_unsigned_fract_types(void) {
    printf("Testing unsigned fract types...\n");
    
    /* Test 1: i_f_bits = 8 (unsigned short _Fract) */
    {
        unsigned short _Fract usf;
        unsigned short _Accum usa = 256.0uhk;  /* 256.0 in unsigned short accum */
        
        /* This conversion should trigger overflow check:
           a_high = 0, a_low = 256, max_s = 255 */
        usf = (unsigned short _Fract)usa;
        printf("  ushort _Fract from 256.0uhk: %f\n", (double)usf);
    }
    
    /* Test 2: i_f_bits = 16 (unsigned _Fract) */
    {
        unsigned _Fract uf;
        unsigned _Accum ua = 65536.0uk;  /* 65536.0 in unsigned accum */
        
        /* This should trigger: a_high = 0, a_low = 65536, max_s = 65535 */
        uf = (unsigned _Fract)ua;
        printf("  unsigned _Fract from 65536.0uk: %f\n", (double)uf);
    }
    
    /* Test 3: i_f_bits = 32 (unsigned long _Fract) */
    {
        unsigned long _Fract ulf;
        unsigned long _Accum ula = 4294967296.0ulk;  /* 2^32 */
        
        /* This should trigger: a_high = 0, a_low = 2^32, max_s = 2^32-1 */
        ulf = (unsigned long _Fract)ula;
        printf("  ulong _Fract from 2^32ulk: %f\n", (double)ulf);
    }
}

void test_signed_fract_types(void) {
    printf("Testing signed fract types...\n");
    
    /* Test 1: i_f_bits = 8 (signed short _Fract) */
    {
        signed short _Fract ssf;
        signed short _Accum ssa = 128.0hk;  /* 128.0 in signed short accum */
        
        /* This should trigger positive overflow */
        ssf = (signed short _Fract)ssa;
        printf("  short _Fract from 128.0hk: %f\n", (double)ssf);
        
        /* Test negative overflow */
        ssa = -129.0hk;
        ssf = (signed short _Fract)ssa;
        printf("  short _Fract from -129.0hk: %f\n", (double)ssf);
    }
}

void test_accum_types(void) {
    printf("Testing accum types...\n");
    
    /* Test conversion between accum types with different fractional bits */
    {
        long _Accum la = 1000000.0lk;
        short _Accum sa;
        
        /* This may trigger overflow depending on implementation */
        sa = (short _Accum)la;
        printf("  short _Accum from 1000000.0lk: %f\n", (double)sa);
    }
}

void test_overflow_operations(void) {
    printf("Testing overflow in operations...\n");
    
    /* Test addition that overflows */
    {
        unsigned short _Fract f1 = 0.8uhr;
        unsigned short _Fract f2 = 0.5uhr;
        unsigned short _Fract sum;
        
        /* 0.8 + 0.5 = 1.3 which exceeds 1.0 for unsigned fract */
        sum = f1 + f2;
        printf("  0.8uhr + 0.5uhr = %f\n", (double)sum);
    }
    
    /* Test multiplication that overflows */
    {
        unsigned _Fract f1 = 0.9ur;
        unsigned _Fract f2 = 0.9ur;
        unsigned _Fract product;
        
        product = f1 * f2;
        printf("  0.9ur * 0.9ur = %f\n", (double)product);
    }
}

void test_builtin_overflow(void) {
    printf("Testing builtin overflow checks...\n");
    
    /* Use __builtin_add_overflow with fixed-point types */
    {
        unsigned short _Fract f1 = 0.8uhr;
        unsigned short _Fract f2 = 0.5uhr;
        unsigned short _Fract result;
        int overflow;
        
        overflow = __builtin_add_overflow(f1, f2, &result);
        printf("  __builtin_add_overflow(0.8uhr, 0.5uhr): overflow=%d, result=%f\n", 
               overflow, (double)result);
    }
    
    /* Test __builtin_mul_overflow */
    {
        unsigned _Fract f1 = 0.9ur;
        unsigned _Fract f2 = 0.9ur;
        unsigned _Fract result;
        int overflow;
        
        overflow = __builtin_mul_overflow(f1, f2, &result);
        printf("  __builtin_mul_overflow(0.9ur, 0.9ur): overflow=%d, result=%f\n",
               overflow, (double)result);
    }
}

void test_saturated_arithmetic(void) {
    printf("Testing saturated arithmetic...\n");
    
    /* Test with -fsaturated-arithmetic */
    {
        unsigned short _Fract f1 = 0.8uhr;
        unsigned short _Fract f2 = 0.5uhr;
        unsigned short _Fract sum;
        
        sum = f1 + f2;
        printf("  Saturated add: 0.8uhr + 0.5uhr = %f\n", (double)sum);
    }
}

/* Main test driver */
int main(void) {
    int test_result = 0;
    
    printf("=== Testing fixed-point overflow conditions ===\n\n");
    
    /* Run all test functions */
    test_unsigned_fract_types();
    printf("\n");
    
    test_signed_fract_types();
    printf("\n");
    
    test_accum_types();
    printf("\n");
    
    test_overflow_operations();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_saturated_arithmetic();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    return test_result;
}
