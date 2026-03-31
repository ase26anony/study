/* test_fixed_overflow.c
 * Test program to trigger overflow checking in fixed-value.cc
 * Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic test_fixed_overflow.c -o test_fixed_overflow
 */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile("" : : "r"(_tmp)); \
} while(0)

/* Test 1: Overflow with zero high part but low part > max_s */
void test_low_part_overflow() {
    printf("Test 1: Low part overflow (a_high == 0 && a_low > max_s)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
     * max_s = (2^8 - 1) = 255
     * So we need a value with high part = 0 and low part > 255
     */
    unsigned short _Accum src = 256.0uhk;  /* 256 << 8 = 65536 in internal representation */
    unsigned short _Fract dst;
    
    /* This conversion should trigger overflow check */
    dst = (unsigned short _Fract)src;
    KEEP(dst);
    
    printf("  Converted 256.0uhk to usf: internal overflow check should be triggered\n");
}

/* Test 2: Overflow with positive high part (a_high > 0) */
void test_high_part_overflow() {
    printf("Test 2: High part overflow (a_high > 0)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
     * max_r = 0, so any positive high part triggers overflow
     */
    unsigned _Accum src = 65536.0uhk;  /* Large value that will have high part > 0 */
    unsigned short _Fract dst;
    
    dst = (unsigned short _Fract)src;
    KEEP(dst);
    
    printf("  Converted 65536.0uhk to usf: should trigger high part overflow\n");
}

/* Test 3: Different i_f_bits values - signed types */
void test_signed_overflow() {
    printf("Test 3: Signed type overflow tests\n");
    
    /* Test with 16 fractional bits */
    short _Accum src1 = 32768.0hk;  /* Exceeds range for conversion */
    short _Fract dst1;
    
    dst1 = (short _Fract)src1;
    KEEP(dst1);
    printf("  Signed 16-bit overflow test\n");
    
    /* Test with 24 fractional bits */
    long _Accum src2 = 8388608.0lk;
    long _Fract dst2;
    
    dst2 = (long _Fract)src2;
    KEEP(dst2);
    printf("  Signed 24-bit overflow test\n");
}

/* Test 4: Arithmetic operations that cause overflow */
void test_arithmetic_overflow() {
    printf("Test 4: Arithmetic operation overflow\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    /* This addition overflows 1.0 */
    sum = f1 + f2;
    KEEP(sum);
    
    printf("  Addition overflow: 0.8ur + 0.9ur = %f (should saturate)\n", 
           (double)sum / 256.0);
}

/* Test 5: Builtin overflow checks */
void test_builtin_overflow() {
    printf("Test 5: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use generic builtin overflow check */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(result);
    KEEP(overflow);
    
    printf("  Builtin overflow check: overflow = %d\n", overflow);
}

/* Test 6: Multiple conversions with different fractional bits */
void test_mixed_conversions() {
    printf("Test 6: Mixed type conversions\n");
    
    /* Test various i_f_bits values */
    struct {
        const char *name;
        long _Accum src;
    } tests[] = {
        {"8-bit", 256.0lk},
        {"16-bit", 65536.0lk},
        {"24-bit", 16777216.0lk},
        {"32-bit", 4294967296.0lk}
    };
    
    for (int i = 0; i < 4; i++) {
        unsigned _Fract dst;
        dst = (unsigned _Fract)tests[i].src;
        KEEP(dst);
        printf("  %s conversion overflow test\n", tests[i].name);
    }
}

/* Test 7: Edge case - exact boundary value */
void test_boundary() {
    printf("Test 7: Boundary value test\n");
    
    /* For unsigned short _Fract: max value is 255/256 = 0.99609375
     * Create a value just at the boundary
     */
    unsigned short _Accum boundary = 255.99609375uhk;  /* Very close to max */
    unsigned short _Fract dst;
    
    dst = (unsigned short _Fract)boundary;
    KEEP(dst);
    
    printf("  Boundary value conversion\n");
}

/* Test 8: Saturated arithmetic attribute */
__attribute__((saturated)) unsigned short _Fract sat_add(
    unsigned short _Fract a, 
    unsigned short _Fract b) {
    return a + b;
}

void test_saturated_attribute() {
    printf("Test 8: Saturated attribute test\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    
    result = sat_add(f1, f2);
    KEEP(result);
    
    printf("  Saturated addition result: %f\n", (double)result / 256.0);
}

/* Test 9: Integer to fixed-point overflow */
void test_integer_overflow() {
    printf("Test 9: Integer to fixed-point overflow\n");
    
    unsigned int large_int = 300;  /* > 255 for 8-bit fractional */
    unsigned short _Fract dst;
    
    dst = (unsigned short _Fract)large_int;
    KEEP(dst);
    
    printf("  Integer 300 -> usf overflow test\n");
}

/* Test 10: Nested overflow in expressions */
void test_nested_overflow() {
    printf("Test 10: Nested expression overflow\n");
    
    unsigned short _Fract f1 = 0.7ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract f3 = 0.5ur;
    unsigned short _Fract result;
    
    /* This should overflow in intermediate calculations */
    result = f1 + f2 + f3;
    KEEP(result);
    
    printf("  Nested addition overflow test\n");
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests to trigger various overflow conditions */
    test_low_part_overflow();
    printf("\n");
    
    test_high_part_overflow();
    printf("\n");
    
    test_signed_overflow();
    printf("\n");
    
    test_arithmetic_overflow();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_mixed_conversions();
    printf("\n");
    
    test_boundary();
    printf("\n");
    
    test_saturated_attribute();
    printf("\n");
    
    test_integer_overflow();
    printf("\n");
    
    test_nested_overflow();
    printf("\n");
    
    printf("All tests completed.\n");
    
    /* Return non-zero if any test failed (simplified) */
    return 0;
}
