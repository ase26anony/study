/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed_test.c -o fixed_test */
/* Additional flags to test: -O2 -fno-math-errno -fno-trapping-math */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -1.0lk;  /* MIN for signed long _Accum (Q15.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_max(void) {
    return 0.9999847412109375lk; /* MAX for signed long _Accum (Q15.16) */
}

/* Force materialization of intermediate results */
volatile signed short _Fract volatile_result;

int main(void) {
    int checksum = 0;
    
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 MAX */
    const signed short _Fract sf_min = -1.0r;               /* Q0.15 MIN */
    
    /* These will trigger range checking during conversion */
    const signed long _Accum la_from_sf_max = (signed long _Accum)sf_max;
    const signed long _Accum la_from_sf_min = (signed long _Accum)sf_min;
    
    /* Complex constant expression that may overflow when converted */
    const signed _Accum ca = (signed _Accum)0.5r * 3.0r;  /* 1.5 in Q15.16 */
    const signed short _Fract sf_from_ca = (signed short _Fract)ca;
    
    /* Test 2: Values just beyond boundaries */
    /* Multiply to get value just above MAX for short _Fract */
    const signed short _Fract sf1 = 0.9r;
    const signed short _Fract sf2 = 0.9r;
    const signed _Accum accum_product = (signed _Accum)sf1 * (signed _Accum)sf2;
    /* This conversion should trigger a_high.sgt(max_r) check */
    const signed short _Fract sf_product = (signed short _Fract)accum_product;
    
    /* Test 3: Using volatile to prevent optimization */
    volatile signed _Accum volatile_accum = 0.9999ak;
    volatile signed short _Fract volatile_sf = (signed short _Fract)volatile_accum;
    volatile_result = volatile_sf;
    
    /* Test 4: Mixed precision with saturation */
    const signed _Sat short _Fract sat_sf1 = 0.999969482421875r;  /* At MAX */
    const signed _Sat short _Fract sat_sf2 = 0.0001r;
    /* This addition would overflow without saturation */
    const signed _Sat short _Fract sat_sum = sat_sf1 + sat_sf2;
    
    /* Convert saturated result to different type - may trigger different path */
    const signed _Accum accum_from_sat = (signed _Accum)sat_sum;
    
    /* Test 5: Unsigned types with boundary checks */
    const unsigned short _Fract uf_max = 0.999969482421875ur;
    const unsigned _Accum ua_large = (unsigned _Accum)1.5ur;
    
    /* This conversion should check a_low.ugt(max_s) */
    const unsigned short _Fract uf_from_ua = (unsigned short _Fract)ua_large;
    
    /* Test 6: Negative to unsigned conversion */
    const signed short _Fract negative_sf = -0.5r;
    /* This should trigger range checking (negative to unsigned) */
    const unsigned short _Fract uf_from_negative = (unsigned short _Fract)negative_sf;
    
    /* Test 7: Loop with small iteration count to create semi-constant values */
    signed _Accum loop_accum = 0.0ak;
    for (int i = 0; i < 3; i++) {
        loop_accum += 0.33333333333333333333ak;  /* 1/3 in Q15.16 */
    }
    /* Conversion after loop - value is known but control flow was present */
    const signed short _Fract sf_from_loop = (signed short _Fract)loop_accum;
    
    /* Test 8: Using function calls to get boundary values */
    signed short _Fract dynamic_sf_max = get_sfract_max();
    signed short _Fract sf_plus_epsilon = dynamic_sf_max + 0.00001r;
    /* This conversion may overflow */
    signed _Accum accum_from_overflow = (signed _Accum)sf_plus_epsilon;
    
    /* Test 9: Minimum value conversions */
    signed long _Accum dynamic_accum_min = get_saccum_min();
    /* Convert minimum long _Accum to short _Fract - should be in range */
    signed short _Fract sf_from_min = (signed short _Fract)dynamic_accum_min;
    
    /* Test 10: Maximum long _Accum to short _Fract */
    signed long _Accum dynamic_accum_max = get_saccum_max();
    signed long _Accum slightly_above_max = dynamic_accum_max + 0.0000001lk;
    /* This should trigger overflow check */
    signed short _Fract sf_from_above_max = (signed short _Fract)slightly_above_max;
    
    /* Create observable output to prevent dead code elimination */
    checksum += (int)(sf_from_ca * 1000r);
    checksum += (int)(sf_product * 1000r);
    checksum += (int)(sat_sum * 1000r);
    checksum += (int)(accum_from_sat * 1000ak);
    checksum += (int)(uf_from_ua * 1000ur);
    checksum += (int)(uf_from_negative * 1000ur);
    checksum += (int)(sf_from_loop * 1000r);
    checksum += (int)(accum_from_overflow * 1000ak);
    checksum += (int)(sf_from_min * 1000r);
    checksum += (int)(sf_from_above_max * 1000r);
    
    /* Use volatile result */
    checksum += (int)(volatile_result * 1000r);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional tests with different optimization barriers */
    
    /* Test 11: Nested conversions with intermediate overflow */
    const signed long _Accum la1 = 0.9999847412109375lk;  /* MAX for Q15.16 */
    const signed long _Accum la2 = 0.9999847412109375lk;
    /* Multiplication in higher precision then convert down */
    const signed _Accum a_mul = (signed _Accum)la1 * (signed _Accum)la2;
    const signed short _Fract sf_mul = (signed short _Fract)a_mul;
    
    /* Test 12: Underflow check for unsigned */
    const unsigned _Accum ua_small = (unsigned _Accum)0.0001ur;
    const unsigned short _Fract uf_small = (unsigned short _Fract)ua_small;
    
    /* Test 13: Exact boundary value */
    const signed _Accum exact_max_fract = 0.999969482421875ak;
    const signed short _Fract sf_exact = (signed short _Fract)exact_max_fract;
    
    /* Test 14: Just over boundary (by 1 LSB) */
    const signed _Accum over_max_fract = 0.999969482421875ak + 0.0000000000000001ak;
    const signed short _Fract sf_over = (signed short _Fract)over_max_fract;
    
    return 0;
}
