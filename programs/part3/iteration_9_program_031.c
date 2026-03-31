/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */
/* Also try: gcc -O2 -std=gnu11 -fno-math-errno -fno-trapping-math fixed-point-test.c */
/* And: gcc -Os -std=gnu11 -frounding-math -fsignaling-nans fixed-point-test.c */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775808lk; /* MIN for signed long _Accum (Q63.0) */
}

__attribute__((noinline)) signed _Accum get_saccum_mid(void) {
    return 0.5k;
}

/* Function to consume values and prevent dead code elimination */
__attribute__((noinline)) void consume(volatile void *ptr) {
    asm volatile("" : : "r"(ptr) : "memory");
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed short _Fract min_sfract = -1.0r;
    
    /* Values just beyond boundaries */
    const signed short _Fract slightly_above_max = 1.000030517578125r;  /* MAX + 1LSB */
    const signed short _Fract slightly_below_min = -1.000030517578125r; /* MIN - 1LSB */
    
    /* Test 2: Complex constant expressions that require range checking */
    const signed _Accum c1 = (signed _Accum)0.75r * 2.0r;  /* 1.5k -> needs conversion */
    const signed _Accum c2 = (signed _Accum)(-0.875r) * 3.0r; /* -2.625k */
    
    /* Test 3: Mixed precision with volatile to force materialization */
    volatile signed _Accum v1 = get_saccum_mid();
    volatile signed short _Fract v2 = v1;  /* Conversion: _Accum -> short _Fract */
    
    /* Test 4: Operations that produce values at boundaries */
    signed long _Accum la1 = 9223372036854775807lk;  /* MAX for signed long _Accum */
    signed long _Accum la2 = la1 + 1lk;  /* Overflow for Q63.0 */
    
    /* Test 5: Saturation qualifier tests */
    signed short _Fract _Sat sf_sat1 = 2.0r;  /* Should saturate to MAX */
    signed short _Fract _Sat sf_sat2 = -2.0r; /* Should saturate to MIN */
    
    /* Test 6: Unsigned fixed-point with overflow */
    unsigned short _Fract uf1 = 1.5ur;  /* Should overflow to MAX */
    unsigned short _Fract uf2 = -0.5ur; /* Should underflow to 0 */
    
    /* Test 7: Conversion chain with intermediate overflow */
    signed _Accum ak1 = 100.0k;
    signed _Accum ak2 = 100.0k;
    signed _Accum ak_prod = ak1 * ak2;  /* 10000.0k */
    signed short _Fract sf_from_prod = ak_prod;  /* Needs range check */
    
    /* Test 8: Values that exercise a_high.sgt(max_r) comparison */
    /* Create values where high word comparison matters */
    signed long _Accum huge_val = 9223372036854775807lk;  /* 2^63-1 */
    signed short _Fract sf_huge = huge_val;  /* Should trigger overflow check */
    
    /* Test 9: Values that exercise a_low.ugt(max_s) comparison */
    /* For cases where high words equal but low word exceeds */
    signed _Accum ak_near_max = 32767.999969482421875k;  /* Close to short _Fract MAX */
    signed short _Fract sf_near = ak_near_max;  /* May trigger low word check */
    
    /* Test 10: Loop with fixed-point accumulation */
    volatile signed short _Fract accum = 0.0r;
    for (int i = 0; i < 10; i++) {
        accum = accum + 0.1r;  /* Accumulate to potentially overflow */
    }
    
    /* Test 11: Mixed signed/unsigned conversions */
    unsigned short _Fract uf_from_signed = (signed short _Fract)(-0.5r);
    signed short _Fract sf_from_unsigned = (unsigned short _Fract)1.5ur;
    
    /* Test 12: Complex expression with multiple conversions */
    constexpr signed _Accum complex_expr = 
        (signed _Accum)((signed short _Fract)0.9r * (signed short _Fract)0.9r) * 2.0k;
    
    /* Prevent optimization */
    consume(&max_sfract);
    consume(&min_sfract);
    consume(&slightly_above_max);
    consume(&slightly_below_min);
    consume(&c1);
    consume(&c2);
    consume(&v2);
    consume(&la2);
    consume(&sf_sat1);
    consume(&sf_sat2);
    consume(&uf1);
    consume(&uf2);
    consume(&sf_from_prod);
    consume(&sf_huge);
    consume(&sf_near);
    consume(&accum);
    consume(&uf_from_signed);
    consume(&sf_from_unsigned);
    
    /* Create a simple checksum to ensure all computations happen */
    uint32_t checksum = 0;
    
    /* Use the fixed-point values in a way that can't be optimized away */
    checksum += *(uint32_t*)&max_sfract;
    checksum += *(uint32_t*)&min_sfract;
    checksum += *(uint32_t*)&c1;
    checksum += *(uint32_t*)&c2;
    checksum += *(uint32_t*)&accum;
    
    printf("Checksum: %u\n", checksum);
    printf("Test completed - if you see this, no compile-time errors occurred\n");
    
    return 0;
}
