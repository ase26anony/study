/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original -fdump-tree-optimized fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Force constant folding with static const */
static const unsigned short _Fract max_ufract = 0.999999ur;
static const signed short _Fract min_sfract = -0.999999r;
static const unsigned _Sat _Fract sat_ufract = 0.75ur;
static const signed long _Accum max_laccum = 9223372036854775.807lk;
static const signed long _Accum min_laccum = -9223372036854775.808lk;

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Accum usa;
    signed long _Sat _Accum slsa;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData fp_array[] = {
    {0.999999ur, -0.5r, 0.99999999999999999999uka, 9223372036854775.807lk},
    {0.5ur, 0.75r, 0.5uka, -9223372036854775.808lk},
    {0.0ur, 0.0r, 0.0uka, 0.0lk}
};

/* Function to trigger range calculations through conversions */
int convert_fixed_to_int(signed _Accum a) {
    /* This conversion should trigger range checking */
    return (int)a;
}

float convert_fixed_to_float(unsigned _Fract f) {
    /* Another conversion that needs range analysis */
    return (float)f;
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Test extreme values that should trigger max/min calculations */
    signed _Accum test_accum = 9223372036854775.807k; /* Near max */
    signed _Accum test_accum2 = -9223372036854775.808k; /* Near min */
    
    /* 2. Use ternary operator with constant condition for compile-time evaluation */
    const int is_constant = 1;
    unsigned _Sat _Fract sat_result = is_constant ? 
        (sat_ufract + 0.5ur) : /* This overflows to 1.0ur at compile time */
        0.0ur;
    
    /* 3. Complex expression with mixed types */
    signed long _Accum mixed_expr = (signed long _Accum)max_ufract * 10000.0lk;
    
    /* 4. Loop with fixed iterations for unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        signed _Fract loop_var;
        if (fp_array[i].sf > 0.0r) {
            loop_var = fp_array[i].sf * 0.999999r; /* May overflow */
        } else {
            loop_var = fp_array[i].sf - 0.999999r; /* May underflow */
        }
        
        /* Cast to integer and accumulate */
        result += convert_fixed_to_int(loop_var);
        
        /* Test saturation arithmetic */
        unsigned _Sat _Fract sat_test = fp_array[i].usf + 0.75ur;
        result += (int)(sat_test * 100.0ur);
    }
    
    /* 5. Test overflow/underflow with saturation types */
    unsigned _Sat _Accum sat_overflow = 0.99999999999999999999uka;
    sat_overflow = sat_overflow + 0.00000000000000000001uka; /* Should saturate to max */
    
    signed long _Sat _Accum sat_underflow = -9223372036854775.808lk;
    sat_underflow = sat_underflow - 0.001lk; /* Should saturate to min */
    
    /* 6. Use __builtin_constant_p to create constant-only code paths */
    #ifdef __GNUC__
    if (__builtin_constant_p(max_ufract)) {
        /* This code only runs if max_ufract is a compile-time constant */
        unsigned _Fract const_test = max_ufract;
        /* Array indexing with fixed-point conversion */
        int index = (int)(const_test * 10.0ur);
        if (index >= 0 && index < 10) {
            result += index;
        }
    }
    #endif
    
    /* 7. Test shift operations (GCC extension for fixed-point) */
    signed _Accum shifted = 0.5k;
    /* Simulate left shift through multiplication by power of 2 */
    shifted = shifted * 2.0k; /* Equivalent to << 1 */
    shifted = shifted * 4.0k; /* Equivalent to << 2 */
    
    /* 8. Force evaluation of boundary cases through comparisons */
    int cmp1 = (test_accum == 9223372036854775.807k);
    int cmp2 = (test_accum2 == -9223372036854775.808k);
    int cmp3 = (max_ufract >= 0.999999ur);
    
    result += cmp1 + cmp2 + cmp3;
    
    /* 9. Mixed initialization with designated initializers */
    struct FixedPointData local_data = {
        .usf = 0.999999ur,
        .sf = -0.999999r,
        .usa = 0.99999999999999999999uka,
        .slsa = (signed long _Sat _Accum)0.5k * 18446744073709551615.0lk
    };
    
    /* 10. Final conversions that should trigger range checks */
    float float_result = convert_fixed_to_float(local_data.usf);
    int int_result = convert_fixed_to_int(local_data.sf);
    
    /* Use results to prevent optimization */
    result += (int)float_result + int_result;
    
    printf("Result: %d\n", result);
    
    return 0;
}
