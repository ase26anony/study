/* fixed-point-test.c
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 * Also test with: -O2 -std=gnu11 -fno-math-errno -fno-trapping-math
 */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile void consume(volatile void *ptr) {
    /* Force materialization of value */
    (void)ptr;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    
    /* Maximum signed short _Fract (Q0.15) */
    const signed short _Fract max_sfract = 0.999969482421875r;
    
    /* Values near maximum to trigger a_high.sgt(max_r) check */
    volatile signed long _Accum v1 = (signed long _Accum)max_sfract;
    volatile signed long _Accum v2 = v1 * 1.0001k;  /* Slightly overflow */
    
    /* Convert to narrower type - should trigger range check */
    signed short _Fract f1 = (signed short _Fract)v2;
    consume(&f1);
    
    /* Test 2: Complex constant expressions forcing range checking */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 */
    constexpr signed _Accum c2 = (signed _Accum)0.9999999999999999r;
    
    /* Multiplication that could overflow when converted */
    const signed _Accum c3 = c1 * c2;
    
    /* Convert to _Fract with different scaling - triggers range check */
    volatile signed _Fract f2 = (signed _Fract)c3;
    consume(&f2);
    
    /* Test 3: Boundary value testing for unsigned types */
    unsigned long _Fract uf1 = 0.9999999999999999ur;  /* Near max */
    unsigned short _Fract uf2 = (unsigned short _Fract)uf1;  /* Conversion */
    
    /* Create value just beyond max for unsigned short _Fract */
    unsigned short _Fract uf3 = get_ufract_max();
    /* Add small epsilon - may overflow when used in expressions */
    volatile unsigned short _Fract uf4 = uf3 + 0.0001ur;
    consume(&uf4);
    
    /* Test 4: Mixed precision with saturation */
    signed _Sat short _Fract sf1 = 0.9r;
    signed _Sat short _Fract sf2 = 0.8r;
    
    /* Multiplication that would saturate */
    signed _Sat short _Fract sf3 = sf1 * sf2;
    
    /* Convert saturated value to different type */
    signed _Fract f3 = (signed _Fract)sf3;
    consume(&f3);
    
    /* Test 5: Negative overflow for signed types */
    const signed short _Fract min_sfract = -1.0r;
    signed long _Accum v3 = (signed long _Accum)min_sfract;
    signed long _Accum v4 = v3 * 1.1k;  /* More negative */
    
    /* Convert to _Fract - triggers min range check */
    signed short _Fract f4 = (signed short _Fract)v4;
    consume(&f4);
    
    /* Test 6: Loop with fixed-point accumulation */
    signed _Accum acc = 0.0k;
    for (int i = 0; i < 10; i++) {
        acc += 0.123456789k;
    }
    
    /* Convert accumulated value to narrower type */
    signed _Fract f5 = (signed _Fract)acc;
    consume(&f5);
    
    /* Test 7: Direct maximum value assignment */
    /* This should trigger a_high == max_r && a_low.ugt(max_s) check */
    const signed long _Accum max_saccum = 9223372036854775.807k;  /* Approx max */
    signed _Fract f6 = (signed _Fract)max_saccum;
    consume(&f6);
    
    /* Test 8: Values requiring i_f_bits shifting in min_s calculation */
    /* Create a value that needs many fractional bits */
    signed long _Accum v5 = 0.0000000000000001k;
    for (int i = 0; i < 5; i++) {
        v5 *= v5;  /* Very small value */
    }
    
    /* Convert with different fractional bits */
    signed short _Fract f7 = (signed short _Fract)v5;
    consume(&f7);
    
    /* Test 9: Mixed signed/unsigned conversions */
    unsigned _Accum ua1 = 0.5ur;
    signed _Fract f8 = (signed _Fract)ua1;  /* Conversion check */
    consume(&f8);
    
    /* Test 10: Compile-time constant that's just at boundary */
    constexpr signed _Accum boundary = (signed _Accum)0.9999999999999999r;
    constexpr signed short _Fract f9 = (signed short _Fract)boundary;
    volatile signed short _Fract vf9 = f9;
    consume(&vf9);
    
    /* Create a checksum to ensure all values are used */
    volatile int checksum = 0;
    checksum += *(int*)&f1;
    checksum += *(int*)&f2;
    checksum += *(int*)&uf2;
    checksum += *(int*)&f3;
    checksum += *(int*)&f4;
    checksum += *(int*)&f5;
    checksum += *(int*)&f6;
    checksum += *(int*)&f7;
    checksum += *(int*)&f8;
    checksum += *(int*)&vf9;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
