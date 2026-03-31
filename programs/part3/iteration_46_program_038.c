/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

/* Dummy function to prevent optimization */
void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

void use_result(int val) {
    /* Create side effect */
    static volatile int sink;
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* 1. Standard floating-point comparisons with -ffast-math */
    
    /* UNORDERED: May generate "unord" */
    if (d1 != d1) { /* NaN check */
        checksum |= 1;
    }
    
    /* UNEQ: May generate "ueq" (unordered or equal) */
    if (d1 == d2) {
        checksum |= 2;
    }
    
    /* UNGE: May generate "nlt" (not less than) */
    if (vd1 >= d2) {
        checksum |= 4;
    }
    
    /* UNGT: May generate "nle" (not less than or equal) */
    if (d1 > d2) {
        checksum |= 8;
    }
    
    /* UNLE: May generate "ule" (unordered or less than or equal) */
    if (f1 <= f2) {
        checksum |= 16;
    }
    
    /* UNLT: May generate "ult" (unordered or less than) */
    if (f1 < f2) {
        checksum |= 32;
    }
    
    /* LTGT: May generate "une" (unordered or not equal) */
    if (d1 != d2) {
        checksum |= 64;
    }
    
    /* ORDERED: May generate "ord" */
    if (d1 == d1 && d2 == d2) { /* Both not NaN */
        checksum |= 128;
    }
    
    /* 2. Built-in functions for explicit condition codes */
    
    /* Direct unordered check */
    if (__builtin_isunordered(d1, d2)) {
        checksum |= 256;
    }
    
    /* Less-greater (LTGT) */
    if (__builtin_islessgreater(f1, f2)) {
        checksum |= 512;
    }
    
    /* Ordered comparison */
    if (__builtin_islessequal(d1, d2)) { /* May use UNLE */
        checksum |= 1024;
    }
    
    /* 3. Vector (SIMD) comparisons */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {f1, f2, f1, f2};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to affect checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 2048;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b); /* Direct unordered */
    
    /* 4. Conditional moves based on FP comparisons */
    
    /* Conditional move with double */
    double cond_result = (d1 >= d2) ? d1 : d2; /* May use UNGE */
    checksum += (int)(cond_result * 100);
    
    /* Conditional move with float */
    float f_cond = (f1 != f2) ? f1 : f2; /* May use LTGT */
    checksum += (int)(f_cond * 100);
    
    /* 5. Loop with floating-point condition */
    
    double arr[4] = {d1, d2, vd1, vd2};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) { /* May use LTGT */
        checksum += i * 10;
    }
    
    /* Switch based on comparison results */
    int cmp_case = 0;
    if (d1 < d2) cmp_case = 1;   /* UNLT */
    else if (d1 > d2) cmp_case = 2; /* UNGT */
    else if (d1 == d2) cmp_case = 3; /* UNEQ */
    else cmp_case = 4; /* UNORDERED */
    
    switch (cmp_case) {
        case 1: checksum += 1000; break;
        case 2: checksum += 2000; break;
        case 3: checksum += 3000; break;
        case 4: checksum += 4000; break;
    }
    
    /* 6. Mixed integer/float comparisons */
    
    int int_val = 5;
    if (f1 < int_val) { /* Float-int comparison */
        checksum += 5000;
    }
    
    if (d1 > int_val) { /* Double-int comparison */
        checksum += 6000;
    }
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    /* Output checksum to ensure all code paths contribute */
    printf("Checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
