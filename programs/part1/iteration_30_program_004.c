/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int dummy;

/* Test 1: Overflow in low part with zero high part */
void test_low_part_overflow(void) {
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* We need a_high = 0 and a_low > 255 */
    
    /* Create a value that when converted to unsigned short _Fract 
       has high part = 0 and low part > 255 */
    unsigned short _Accum a = 1.0uhk;  /* 1.0 in 8.8 format = 256 */
    unsigned short _Fract b;
    
    /* This conversion should trigger overflow check */
    b = (unsigned short _Fract)a;
    
    dummy = (unsigned int)b;
    printf("test_low_part_overflow: b = %u/256\n", (unsigned int)(b * 256));
}

/* Test 2: Positive high part case */
void test_positive_high_part(void) {
    /* We need a_high > 0 */
    /* Use a larger accumulator type */
    unsigned long _Accum a = 256.0ULK;  /* 256 in format with more integer bits */
    unsigned short _Fract b;
    
    /* This should have a_high > 0 when converted */
    b = (unsigned short _Fract)a;
    
    dummy = (unsigned int)b;
    printf("test_positive_high_part: b = %u/256\n", (unsigned int)(b * 256));
}

/* Test 3: Different i_f_bits values */
void test_various_fractional_bits(void) {
    /* Test with different fixed-point types to hit different i_f_bits */
    
    /* i_f_bits = 16 (unsigned _Fract) */
    unsigned _Accum a1 = 65536.0UK;  /* 65536 */
    unsigned _Fract b1;
    b1 = (unsigned _Fract)a1;
    printf("test_16bit: b1 = %u/65536\n", (unsigned int)(b1 * 65536));
    
    /* i_f_bits = 24 (unsigned long _Fract) */
    unsigned long _Accum a2 = 16777216.0ULK;  /* 2^24 */
    unsigned long _Fract b2;
    b2 = (unsigned long _Fract)a2;
    printf("test_24bit: a2 = %lu\n", (unsigned long)(a2));
    
    dummy = (unsigned int)b1 + (unsigned int)b2;
}

/* Test 4: Signed types */
void test_signed_overflow(void) {
    /* Signed overflow checking might use similar logic */
    short _Accum a = 128.0hk;  /* 128 in signed 8.8 format */
    short _Fract b;
    
    b = (short _Fract)a;  /* Should overflow for signed 1.7 format */
    
    dummy = (unsigned int)b;
    printf("test_signed: b = %d/128\n", (int)(b * 128));
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin might trigger the overflow checking code */
    overflow = __builtin_mul_overflow(f1, f2, &result);
    
    printf("test_builtin: overflow = %d, result = %u/256\n", 
           overflow, (unsigned int)(result * 256));
    dummy = overflow;
}

/* Test 6: Arithmetic operations that overflow */
void test_arithmetic_overflow(void) {
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    /* Addition that exceeds 1.0 */
    sum = f1 + f2;
    
    printf("test_arithmetic: sum = %u/256\n", (unsigned int)(sum * 256));
    dummy = (unsigned int)sum;
}

/* Test 7: Explicit casts with constants */
void test_constant_casts(void) {
    /* Direct constant conversions that should overflow */
    unsigned short _Fract b1 = (unsigned short _Fract)1.5;
    unsigned short _Fract b2 = (unsigned short _Fract)256;
    unsigned short _Fract b3 = (unsigned short _Fract)-1;  /* For unsigned, this wraps */
    
    printf("test_constants: b1=%u/256, b2=%u/256, b3=%u/256\n",
           (unsigned int)(b1 * 256),
           (unsigned int)(b2 * 256),
           (unsigned int)(b3 * 256));
    
    dummy = (unsigned int)b1 + (unsigned int)b2 + (unsigned int)b3;
}

/* Test 8: Saturation mode */
#pragma GCC push_options
#pragma GCC optimize ("-fsaturated-arithmetic")
void test_saturation(void) {
    /* With saturation, overflow should saturate to max value */
    unsigned short _Accum a = 2.0uhk;  /* 512 in 8.8 format */
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;  /* Should saturate to 1.0 */
    
    printf("test_saturation: b = %u/256 (should be 255/256)\n", 
           (unsigned int)(b * 256));
    dummy = (unsigned int)b;
}
#pragma GCC pop_options

/* Test 9: Nested conversions */
void test_nested_conversions(void) {
    /* Multiple conversions to trigger different code paths */
    unsigned long _Accum a1 = 1000000.0ULK;
    unsigned _Accum a2 = (unsigned _Accum)a1;
    unsigned short _Accum a3 = (unsigned short _Accum)a2;
    unsigned short _Fract b = (unsigned short _Fract)a3;
    
    printf("test_nested: b = %u/256\n", (unsigned int)(b * 256));
    dummy = (unsigned int)b;
}

/* Test 10: Using asm to force value usage */
void test_asm_volatile(void) {
    unsigned short _Accum a = 300.0uhk;  /* 300 > 255 */
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;
    
    /* Force the compiler to actually compute b */
    asm volatile("" : : "r" (b));
    
    printf("test_asm: a = %u, b = %u/256\n", 
           (unsigned int)a, (unsigned int)(b * 256));
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n");
    
    test_low_part_overflow();      /* Should trigger: a_high == 0 && a_low > max_s */
    test_positive_high_part();     /* Should trigger: a_high > 0 */
    test_various_fractional_bits();/* Different i_f_bits values */
    test_signed_overflow();        /* Signed version */
    test_builtin_overflow();       /* Using builtins */
    test_arithmetic_overflow();    /* Arithmetic operations */
    test_constant_casts();         /* Constant conversions */
    test_saturation();             /* With saturation enabled */
    test_nested_conversions();     /* Nested conversions */
    test_asm_volatile();           /* Using asm volatile */
    
    printf("\nAll tests completed.\n");
    printf("Check coverage of fixed-value.cc lines 264-277\n");
    
    return 0;
}
