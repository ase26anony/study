/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -1.99999988079071044921875lk; /* MIN for signed long _Accum (Q15.16) */
}

__attribute__((noinline)) signed _Sat long _Accum get_sat_accum(void) {
    return 0.5lk;
}

/* Function to consume values and prevent dead code elimination */
volatile signed short _Fract global_sfract;
volatile unsigned short _Fract global_ufract;
volatile signed _Sat short _Fract global_sat_sfract;

__attribute__((noinline)) void consume_values(signed short _Fract sf, 
                                              unsigned short _Fract uf,
                                              signed _Sat short _Fract sat_sf) {
    global_sfract = sf;
    global_ufract = uf;
    global_sat_sfract = sat_sf;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract max_sfract = 0.999969482421875r;      /* Q0.15 max */
    const signed short _Fract min_sfract = -1.0r;                   /* Q0.15 min */
    const signed short _Fract just_above_max = 1.000030517578125r;  /* MAX + 1LSB */
    const signed short _Fract just_below_min = -1.000030517578125r; /* MIN - 1LSB */
    
    /* Test 2: Boundary values for unsigned types */
    const unsigned short _Fract max_ufract = 0.999969482421875ur;   /* U0.16 max */
    const unsigned short _Fract zero_ufract = 0.0ur;                /* U0.16 min */
    const unsigned short _Fract just_above_max_u = 1.000030517578125ur; /* MAX + 1LSB */
    
    /* Test 3: _Accum types with different precision */
    const signed long _Accum max_laccum = 32767.9999847412109375lk; /* Q15.16 max */
    const signed long _Accum min_laccum = -32768.0lk;               /* Q15.16 min */
    const signed short _Accum max_saccum = 127.99951171875hk;       /* Q7.8 max */
    
    /* Test 4: Saturated types */
    const signed _Sat short _Fract sat_max_sfract = 0.999969482421875r;
    const signed _Sat long _Accum sat_max_laccum = 32767.9999847412109375lk;
    
    /* Complex constant expressions that should trigger range checking */
    
    /* 1. Conversion from _Accum to _Fract that overflows */
    volatile signed long _Accum v1 = max_laccum * 1.0001lk;  /* Slightly > max */
    volatile signed short _Fract f1 = (signed short _Fract)v1; /* Should trigger range check */
    
    /* 2. Conversion that underflows */
    volatile signed long _Accum v2 = min_laccum * 1.0001lk;  /* Slightly < min */
    volatile signed short _Fract f2 = (signed short _Fract)v2;
    
    /* 3. Arithmetic near boundaries */
    volatile signed short _Fract f3 = max_sfract + (signed short _Fract)0.0001r;
    volatile signed short _Fract f4 = min_sfract - (signed short _Fract)0.0001r;
    
    /* 4. Mixed signed/unsigned conversions */
    volatile unsigned short _Fract uf1 = (unsigned short _Fract)max_sfract;
    volatile unsigned short _Fract uf2 = (unsigned short _Fract)min_sfract; /* Negative to unsigned */
    
    /* 5. Saturated arithmetic */
    volatile signed _Sat short _Fract sat1 = sat_max_sfract + (signed _Sat short _Fract)0.5r;
    volatile signed _Sat long _Accum sat2 = sat_max_laccum * 2.0lk;
    
    /* 6. Complex nested expressions */
    const signed short _Fract c1 = (signed short _Fract)(0.75r * 1.333r);  /* ~1.0 */
    const signed short _Fract c2 = (signed short _Fract)(-0.75r * 1.333r); /* ~-1.0 */
    
    /* 7. Use opaque function results in conversions */
    volatile signed short _Fract f5 = (signed short _Fract)get_saccum_min();
    volatile unsigned short _Fract uf3 = (unsigned short _Fract)get_ufract_max();
    
    /* 8. Loop with fixed iteration to create semi-constant values */
    signed short _Fract accum = 0.0r;
    for (int i = 0; i < 3; i++) {
        accum += 0.333r;  /* Should approach 0.999r */
    }
    volatile signed short _Fract f6 = accum;
    
    /* 9. Multiplication that overflows precision */
    volatile signed long _Accum v3 = max_saccum * max_saccum;  /* ~16384, exceeds Q7.8 range */
    volatile signed short _Accum a1 = (signed short _Accum)v3;
    
    /* 10. Boundary case: exactly at maximum */
    volatile signed short _Fract f7 = (signed short _Fract)max_sfract;
    volatile signed short _Fract f8 = (signed short _Fract)(max_sfract - (signed short _Fract)0.000030517578125r); /* MAX - 1LSB */
    
    /* 11. Test with just beyond maximum (should trigger a_low.ugt(max_s) check) */
    volatile signed long _Accum v4 = (signed long _Accum)max_sfract;
    /* Add a small epsilon in Q15.16 format */
    v4 = v4 + (signed long _Accum)(1.0 / 65536.0);  /* Add 1 LSB in Q15.16 */
    volatile signed short _Fract f9 = (signed short _Fract)v4;
    
    /* 12. Similar test for unsigned */
    volatile unsigned long _Accum v5 = (unsigned long _Accum)max_ufract;
    v5 = v5 + (unsigned long _Accum)(1.0 / 65536.0);  /* Add 1 LSB */
    volatile unsigned short _Fract uf4 = (unsigned short _Fract)v5;
    
    /* Consume all values to prevent optimization */
    consume_values(f1, uf1, sat1);
    consume_values(f2, uf2, sat1);
    consume_values(f3, uf3, sat1);
    consume_values(f4, uf4, sat1);
    consume_values(f5, uf1, sat1);
    consume_values(f6, uf2, sat1);
    consume_values(f7, uf3, sat1);
    consume_values(f8, uf4, sat1);
    consume_values(f9, uf1, sat1);
    
    /* Create a simple checksum for observable behavior */
    signed short _Fract checksum = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9;
    checksum += (signed short _Fract)uf1 + (signed short _Fract)uf2 + 
                (signed short _Fract)uf3 + (signed short _Fract)uf4;
    checksum += sat1;
    
    /* Print something to ensure the program runs */
    printf("Checksum (as float): %f\n", (double)checksum);
    printf("Test completed.\n");
    
    return 0;
}
