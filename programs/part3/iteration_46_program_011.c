/* test_condcodes.c - Target x86 condition code mnemonics for i386.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding and optimization */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static const double values[] = {1.0, 2.0, 0.0, -1.0, 3.14, -2.71};
    return values[idx % 6];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static const float values[] = {1.0f, 2.0f, 0.0f, -1.0f, 3.14f, -2.71f};
    return values[idx % 6];
}

/* Dummy function to create side effects */
void __attribute__((noinline)) use_result(int val) {
    /* Prevent dead code elimination */
    volatile static int sink;
    sink = val;
}

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize FP variables from mixed sources */
    volatile double vd1 = 1.5;
    volatile double vd2 = 2.5;
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    double d3 = get_double_input(2);
    
    volatile float vf1 = 1.5f;
    volatile float vf2 = 2.5f;
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    float f3 = get_float_input(2);
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    /* Using -ffast-math, these may generate UNORDERED/ORDERED condition codes */
    
    /* UNORDERED/UNEQ patterns */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (!(vf1 == vf2)) {  /* Another way to trigger UNEQ */
        checksum += 2;
    }
    
    /* UNLE pattern */
    if (d1 <= d3) {  /* May generate UNLE with -ffast-math */
        checksum += 4;
    }
    
    /* UNLT pattern */
    if (vd1 < vd2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    /* UNGE pattern */
    if (f1 >= f2) {  /* May generate UNGE (printed as "nlt") */
        checksum += 16;
    }
    
    /* UNGT pattern */
    if (vf1 > vf2) {  /* May generate UNGT (printed as "nle") */
        checksum += 32;
    }
    
    /* LTGT pattern (une) */
    if (d2 != d3 && d2 == d2 && d3 == d3) {  /* Explicit LTGT pattern */
        checksum += 64;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    /* Directly map to specific condition codes */
    
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 128;
    }
    
    /* ORDERED condition */
    if (__builtin_isordered(f1, f2)) {
        checksum += 256;
    }
    
    /* LTGT condition (builtin) */
    if (__builtin_islessgreater(vd1, vd2)) {
        checksum += 512;
    }
    
    /* UNLE condition (builtin) */
    if (__builtin_islessequal(d1, d3)) {
        checksum += 1024;
    }
    
    /* UNGE condition (builtin) */
    if (__builtin_isgreaterequal(f1, f2)) {
        checksum += 2048;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    /* Force materialization of condition codes */
    double cmov_result1 = (d1 < d2) ? d1 : d2;  /* May use UNLT */
    float cmov_result2 = (vf1 >= vf2) ? vf1 : vf2;  /* May use UNGE */
    checksum += (int)(cmov_result1 + cmov_result2);
    
    /* 5. Vector (SIMD) comparisons */
    /* Using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_c);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_c);    /* May use UNGE */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    
    /* Extract results to affect checksum */
    float* eq_results = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_results[i] != 0.0f) checksum += 1 << i;
    }
    
    /* Using x86 intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* _CMP_NEQ_UQ generates UNEQ condition */
    __m128 cmp_neq_uq = _mm_cmpneq_ps(sse_a, sse_b);
    
    /* Extract mask from comparison results */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_neq = _mm_movemask_ps(cmp_neq_uq);
    checksum += mask_unord + mask_neq;
    
    /* Double precision vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    v2df cmp_dbl_eq = (vec_da == vec_db);  /* May use UNEQ for doubles */
    
    double* dbl_results = (double*)&cmp_dbl_eq;
    if (dbl_results[0] != 0.0) checksum += 4096;
    if (dbl_results[1] != 0.0) checksum += 8192;
    
    /* 6. Loop with FP condition to generate multiple condition code sites */
    for (int i = 0; i < 10; i++) {
        double loop_val = get_double_input(i);
        /* Mix different comparisons in loop */
        if (loop_val != 0.0) {           /* May use UNEQ/LTGT */
            checksum += i;
        }
        if (loop_val <= 2.0) {           /* May use UNLE */
            checksum -= i;
        }
        if (!(loop_val < 1.0)) {         /* May use UNGE (nlt) */
            checksum |= 1 << (i % 16);
        }
    }
    
    /* 7. Switch based on FP comparison results (indirectly) */
    int fp_case = 0;
    if (d1 < d2) fp_case = 1;      /* UNLT */
    else if (d1 == d2) fp_case = 2; /* UNEQ */
    else if (d1 > d2) fp_case = 3;  /* UNGT */
    else if (__builtin_isunordered(d1, d2)) fp_case = 4; /* UNORDERED */
    
    switch (fp_case) {
        case 1: checksum |= 0x10000; break;
        case 2: checksum |= 0x20000; break;
        case 3: checksum |= 0x40000; break;
        case 4: checksum |= 0x80000; break;
    }
    
    /* Create observable output to prevent optimization */
    use_result(checksum);
    
    printf("Condition code test checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
