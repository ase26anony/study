/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_max(void) {
    return 0.99999999976716935634613037109375k; /* MAX for signed _Accum (Q15.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -1.0k; /* MIN for signed _Accum (Q15.16) */
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;

int main(void) {
    int checksum = 0;
    
    /* Test 1: Boundary values for signed _Fract */
    printf("Test 1: Signed _Fract boundary tests\n");
    
    /* At maximum boundary */
    const signed short _Fract sf_max = 0.999969482421875r;
    const signed short _Fract sf_min = -1.0r;
    
    /* Just beyond maximum (will overflow in conversion) */
    const signed _Accum just_above_sf_max = 1.000030517578125k; /* sf_max + 1LSB in Q15.16 */
    const signed _Accum just_below_sf_min = -1.000030517578125k; /* sf_min - 1LSB in Q15.16 */
    
    /* Conversions that should trigger range checks */
    volatile_sfract = (signed short _Fract)just_above_sf_max;  /* Should overflow */
    checksum += (int)(volatile_sfract * 1000);
    
    volatile_sfract = (signed short _Fract)just_below_sf_min;  /* Should underflow */
    checksum += (int)(volatile_sfract * 1000);
    
    /* Test 2: Unsigned _Fract with values near 0 and 1.0 */
    printf("Test 2: Unsigned _Fract boundary tests\n");
    
    const unsigned short _Fract uf_max = 0.999969482421875ur;
    const unsigned short _Fract uf_min = 0.0ur;
    
    /* Values that should trigger unsigned range checks */
    const signed _Accum negative_for_uf = -0.000030517578125k; /* Just below 0 */
    const signed _Accum above_one_for_uf = 1.000030517578125k; /* Just above 1.0 */
    
    volatile_ufract = (unsigned short _Fract)negative_for_uf;  /* Should underflow to 0 */
    checksum += (int)(volatile_ufract * 1000);
    
    volatile_ufract = (unsigned short _Fract)above_one_for_uf;  /* Should overflow */
    checksum += (int)(volatile_ufract * 1000);
    
    /* Test 3: Complex constant expressions with mixed precision */
    printf("Test 3: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time, triggering range checks */
    constexpr signed _Accum c1 = (signed _Accum)0.75r * 2.0k;  /* 1.5 in Q15.16 */
    constexpr signed _Accum c2 = (signed _Accum)0.999969482421875r * 1.000030517578125k;
    
    /* Convert to narrower type - should trigger range checking */
    const signed short _Fract f1 = (signed short _Fract)c1;  /* 1.5 -> 1.0 (saturates) */
    const signed short _Fract f2 = (signed short _Fract)c2;  /* ~1.0 -> MAX */
    
    volatile_sfract = f1;
    checksum += (int)(volatile_sfract * 1000);
    volatile_sfract = f2;
    checksum += (int)(volatile_sfract * 1000);
    
    /* Test 4: Saturation qualifier tests */
    printf("Test 4: Saturation qualifier tests\n");
    
    /* Saturated types should use different overflow handling */
    signed short _Fract _Sat sf_sat;
    unsigned short _Fract _Sat uf_sat;
    
    /* Operations that would overflow without saturation */
    sf_sat = (signed short _Fract _Sat)(get_sfract_max() + (signed short _Fract)0.0001r);
    uf_sat = (unsigned short _Fract _Sat)(get_ufract_max() + (unsigned short _Fract)0.0001ur);
    
    volatile_sfract = sf_sat;  /* Should be MAX due to saturation */
    checksum += (int)(volatile_sfract * 1000);
    volatile_ufract = uf_sat;  /* Should be MAX due to saturation */
    checksum += (int)(volatile_ufract * 1000);
    
    /* Test 5: Loop with small iteration count to create semi-constant values */
    printf("Test 5: Loop-based semi-constant tests\n");
    
    signed _Accum accum = 0.0k;
    for (int i = 0; i < 3; i++) {
        accum += 0.333333333333333k;  /* Add 1/3 each iteration */
        
        /* Convert to _Fract - may trigger range check each iteration */
        signed short _Fract f = (signed short _Fract)accum;
        volatile_sfract = f;
        checksum += (int)(volatile_sfract * 1000);
    }
    
    /* Test 6: Mixed signed/unsigned conversions */
    printf("Test 6: Mixed signed/unsigned conversions\n");
    
    /* Convert signed negative to unsigned - should trigger special handling */
    signed _Accum negative_value = -0.5k;
    unsigned short _Fract u_from_s = (unsigned short _Fract)negative_value;  /* Should become 0 */
    
    /* Convert unsigned to signed - may also trigger range checks */
    unsigned short _Fract large_uf = 0.999969482421875ur;
    signed short _Fract s_from_u = (signed short _Fract)large_uf;  /* Should work */
    
    volatile_ufract = u_from_s;
    checksum += (int)(volatile_ufract * 1000);
    volatile_sfract = s_from_u;
    checksum += (int)(volatile_sfract * 1000);
    
    /* Test 7: Multiplication that overflows fixed-point range */
    printf("Test 7: Multiplication overflow tests\n");
    
    /* Multiply two values whose product exceeds 1.0 */
    const signed short _Fract a = 0.999969482421875r;  /* MAX */
    const signed short _Fract b = 1.5r;  /* Actually 1.0 due to saturation, but conceptually >1 */
    
    /* The multiplication in wider type then conversion should trigger range check */
    signed _Accum product = (signed _Accum)a * (signed _Accum)b;
    signed short _Fract narrowed = (signed short _Fract)product;
    
    volatile_sfract = narrowed;
    checksum += (int)(volatile_sfract * 1000);
    
    /* Final output to prevent complete optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
