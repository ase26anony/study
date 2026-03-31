/* Target: fixed-value.cc uncovered lines 264-277 */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test.c */

#include <stdio.h>

/* Fixed-point type declarations covering various modes */
typedef short _Fract sf;
typedef _Fract f;
typedef long _Fract lf;
typedef short _Accum sa;
typedef _Accum a;
typedef long _Accum la;
typedef unsigned short _Fract usf;
typedef unsigned _Fract uf;
typedef unsigned long _Fract ulf;
typedef unsigned short _Accum usa;
typedef unsigned _Accum ua;
typedef unsigned long _Accum ula;

/* Saturated versions */
typedef _Sat short _Fract ssf;
typedef _Sat _Fract sfx;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum sax;
typedef _Sat long _Accum sla;
typedef _Sat unsigned short _Fract susf;
typedef _Sat unsigned _Fract suf;
typedef _Sat unsigned long _Accum sula;

/* Force compile-time evaluation with constexpr */
static constexpr sf MAX_SF = 0.999999r;  /* Near max of short _Fract */
static constexpr sf MIN_SF = -0.999999r; /* Near min of short _Fract */
static constexpr uf MAX_UF = 0.999999r;  /* Near max of unsigned _Fract */
static constexpr a MAX_A = 32767.999999k; /* Near max of _Accum */
static constexpr a MIN_A = -32768.999999k; /* Near min of _Accum */

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    sf short_fract;
    f fract;
    lf long_fract;
    sa short_accum;
    a accum;
    la long_accum;
    usf ushort_fract;
    uf ufract;
    ula ulong_accum;
};

/* Array initialized with fixed-point constants */
static const struct FixedPointStruct fp_array[] = {
    {0.5r, 0.25r, 0.125lr, 1.5hk, 2.5k, 3.5lk, 0.1ur, 0.2ur, 4.5ulk},
    {0.75r, 0.875r, 0.9375lr, 7.5hk, 15.5k, 31.5lk, 0.5ur, 0.75ur, 63.5ulk},
    {MAX_SF, 0.999999r, 0.999999lr, 127.999999hk, 32767.999999k, 
     2147483647.999999lk, MAX_UF, 0.999999ur, 4294967295.999999ulk}
};

/* Function to trigger range checks through conversions */
static int convert_and_check(f value) {
    /* These conversions will trigger range checking */
    int as_int = (int)value;
    float as_float = (float)value;
    
    /* Use __builtin_constant_p to create conditional compilation */
#if __GNUC__ >= 5
    if (__builtin_constant_p(value > 0.5r)) {
        /* This path only taken for constants */
        return as_int + (value > 0.5r ? 100 : 200);
    }
#endif
    
    return as_int;
}

/* Main function with operations targeting uncovered logic */
int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Saturation arithmetic that will overflow/underflow */
    ssf saturated_sf = 0.9r;
    ssf saturated_sf2 = 0.8r;
    ssf saturated_sum = saturated_sf + saturated_sf2; /* Should saturate to ~1.0r */
    
    /* 2. Operations near boundaries */
    a accum1 = MAX_A;
    a accum2 = 0.000001k;
    a accum_sum = accum1 + accum2; /* May trigger overflow check */
    
    a accum3 = MIN_A;
    a accum_diff = accum3 - accum2; /* May trigger underflow check */
    
    /* 3. Multiplication that can overflow */
    f fract1 = 0.9r;
    f fract2 = 0.9r;
    f fract_prod = fract1 * fract2;
    
    /* 4. Loop with fixed-point operations */
    for (int i = 0; i < 3; i++) {
        /* Use array values in calculations */
        sf current = fp_array[i].short_fract;
        
        /* Conditional based on fixed-point comparison */
        if (current > 0.5r) {
            /* Scale up - may overflow */
            current = current * 1.5r;
        } else {
            /* Scale down */
            current = current * 0.5r;
        }
        
        /* Convert to integer for output */
        int int_val = (int)(current * 1000);
        result += int_val;
        
        /* Use ternary operator with constant condition */
        f test_val = (i == 1) ? 0.666667r : 0.333333r;
        int_val = convert_and_check(test_val);
        result += int_val;
    }
    
    /* 5. Test extreme values with shifts (simulated via multiplication) */
    usa unsigned_accum = 65535.999999uhk;
    /* Shift left by multiplying by power of 2 */
    usa shifted = unsigned_accum * 2.0uhk; /* Will overflow for saturating types */
    
    /* 6. Mixed-type expressions */
    la long_accum = 1000.5lk;
    f fract_val = 0.75r;
    /* Convert fract to accum and add */
    la mixed_sum = long_accum + (la)(fract_val * 1000.0r);
    
    /* 7. Compile-time constant expression */
    constexpr f const_fract = 0.123456r;
    constexpr int array_index = (int)(const_fract * 10);
    
    /* Prevent unused variable warnings */
    (void)saturated_sum;
    (void)accum_sum;
    (void)accum_diff;
    (void)fract_prod;
    (void)shifted;
    (void)mixed_sum;
    
    /* Use array_index to prevent elimination */
    result += array_index;
    
    printf("Result: %d\n", result);
    
    return 0;
}

/* Additional compile-time tests using preprocessor */
#if defined(__GNUC__) && __GNUC__ >= 5
/* This section only compiled if __builtin_constant_p works */
static const f compile_time_fract = 0.999999r;
static const int ct_index = (int)(compile_time_fract * 100);
static int ct_array[ct_index > 50 ? 100 : 50];
#endif
