/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -1.99999988079071044921875k; /* Close to min for 16-bit accum */
}

__attribute__((noinline)) volatile void use_result(volatile void *ptr) {
    /* Force compiler to materialize value */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* ===== Test 1: Signed overflow at maximum boundary ===== */
    {
        /* This should trigger a_high.sgt(max_r) or a_low.ugt(max_s) */
        const signed short _Fract max_sfract = 0.999969482421875r; /* Q0.15 MAX */
        const signed short _Fract small = 0.000030517578125r; /* 1 LSB */
        
        /* Operation that overflows signed Q0.15 range */
        volatile signed _Accum temp = (signed _Accum)max_sfract + (signed _Accum)small;
        
        /* Conversion to narrower type triggers range check */
        volatile signed short _Fract result1 = (signed short _Fract)temp;
        checksum += *(int*)&result1;
        
        use_result(&result1);
    }
    
    /* ===== Test 2: Unsigned overflow at maximum boundary ===== */
    {
        const unsigned short _Fract max_ufract = 0.999969482421875ur; /* U0.16 MAX */
        const unsigned short _Fract tiny = 0.0000152587890625ur; /* 0.5 LSB */
        
        /* Create value just above max for unsigned short _Fract */
        volatile unsigned _Accum temp = (unsigned _Accum)max_ufract * 1.0001ur;
        
        /* This conversion should trigger unsigned overflow check */
        volatile unsigned short _Fract result2 = (unsigned short _Fract)temp;
        checksum += *(int*)&result2;
        
        use_result(&result2);
    }
    
    /* ===== Test 3: Complex constant expression with mixed types ===== */
    {
        /* Complex compile-time expression */
        constexpr signed _Accum a = (signed _Accum)0.75r;
        constexpr signed _Accum b = (signed _Accum)0.8r;
        constexpr signed _Accum c = a * b * 2.0r; /* 0.75 * 0.8 * 2 = 1.2 */
        
        /* This exceeds signed short _Fract range (0.9999...) */
        volatile signed short _Fract result3 = (signed short _Fract)c;
        checksum += *(int*)&result3;
        
        /* Force evaluation with volatile */
        volatile signed _Accum temp = c;
        use_result(&temp);
    }
    
    /* ===== Test 4: Negative overflow for signed types ===== */
    {
        const signed _Accum min_val = -1.99999988079071044921875k;
        const signed _Accum offset = -0.00000011920928955078125k; /* Tiny negative */
        
        /* Create value just below minimum */
        volatile signed _Accum temp = min_val + offset;
        
        /* Conversion to signed short _Fract (range -1 to ~0.9999) */
        volatile signed short _Fract result4 = (signed short _Fract)temp;
        checksum += *(int*)&result4;
        
        use_result(&result4);
    }
    
    /* ===== Test 5: Saturation qualifier tests ===== */
    {
        /* Test with _Sat qualifier - may use different overflow logic */
        volatile signed _Sat _Accum sat_max = 1.99999988079071044921875k;
        volatile signed _Sat _Accum sat_inc = 0.00000011920928955078125k;
        
        /* This should saturate rather than wrap */
        volatile signed _Sat _Accum sat_result = sat_max + sat_inc;
        
        /* Convert saturated result to narrower type */
        volatile signed short _Fract result5 = (signed short _Fract)sat_result;
        checksum += *(int*)&result5;
        
        use_result(&sat_result);
    }
    
    /* ===== Test 6: Mixed signed/unsigned conversions ===== */
    {
        /* Negative signed value to unsigned conversion */
        volatile signed _Accum neg_value = -0.5k;
        
        /* This should trigger range check (negative to unsigned) */
        volatile unsigned short _Fract result6 = (unsigned short _Fract)neg_value;
        checksum += *(int*)&result6;
        
        use_result(&result6);
    }
    
    /* ===== Test 7: Loop with constant iterations ===== */
    {
        /* Loop prevents complete compile-time elimination */
        volatile signed short _Fract accum = 0.5r;
        for (int i = 0; i < 3; i++) {
            accum = accum * 1.1r; /* Grows beyond 1.0 */
        }
        
        /* Final conversion after loop */
        volatile signed char _Fract result7 = (signed char _Fract)accum;
        checksum += *(int*)&result7;
        
        use_result(&accum);
    }
    
    /* ===== Test 8: Boundary value exactly at maximum ===== */
    {
        /* Value exactly at maximum for signed short _Fract */
        const signed short _Fract exact_max = 0.999969482421875r;
        
        /* Convert through intermediate type */
        volatile signed _Accum temp = (signed _Accum)exact_max;
        
        /* This should pass range check (a_high == max_r && a_low == max_s) */
        volatile signed short _Fract result8 = (signed short _Fract)temp;
        checksum += *(int*)&result8;
        
        use_result(&result8);
    }
    
    /* ===== Test 9: One LSB beyond maximum ===== */
    {
        /* Create value 1 LSB beyond maximum */
        const signed _Accum beyond_max = 1.000030517578125k; /* 1 + 1 LSB of Q0.15 */
        
        /* This should fail range check (a_high == max_r && a_low.ugt(max_s)) */
        volatile signed short _Fract result9 = (signed short _Fract)beyond_max;
        checksum += *(int*)&result9;
        
        use_result(&beyond_max);
    }
    
    /* ===== Test 10: Very large value (high part exceeds max_r) ===== */
    {
        /* Create value where high part alone exceeds max_r */
        volatile signed long _Accum huge = 1000.0lk;
        
        /* This should trigger a_high.sgt(max_r) */
        volatile signed short _Fract result10 = (signed short _Fract)huge;
        checksum += *(int*)&result10;
        
        use_result(&huge);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
