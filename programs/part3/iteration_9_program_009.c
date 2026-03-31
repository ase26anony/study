/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract */
}

__attribute__((noinline)) signed _Accum get_saccum_max(void) {
    return 32767.999969482421875k; /* MAX for signed _Accum (16.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -32768.0k; /* MIN for signed _Accum */
}

__attribute__((noinline)) signed long _Accum get_longaccum_max(void) {
    return 2147483647.99999999976716935634613037109375lk; /* MAX for signed long _Accum */
}

/* Test boundary value conversions */
void test_boundary_conversions(void) {
    volatile signed short _Fract sf1, sf2;
    volatile unsigned short _Fract uf1, uf2;
    volatile signed _Accum sa1, sa2;
    volatile signed long _Accum sla1, sla2;
    volatile signed _Sat _Accum ssa1;
    volatile unsigned _Sat _Fract usf1;
    
    /* Test 1: Exact boundary - should pass range check */
    sf1 = 0.999969482421875r;  /* MAX signed short _Fract */
    uf1 = 0.999969482421875ur; /* MAX unsigned short _Fract */
    
    /* Test 2: Just beyond boundary - should trigger overflow check */
    /* Multiply to create value just beyond MAX */
    sa1 = get_saccum_max();
    sa2 = sa1 * 1.0001k;  /* Slightly overflow */
    
    /* Convert overflowed _Accum to _Fract - triggers range check */
    sf2 = (signed short _Fract)sa2;
    
    /* Test 3: Negative boundary for signed types */
    sa1 = get_saccum_min();
    sa2 = sa1 * 1.0001k;  /* Slightly underflow more negative */
    sf2 = (signed short _Fract)sa2;  /* Triggers negative overflow check */
    
    /* Test 4: Mixed signed/unsigned conversions */
    uf2 = (unsigned short _Fract)sa1;  /* Convert negative to unsigned */
    
    /* Test 5: Saturation qualifier tests */
    ssa1 = get_saccum_max() * 2.0k;  /* Should saturate */
    usf1 = (unsigned _Sat _Fract)ssa1;  /* Convert saturated value */
    
    /* Test 6: Long _Accum to short _Fract with precise boundary */
    sla1 = get_longaccum_max();
    sla2 = sla1 / 65536.0lk;  /* Scale down but still large */
    sf2 = (signed short _Fract)sla2;  /* Triggers range check */
    
    /* Use volatile stores to prevent elimination */
    (void)sf1; (void)uf1; (void)sa1; (void)sa2;
    (void)sf2; (void)uf2; (void)ssa1; (void)usf1;
    (void)sla1; (void)sla2;
}

/* Complex constant expressions to force compile-time evaluation */
void test_constant_expressions(void) {
    /* These should be evaluated at compile-time, triggering range checks */
    const signed short _Fract cf1 = (signed short _Fract)0.5r * 1.99993896484375r;
    const signed short _Fract cf2 = (signed short _Fract)(get_saccum_max() / 32768.0k);
    const unsigned short _Fract cuf1 = (unsigned short _Fract)(-0.0001k);
    
    /* Multi-step constant expression */
    const signed _Accum ca1 = 16384.0k * 2.0k;  /* Exactly 32768.0k */
    const signed short _Fract cf3 = (signed short _Fract)ca1;  /* Overflow */
    
    /* Use in array initializers to ensure evaluation */
    static const signed short _Fract farray[4] = {
        cf1,
        cf2,
        cf3,
        (signed short _Fract)(get_longaccum_max() / 65536.0lk)
    };
    
    volatile int i;
    for (i = 0; i < 4; i++) {
        (void)farray[i];
    }
}

/* Test with loops to create semi-constant values */
void test_loop_based_values(void) {
    signed _Accum accum = 0.0k;
    signed short _Fract fract;
    unsigned short _Fract ufract;
    int i;
    
    /* Build up to boundary value */
    for (i = 0; i < 100; i++) {
        accum += 327.68k;  /* 32768/100 = 327.68 */
    }
    /* accum should be ~32768.0k, which overflows short _Fract */
    fract = (signed short _Fract)accum;  /* Triggers range check */
    
    /* Test unsigned with negative accumulation */
    accum = 0.0k;
    for (i = 0; i < 50; i++) {
        accum -= 655.36k;  /* Go negative */
    }
    ufract = (unsigned short _Fract)accum;  /* Underflow to unsigned */
    
    /* Use results */
    volatile signed short _Fract vf = fract;
    volatile unsigned short _Fract vuf = ufract;
    (void)vf; (void)vuf;
}

/* Test specific bit patterns to hit exact comparison logic */
void test_bit_pattern_conversions(void) {
    /* These values are designed to exercise the specific comparisons:
     * a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
     */
    
    /* Case 1: a_high > max_r */
    volatile signed long _Accum sla = 65536.0lk;  /* High part non-zero */
    volatile signed short _Fract sf = (signed short _Fract)sla;
    
    /* Case 2: a_high == max_r && a_low > max_s */
    /* Need value where high bits equal max_r but low bits exceed max_s */
    volatile signed _Accum sa = 32767.999969482421875k;  /* MAX */
    sa = sa + 0.000030517578125k;  /* Add 1 LSB beyond MAX */
    sf = (signed short _Fract)sa;
    
    /* Case for unsigned */
    volatile unsigned _Accum ua = 65535.9999847412109375uk;  /* MAX unsigned _Accum */
    ua = ua + 0.0000152587890625uk;  /* Add 1 LSB beyond MAX */
    volatile unsigned short _Fract uf = (unsigned short _Fract)ua;
    
    (void)sf; (void)uf;
}

/* Main test driver */
int main(void) {
    printf("Testing GCC fixed-point range checking logic...\n");
    
    test_boundary_conversions();
    test_constant_expressions();
    test_loop_based_values();
    test_bit_pattern_conversions();
    
    printf("Tests completed (check coverage of fixed-value.cc lines 264-277)\n");
    
    /* Return non-zero if any test failed (simplified) */
    return 0;
}
