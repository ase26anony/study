/* Test program for fixed-point range calculation logic in fixed-value.cc
 * Targets uncovered lines 264-277 in GCC's fixed-point implementation
 */

/* Force C23 mode for standard fixed-point support */
#if __STDC_VERSION__ < 202311L
#error "Requires C23 or GCC with fixed-point extensions"
#endif

#include <stdio.h>

/* Use volatile to prevent dead code elimination */
static volatile int output;

int main(void) {
    /* ====== Requirement 1: Fixed-point types at limits ====== */
    /* Unsigned fract at maximum representable value */
    unsigned _Fract uf_max = 0.999999r;
    
    /* Signed accum near minimum */
    _Sat _Accum sa_min = -0.999999999k;
    
    /* Long accum at extremes */
    long _Sat _Accum lsa_max = 0.999999999999999lk;
    long _Sat _Accum lsa_min = -0.999999999999999lk;
    
    /* Short fract */
    short _Fract sf_mid = 0.5hr;
    
    /* ====== Requirement 2: Constant folding with ternary ====== */
    static const unsigned _Fract const_uf = 0.75r;
    static const _Accum const_acc = 0.5k;
    
    /* Compile-time evaluation with ternary */
    const _Accum folded = (const_uf > 0.5r) ? (const_acc * 2.0k) : (const_acc / 2.0k);
    
    /* ====== Requirement 3: Saturation arithmetic ====== */
    unsigned _Sat _Fract usf1 = 0.8r;
    unsigned _Sat _Fract usf2 = 0.9r;
    
    /* This will saturate to 1.0r */
    unsigned _Sat _Fract usf_sum = usf1 + usf2;
    
    /* Underflow test */
    unsigned _Sat _Fract usf3 = 0.1r;
    unsigned _Sat _Fract usf_sub = usf3 - 0.5r;  /* Should saturate to 0.0r */
    
    /* ====== Requirement 4: Mixed-type conversions ====== */
    /* Fixed-point to integer */
    int int_from_acc = (_Accum)0.999k;
    
    /* Fixed-point to float */
    float float_from_fract = (float)uf_max;
    
    /* Integer to fixed-point with overflow potential */
    _Sat _Accum acc_from_int = (_Sat _Accum)500;  /* May overflow */
    
    /* ====== Requirement 5: Compile-time conditional blocks ====== */
    #if __has_builtin(__builtin_constant_p)
    if (__builtin_constant_p(const_uf)) {
        /* Array indexing with fixed-point conversion */
        int array[4] = {0};
        int idx = (int)(const_uf * 4.0r);
        if (idx >= 0 && idx < 4) {
            output = array[idx];
        }
    }
    #endif
    
    /* ====== Requirement 6: Aggregate initializers ====== */
    struct MixedFixed {
        short _Fract sf;
        _Accum acc;
        unsigned _Sat _Fract usf;
        long _Accum lacc;
    };
    
    struct MixedFixed agg = {
        .sf = 0.75hr,
        .acc = -0.25k,
        .usf = 0.999r,  /* Near maximum */
        .lacc = 0.999999999lk
    };
    
    /* Array with fixed-point values */
    _Accum acc_array[3] = {0.1k, 0.5k, 0.9k};
    
    /* ====== Execution flow with loop ====== */
    _Accum loop_acc = 0.0k;
    
    /* Small fixed loop for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (loop_acc > 0.5k) {
            loop_acc = loop_acc * 0.5k;  /* Multiplication may underflow */
        } else {
            loop_acc = loop_acc + 0.3k;  /* Addition may overflow */
        }
        
        /* Mix with array values */
        loop_acc = loop_acc + acc_array[i];
    }
    
    /* Additional overflow/underflow triggers */
    
    /* 1. Left shift simulation through multiplication */
    _Sat _Accum shift_test = 0.9k;
    for (int i = 0; i < 5; i++) {
        shift_test = shift_test * 2.0k;  /* Will saturate */
    }
    
    /* 2. Extreme value comparisons */
    _Accum extreme = 0.999999999999k;  /* Very close to 1.0 */
    int is_max = (extreme >= 1.0k);  /* Should be false but tests comparison */
    
    /* 3. Conversion chain that stresses range checking */
    long _Accum big_val = 0.999999999999999lk;
    _Accum down_convert = (_Accum)big_val;  /* Conversion with potential loss */
    short _Fract small_convert = (short _Fract)down_convert;
    
    /* 4. Direct overflow through addition */
    _Sat _Accum overflow_test = 0.9k;
    overflow_test = overflow_test + 0.2k;  /* Should be 1.1k but saturates to 1.0k */
    
    /* 5. Underflow through subtraction */
    _Sat _Accum underflow_test = -0.9k;
    underflow_test = underflow_test - 0.2k;  /* Should be -1.1k but saturates to -1.0k */
    
    /* Prevent dead code elimination */
    output = (int)(usf_sum * 100.0r) + 
             (int)(loop_acc * 100.0k) + 
             (int)(shift_test * 100.0k) +
             is_max +
             (int)(small_convert * 100.0hr);
    
    printf("Output: %d\n", output);
    
    return 0;
}
