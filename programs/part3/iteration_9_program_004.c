/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -32768.999969482421875k; /* Approx min for signed _Accum (Q15.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_max(void) {
    return 32767.999969482421875k;  /* Approx max for signed _Accum (Q15.16) */
}

/* Force materialization of values */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;
volatile signed long _Accum volatile_laccum;

int main(void) {
    int checksum = 0;
    
    /* Test 1: Direct boundary conversions */
    printf("Test 1: Boundary conversions\n");
    
    /* Maximum signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed short _Fract max_sfract_minus_lsb = 0.99993896484375r;
    
    /* Convert to narrower type - should trigger range check */
    signed _Fract f1 = max_sfract;  /* Q0.15 to Q0.15, same size */
    signed char _Fract f2 = max_sfract;  /* Q0.15 to Q0.7 - may overflow */
    
    volatile_sfract = f1;
    volatile_sfract = f2;
    checksum += (int)(volatile_sfract * 1000);
    
    /* Test 2: Arithmetic near boundaries */
    printf("Test 2: Arithmetic near boundaries\n");
    
    /* Use volatile to prevent constant folding */
    volatile signed _Accum v1 = 32767.999969482421875k;  /* Near max */
    volatile signed _Accum v2 = 0.000030517578125k;      /* One LSB */
    
    /* This addition would overflow in Q15.16 */
    signed _Accum sum = v1 + v2;
    
    /* Convert to narrower type - triggers range check */
    signed short _Accum narrow_sum = sum;  /* Q15.16 to Q7.8 */
    
    volatile_saccum = narrow_sum;
    checksum += (int)(volatile_saccum * 100);
    
    /* Test 3: Complex constant expressions */
    printf("Test 3: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 */
    constexpr signed _Accum c2 = (signed _Accum)0.999969482421875r * 32768.0k;
    
    /* Convert to different precisions */
    const signed long _Accum cl1 = c1;  /* Q31.32 */
    const signed short _Fract cf1 = c1; /* Q0.15 - 1.5 > 1.0, may saturate */
    
    volatile_laccum = cl1;
    volatile_sfract = cf1;
    checksum += (int)(volatile_laccum * 1000);
    checksum += (int)(volatile_sfract * 1000);
    
    /* Test 4: Mixed signed/unsigned conversions */
    printf("Test 4: Mixed signed/unsigned conversions\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    const signed short _Fract neg_sfract = -0.5r;
    
    /* Signed to unsigned conversion - triggers range check */
    unsigned short _Fract u1 = (unsigned short _Fract)neg_sfract;  /* Negative to unsigned */
    unsigned short _Fract u2 = max_ufract;
    
    volatile_ufract = u1;
    volatile_ufract = u2;
    checksum += (int)(volatile_ufract * 1000);
    
    /* Test 5: Saturation qualifier tests */
    printf("Test 5: Saturation tests\n");
    
    /* _Sat types should use different overflow handling */
    signed _Sat _Accum sat1 = 40000.0k;  /* Above max for Q15.16 */
    signed _Sat short _Accum sat2 = sat1;  /* Convert with saturation */
    
    unsigned _Sat short _Fract sat3 = 1.5ur;  /* Above max for U0.16 */
    unsigned _Sat short _Fract sat4 = -0.5ur; /* Below min for unsigned */
    
    volatile_saccum = sat1;
    volatile_saccum = sat2;
    volatile_ufract = sat3;
    volatile_ufract = sat4;
    checksum += (int)(volatile_saccum * 100);
    checksum += (int)(volatile_ufract * 100);
    
    /* Test 6: Loop with boundary values */
    printf("Test 6: Loop tests\n");
    
    signed short _Fract accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Build up to overflow */
        accum += 0.6r;
        
        /* Convert to narrower type each iteration */
        signed char _Fract narrow = accum;  /* Q0.15 to Q0.7 */
        
        volatile_sfract = narrow;
        checksum += (int)(volatile_sfract * 100);
    }
    
    /* Test 7: Multiplication overflow */
    printf("Test 7: Multiplication overflow\n");
    
    const signed _Accum m1 = 200.0k;
    const signed _Accum m2 = 200.0k;
    const signed _Accum product = m1 * m2;  /* 40000 > 32767.999... */
    
    /* Convert to narrower fixed-point type */
    signed short _Accum narrow_product = product;  /* Q15.16 to Q7.8 */
    
    volatile_saccum = narrow_product;
    checksum += (int)(volatile_saccum * 100);
    
    /* Test 8: Exact boundary values */
    printf("Test 8: Exact boundary values\n");
    
    /* Values at exact boundaries */
    const signed long _Accum exact_max = 32767.999969482421875lk;
    const signed long _Accum exact_min = -32768.999969482421875lk;
    const signed long _Accum just_above_max = 32768.0lk;
    const signed long _Accum just_below_min = -32769.0lk;
    
    /* Convert to different precisions */
    signed _Accum conv1 = exact_max;      /* Should fit */
    signed _Accum conv2 = exact_min;      /* Should fit */
    signed _Accum conv3 = just_above_max; /* May overflow */
    signed _Accum conv4 = just_below_min; /* May underflow */
    
    volatile_saccum = conv1;
    volatile_saccum = conv2;
    volatile_saccum = conv3;
    volatile_saccum = conv4;
    checksum += (int)(volatile_saccum * 100);
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
