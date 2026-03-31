/* Test program for fixed-point range calculations in GCC */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test_fixed.c */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-style usage */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct mixing different fixed-point types */
struct MixedFixed {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum ssata;
};

/* Array initialized with fixed-point constants at extremes */
static const struct MixedFixed fixed_array[] = {
    /* First element - near maximum values */
    {
        .usf = 0.999999hr,  /* Very close to max unsigned short fract */
        .sf = 0.999999r,    /* Very close to max signed fract */
        .ua = 255.999999k,  /* Near max for 8-bit integer part */
        .sla = 32767.999999999lk, /* Near max for 16-bit integer part */
        .usatf = 0.999999r,
        .ssata = 127.999999k
    },
    /* Second element - near minimum values */
    {
        .usf = 0.000001hr,  /* Very close to min */
        .sf = -0.999999r,   /* Very close to min signed */
        .ua = 0.000001k,    /* Very close to min */
        .sla = -32768.000000001lk, /* Very close to min */
        .usatf = 0.000001r,
        .ssata = -128.000001k
    }
};

/* Function to trigger range checks through conversions */
static int convert_and_check(unsigned _Accum val) {
    /* These conversions will trigger range checking */
    int as_int = (int)val;
    float as_float = (float)val;
    signed _Accum as_signed = (signed _Accum)val;
    
    /* Conditional based on fixed-point comparison */
    return (val > 127.5k) ? as_int : (int)(as_float * 100);
}

/* Main test function */
int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Test 1: Direct extreme value assignments */
    unsigned _Fract max_uf = 0.999999r;
    unsigned _Fract min_uf = 0.000001r;
    signed _Fract max_sf = 0.999999r;
    signed _Fract min_sf = -0.999999r;
    unsigned _Accum max_ua = 255.999999k;
    signed _Accum min_sa = -128.000001k;
    
    /* Test 2: Saturation arithmetic that should overflow/underflow */
    unsigned _Sat _Fract sat1 = 0.75r;
    unsigned _Sat _Fract sat2 = 0.5r;
    unsigned _Sat _Fract sat_sum = sat1 + sat2; /* Should saturate to 0.999999r */
    
    signed _Sat _Accum sata1 = 100.0k;
    signed _Sat _Accum sata2 = 50.0k;
    signed _Sat _Accum sata_sum = sata1 + sata2; /* May trigger range check */
    
    /* Test 3: Compile-time constant expressions */
    #if 1  /* Always true, forces compilation of this block */
    {
        /* These should be evaluated at compile-time */
        static const unsigned _Fract compile_time_fract = 
            EVAL_CONST(0.999999r / 2.0r);
        
        static const signed long _Accum compile_time_accum = 
            EVAL_CONST(16384.5lk * 2.0lk);
            
        /* This multiplication might overflow in non-sat types */
        static const unsigned _Accum maybe_overflow = 
            EVAL_CONST(200.0k * 2.0k);
    }
    #endif
    
    /* Test 4: Loop with fixed-point operations */
    for (int i = 0; i < 3; i++) {  /* Small, unrollable loop */
        /* Vary values based on loop iteration */
        unsigned _Accum loop_val;
        switch (i) {
            case 0: loop_val = 0.0k; break;
            case 1: loop_val = 128.0k; break;
            case 2: loop_val = 255.999999k; break;  /* Max value */
        }
        
        /* Conditional assignment based on fixed-point comparison */
        signed _Accum conditional_val;
        if (loop_val > 127.5k) {
            /* This branch should be taken for i=2, possibly i=1 */
            conditional_val = (signed _Accum)(loop_val - 128.0k);
        } else {
            conditional_val = (signed _Accum)loop_val;
        }
        
        /* Convert to integer and accumulate */
        result += (int)(conditional_val * 1000k);
    }
    
    /* Test 5: Array access with fixed-point index calculation */
    {
        /* Fixed-point index calculation */
        unsigned _Fract index_frac = 0.5r;
        /* Convert to array index - triggers conversion logic */
        int array_index = (int)(index_frac * 2.0r);
        
        if (array_index < 2) {
            /* Access the pre-initialized array */
            result += (int)(fixed_array[array_index].ua * 100k);
        }
    }
    
    /* Test 6: Explicit overflow cases */
    {
        /* These operations should trigger range checks */
        unsigned _Accum overflow_test = 200.0k;
        unsigned _Accum multiplied = overflow_test * 2.0k;  /* Would overflow to 400 > 255.999999 */
        
        signed _Accum underflow_test = -100.0k;
        signed _Accum subtracted = underflow_test - 50.0k;  /* Would underflow to -150 < -128 */
        
        /* Convert results to prevent elimination */
        result += (int)multiplied;
        result += (int)subtracted;
    }
    
    /* Test 7: Mixed-type expressions */
    {
        unsigned short _Fract ushf = 0.5hr;
        signed _Fract sf = -0.25r;
        unsigned _Accum uacc = 100.5k;
        
        /* Complex expression mixing types */
        signed long _Accum mixed = 
            (signed long _Accum)ushf * 10000lk +
            (signed long _Accum)sf * 5000lk +
            (signed long _Accum)uacc;
            
        result += (int)(mixed / 10lk);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
