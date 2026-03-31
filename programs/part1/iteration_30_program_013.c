/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c -o test */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow with zero high part but low part > max_s */
void test_overflow_low_part(void) {
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* We need a_high = 0 and a_low > 255 */
    
    /* Create a value that when converted to unsigned short _Fract 
       has low part > 255 but high part = 0 */
    unsigned short _Accum a = 1.0uhk;  /* 1.0 in 8.8 format = 256 */
    unsigned short _Fract b;
    
    /* This conversion should trigger overflow check */
    b = (unsigned short _Fract)a;
    
    dummy = (int)b;
    printf("test_overflow_low_part: b = %u (0x%x)\n", 
           (unsigned)(b * 256), (unsigned)(b * 256));
}

/* Test 2: Overflow with positive high part (a_high > 0) */
void test_overflow_high_part(void) {
    /* Need a_high > 0 */
    /* Use a larger accumulator type */
    unsigned long _Accum a = 256.0ULK;  /* Large value that will have high part > 0 */
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;
    
    dummy = (int)b;
    printf("test_overflow_high_part: b = %u (0x%x)\n",
           (unsigned)(b * 256), (unsigned)(b * 256));
}

/* Test 3: Different i_f_bits values (16 bits) */
void test_i_f_bits_16(void) {
    /* For unsigned _Fract: i_f_bits = 16 */
    unsigned _Accum a = 65536.0UK;  /* 65536 in 16.16 format */
    unsigned _Fract b;
    
    b = (unsigned _Fract)a;
    
    dummy = (int)b;
    printf("test_i_f_bits_16: b = %u (0x%x)\n",
           (unsigned)(b * 65536), (unsigned)(b * 65536));
}

/* Test 4: Signed types with negative overflow */
void test_signed_overflow(void) {
    /* For signed short _Fract: i_f_bits = 7 (one sign bit) */
    short _Accum a = 128.0hk;  /* Max positive is 127.996 in 8.7 format */
    short _Fract b;
    
    b = (short _Fract)a;
    
    dummy = (int)b;
    printf("test_signed_overflow: b = %d (0x%x)\n",
           (int)(b * 128), (int)(b * 128));
}

/* Test 5: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    /* 0.8 + 0.8 = 1.6 > 1.0, should overflow */
    sum = f1 + f2;
    
    dummy = (int)sum;
    printf("test_arithmetic_overflow: sum = %u (0x%x)\n",
           (unsigned)(sum * 256), (unsigned)(sum * 256));
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin should trigger the overflow checking code */
    overflow = __builtin_mul_overflow((int)(f1 * 256), (int)(f2 * 256), 
                                      (int*)(&result));
    
    dummy = overflow + (int)result;
    printf("test_builtin_overflow: overflow=%d, result=%u\n",
           overflow, (unsigned)(result * 256));
}

/* Test 7: Multiple conversions with different fractional bits */
void test_multiple_conversions(void) {
    /* Test various i_f_bits values */
    struct {
        const char *name;
        unsigned long long value;
        int shift;
    } tests[] = {
        {"8-bit", 300, 8},    /* > 255 */
        {"12-bit", 5000, 12}, /* > 4095 */
        {"16-bit", 70000, 16}, /* > 65535 */
        {"20-bit", 1500000, 20}, /* > 1048575 */
        {"24-bit", 20000000, 24}, /* > 16777215 */
    };
    
    for (int i = 0; i < 5; i++) {
        /* Create values that will overflow for each i_f_bits */
        unsigned long long val = tests[i].value;
        unsigned _Accum a = (unsigned _Accum)val;
        unsigned _Fract b;
        
        /* Force conversion - the exact i_f_bits depends on target type */
        b = (unsigned _Fract)a;
        
        dummy += (int)b;
        printf("%s: input=%llu, output=%u\n",
               tests[i].name, val, (unsigned)(b * (1 << tests[i].shift)));
    }
}

/* Test 8: Edge case - exactly at boundary */
void test_boundary_case(void) {
    /* Test value exactly equal to max_s (should not trigger overflow) */
    unsigned short _Accum a = 255.0uhk / 256.0uhk;  /* 255/256 = 0.996 */
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;
    
    dummy = (int)b;
    printf("test_boundary_case: b = %u (exactly at boundary)\n",
           (unsigned)(b * 256));
}

/* Test 9: Saturated arithmetic */
void test_saturated_arithmetic(void) {
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    sum = f1 + f2;  /* Should saturate to 1.0 */
    
    dummy = (int)sum;
    printf("test_saturated_arithmetic: 0.9 + 0.9 = %u (0x%x)\n",
           (unsigned)(sum * 256), (unsigned)(sum * 256));
}

/* Test 10: Mixed type conversions */
void test_mixed_conversions(void) {
    /* Convert between different fixed-point types */
    long _Accum la = 1000.0lk;
    short _Accum sa;
    unsigned short _Fract uf;
    
    sa = (short _Accum)la;  /* May overflow */
    uf = (unsigned short _Fract)sa;  /* May overflow again */
    
    dummy = (int)sa + (int)uf;
    printf("test_mixed_conversions: la=1000.0 -> sa=%d, uf=%u\n",
           (int)sa, (unsigned)(uf * 256));
}

int main(void) {
    int result = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_overflow_low_part();
    test_overflow_high_part();
    test_i_f_bits_16();
    test_signed_overflow();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_multiple_conversions();
    test_boundary_case();
    test_saturated_arithmetic();
    test_mixed_conversions();
    
    printf("\nAll tests completed.\n");
    
    /* Use dummy to prevent dead code elimination */
    result = dummy;
    
    return result != 0 ? 0 : 0;  /* Always return 0 for successful test run */
}
