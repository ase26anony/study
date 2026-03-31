/* fixed-point-test.c
 * Test program for GCC fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -32768.999969482421875k; /* Approx min for 16-bit accum */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accum sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Signed fract boundary testing */
    printf("Test 1: Signed _Fract boundary tests\n");
    
    /* Maximum representable signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed short _Fract min_sfract = -1.0r;
    
    /* Values near boundaries */
    const signed short _Fract just_below_max = 0.99993896484375r;  /* max - 1LSB */
    const signed short _Fract just_above_min = -0.999969482421875r; /* min + 1LSB */
    
    /* Operations that should trigger range checks */
    volatile signed short _Fract result1;
    
    /* This multiplication should produce value requiring range check */
    result1 = max_sfract * just_below_max;
    
    /* Test 2: Conversion between different fixed-point types */
    printf("Test 2: Type conversion range checks\n");
    
    /* Use _Accum types which have more bits */
    const signed _Accum large_accum = 255.999969482421875k;  /* Large value */
    const signed _Accum neg_large_accum = -256.999969482421875k;
    
    /* Convert to narrower types - should trigger range checking */
    volatile signed short _Fract conv1 = (signed short _Fract)large_accum;
    volatile signed short _Fract conv2 = (signed short _Fract)neg_large_accum;
    
    /* Test 3: Complex constant expressions */
    printf("Test 3: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time with range checking */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;
    constexpr signed _Accum c2 = (signed _Accum)(-0.75r) * 2.0r;
    
    /* Convert to different precision */
    const signed short _Fract from_c1 = (signed short _Fract)c1;
    const signed short _Fract from_c2 = (signed short _Fract)c2;
    
    /* Test 4: Mixed signed/unsigned conversions */
    printf("Test 4: Signed/unsigned mixed conversions\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    const signed _Accum signed_to_unsigned = 0.999969482421875k;
    const signed _Accum negative_signed = -0.5k;
    
    /* These conversions should trigger range checks */
    volatile unsigned short _Fract uconv1 = (unsigned short _Fract)signed_to_unsigned;
    volatile unsigned short _Fract uconv2 = (unsigned short _Fract)negative_signed;
    
    /* Test 5: Saturation qualifier testing */
    printf("Test 5: Saturation qualifier tests\n");
    
    /* Saturated types have different overflow behavior */
    volatile signed short _Sat _Fract sat_result;
    volatile unsigned short _Sat _Fract usat_result;
    
    /* Operations that would overflow without saturation */
    sat_result = max_sfract * max_sfract;  /* Should saturate */
    usat_result = max_ufract * max_ufract; /* Should saturate */
    
    /* Test 6: Boundary value arithmetic */
    printf("Test 6: Boundary value arithmetic\n");
    
    /* Create values at exact boundaries */
    const signed _Accum boundary_accum = 32767.999969482421875k;  /* Near 16-bit accum max */
    const signed _Accum small_increment = 0.000030517578125k;     /* 1 LSB for some formats */
    
    /* Operations near boundaries */
    volatile signed _Accum near_boundary = boundary_accum + small_increment;
    volatile signed short _Fract from_near_boundary = (signed short _Fract)near_boundary;
    
    /* Test 7: Loop-based computations (prevents some optimizations) */
    printf("Test 7: Loop-based computations\n");
    
    volatile signed short _Fract loop_result = 0.0r;
    for (int i = 0; i < 3; i++) {
        /* Build up value gradually */
        loop_result = loop_result + 0.333333333333333r;
        
        /* Convert intermediate result to different type */
        volatile signed _Accum temp_accum = (signed _Accum)loop_result;
        volatile signed short _Fract back_conv = (signed short _Fract)temp_accum;
    }
    
    /* Test 8: Explicit overflow cases */
    printf("Test 8: Explicit overflow cases\n");
    
    /* Values guaranteed to overflow when converted */
    const signed _Accum way_too_big = 1000.0k;
    const signed _Accum way_too_small = -1000.0k;
    
    volatile signed short _Fract overflow1 = (signed short _Fract)way_too_big;
    volatile signed short _Fract overflow2 = (signed short _Fract)way_too_small;
    
    /* Test 9: Use of opaque function results */
    printf("Test 9: Opaque function results\n");
    
    /* Compiler can't know these values at compile time */
    signed short _Fract opaque_max = get_sfract_max();
    unsigned short _Fract opaque_umax = get_ufract_max();
    signed _Accum opaque_min = get_saccum_min();
    
    /* Operations with opaque values */
    volatile signed short _Fract opaque_result = opaque_max * 0.9r;
    volatile unsigned short _Fract opaque_uresult = opaque_umax * 0.9ur;
    
    /* Convert opaque accum to fract */
    volatile signed short _Fract from_opaque_accum = (signed short _Fract)opaque_min;
    
    /* Test 10: Prevent dead code elimination */
    printf("Test 10: Final consumption\n");
    
    /* Force all results to be used */
    volatile signed _Accum final_sink = 0.0k;
    final_sink = final_sink + (signed _Accum)result1;
    final_sink = final_sink + (signed _Accum)conv1;
    final_sink = final_sink + (signed _Accum)conv2;
    final_sink = final_sink + (signed _Accum)from_c1;
    final_sink = final_sink + (signed _Accum)from_c2;
    final_sink = final_sink + (signed _Accum)uconv1;
    final_sink = final_sink + (signed _Accum)uconv2;
    final_sink = final_sink + (signed _Accum)sat_result;
    final_sink = final_sink + (signed _Accum)usat_result;
    final_sink = final_sink + (signed _Accum)from_near_boundary;
    final_sink = final_sink + (signed _Accum)loop_result;
    final_sink = final_sink + (signed _Accum)overflow1;
    final_sink = final_sink + (signed _Accum)overflow2;
    final_sink = final_sink + (signed _Accum)opaque_result;
    final_sink = final_sink + (signed _Accum)opaque_uresult;
    final_sink = final_sink + (signed _Accum)from_opaque_accum;
    
    /* Use consume_accum to prevent optimization */
    volatile signed _Accum final_result = consume_accum(final_sink);
    
    /* Simple checksum output */
    printf("Final checksum (lower bits): %lld\n", 
           (long long)(final_result * 1000000k));
    
    return 0;
}
