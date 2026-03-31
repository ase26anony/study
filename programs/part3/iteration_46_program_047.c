/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float __attribute__((noinline)) get_float_input(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    /* Create side effect */
    static int accumulator = 0;
    accumulator += val;
    external_double = accumulator;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize scalar floating-point values */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = get_double_input() + 1.0;
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_float_input();
    float f2 = get_float_input() + 1.0f;
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED/ORDERED cases - using fast-math assumptions */
    if (vd1 != vd2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (d1 < d2) {     /* May generate UNLT with -ffast-math */
        checksum += 2;
    }
    
    if (f1 > f2) {     /* May generate UNGT with -ffast-math */
        checksum += 4;
    }
    
    /* UNLE case */
    if (vd1 <= vd2) {  /* May generate UNLE */
        checksum += 8;
    }
    
    /* UNGE case */
    if (vf1 >= vf2) {  /* May generate UNGE */
        checksum += 16;
    }
    
    /* LTGT case (not equal and ordered) */
    if (d1 != d2 && d1 == d1 && d2 == d2) {  /* Explicit check for LTGT */
        checksum += 32;
    }
    
    /* ===== BUILTIN FUNCTIONS FOR EXPLICIT CONDITION CODES ===== */
    
    /* Direct unordered check - should generate UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* Direct ordered check - should generate ORDERED */
    if (__builtin_isordered(f1, f2)) {
        checksum += 128;
    }
    
    /* LTGT builtin */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 256;
    }
    
    /* UNEQ builtin equivalent */
    if (!__builtin_islessgreater(d1, d2) && !__builtin_isunordered(d1, d2)) {
        checksum += 512;
    }
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* GCC vector extension comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to prevent optimization */
    int vec_mask = __builtin_ia32_movmskps((__m128)cmp_eq) +
                   __builtin_ia32_movmskps((__m128)cmp_neq) +
                   __builtin_ia32_movmskps((__m128)cmp_lt) +
                   __builtin_ia32_movmskps((__m128)cmp_gt);
    checksum += vec_mask;
    
    /* Intrinsic-based unordered comparison */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 unord = _mm_cmpunord_ps(a, b);  /* Explicit UNORDERED */
    int unord_mask = _mm_movemask_ps(unord);
    checksum += unord_mask * 2;
    
    /* Double precision vector */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_d_eq = (vec_da == vec_db);
    v2df cmp_d_neq = (vec_da != vec_db);
    
    /* ===== CONDITIONAL MOVES BASED ON FP COMPARISONS ===== */
    
    /* Conditional move using ?: operator */
    double cond_result = (vd1 < vd2) ? vd1 : vd2;      /* May use UNLT */
    float cond_result_f = (f1 >= f2) ? f1 : f2;        /* May use UNGE */
    
    /* More complex conditional expressions */
    for (int i = 0; i < 4; i++) {
        double loop_val = get_double_input();
        if (loop_val != 0.0 && i < 3) {  /* May use UNEQ/LTGT in loop */
            checksum += i * 10;
        }
    }
    
    /* Switch with FP comparison results */
    int fp_case = 0;
    if (d1 == d2) fp_case = 1;        /* UNEQ */
    else if (d1 < d2) fp_case = 2;    /* UNLT */
    else if (d1 > d2) fp_case = 3;    /* UNGT */
    else if (__builtin_isunordered(d1, d2)) fp_case = 4; /* UNORDERED */
    
    switch (fp_case) {
        case 1: checksum += 1000; break;
        case 2: checksum += 2000; break;
        case 3: checksum += 3000; break;
        case 4: checksum += 4000; break;
    }
    
    /* ===== MIXED INTEGER/FLOAT COMPARISONS ===== */
    
    /* Compare float with integer */
    int int_val = 5;
    if (f1 < int_val) {          /* May generate UNLT */
        checksum += 10000;
    }
    
    if (vd1 >= int_val) {        /* May generate UNGE */
        checksum += 20000;
    }
    
    /* ===== PREVENT DEAD CODE ELIMINATION ===== */
    
    /* Use all results to create observable output */
    use_result(checksum);
    use_result((int)cond_result);
    use_result((int)cond_result_f);
    use_result(unord_mask);
    
    /* Return checksum based on all comparisons */
    printf("Condition code test checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-constant result */
}

/* Define external volatiles */
volatile double external_double = 0.0;
volatile float external_float = 0.0f;
