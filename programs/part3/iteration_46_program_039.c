/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent optimization of inputs */
extern float get_float(void) __attribute__((noinline));
extern double get_double(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global volatile variables to prevent constant folding */
volatile float volatile_float = 1.5f;
volatile double volatile_double = 2.5;

/* Opaque function to get inputs */
float get_float(void) {
    static float counter = 0.0f;
    return counter += 1.0f;
}

double get_double(void) {
    static double counter = 0.0;
    return counter += 1.0;
}

void use_result(int val) {
    /* Create side effect to prevent dead code elimination */
    static int accumulator = 0;
    accumulator += val;
}

int main(void) {
    int checksum = 0;
    
    /* 1. SCALAR FLOATING-POINT COMPARISONS WITH FAST-MATH ASSUMPTIONS */
    
    /* Get dynamic values */
    float f1 = get_float();
    float f2 = get_float() + 0.5f;
    double d1 = get_double();
    double d2 = get_double() - 0.25;
    
    /* Mix with volatile and constants */
    float f3 = volatile_float;
    double d3 = volatile_double;
    
    /* UNORDERED/ORDERED comparisons */
    if (f1 != f2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (f1 == f3) {  /* May generate UNEQ */
        checksum += 2;
    }
    
    /* UNGE: "nlt" (not less than) */
    if (d1 >= d2) {
        checksum += 4;
    }
    
    /* UNGT: "nle" (not less than or equal) */
    if (d1 > d3) {
        checksum += 8;
    }
    
    /* UNLE: "ule" (unordered or less than or equal) */
    if (f2 <= f3) {
        checksum += 16;
    }
    
    /* UNLT: "ult" (unordered or less than) */
    if (f1 < f2) {
        checksum += 32;
    }
    
    /* LTGT: "une" (unordered or not equal) */
    if (d1 != d3) {
        checksum += 64;
    }
    
    /* 2. EXPLICIT BUILTIN FUNCTIONS FOR UNORDERED CHECKS */
    
    /* Direct unordered check - should generate UNORDERED condition */
    if (__builtin_isunordered(f1, f2)) {
        checksum += 128;
    }
    
    /* Ordered check - should generate ORDERED condition */
    if (__builtin_isordered(d1, d2)) {
        checksum += 256;
    }
    
    /* Less-greater check - should generate LTGT condition */
    if (__builtin_islessgreater(f3, f1)) {
        checksum += 512;
    }
    
    /* 3. VECTOR (SIMD) COMPARISONS */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, NAN, 3.0f, 4.0f};  /* Include NaN for unordered */
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp1 = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp2 = (vec_a > vec_b);   /* May use UNGT */
    v4sf cmp3 = (vec_a <= vec_b);  /* May use UNLE */
    v4sf cmp4 = (vec_a < vec_b);   /* May use UNLT */
    
    /* Extract results to scalar checksum */
    float* cmp1_ptr = (float*)&cmp1;
    for (int i = 0; i < 4; i++) {
        if (cmp1_ptr[i] != 0.0f) checksum += 1024;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct unordered */
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(sse_cmp);
    if (mask != 0) {
        checksum += 2048;
    }
    
    /* Double precision vector */
    v2df vec_d1 = {d1, d2};
    v2df vec_d2 = {d2, d1};
    v2df cmp_d = (vec_d1 != vec_d2);  /* May generate LTGT */
    
    double* cmp_d_ptr = (double*)&cmp_d;
    if (cmp_d_ptr[0] != 0.0) checksum += 4096;
    
    /* 4. CONDITIONAL MOVES BASED ON FP COMPARISONS */
    
    /* Conditional move with floating-point result */
    double cond_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)(cond_result * 100);
    
    float cond_float = (f1 < f2) ? f1 : f2;  /* May use UNLT */
    checksum += (int)(cond_float * 10);
    
    /* Complex conditional expression */
    double complex_cond = (f1 != f2 && d1 > d3) ? 1.0 : 0.0;
    checksum += (int)(complex_cond * 1000);
    
    /* 5. LOOP WITH FP CONDITION */
    
    float array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = get_float();
    }
    
    /* Loop condition with FP comparison */
    for (int i = 0; i < 10 && (array[i] != 0.0f); i++) {
        checksum += i;
    }
    
    /* 6. SWITCH-LIKE BEHAVIOR WITH FP COMPARISONS */
    
    double test_val = get_double();
    int branch_taken = 0;
    
    if (test_val == d1) branch_taken = 1;      /* UNEQ */
    else if (test_val < d2) branch_taken = 2;  /* UNLT */
    else if (test_val >= d3) branch_taken = 3; /* UNGE */
    else if (test_val > d1) branch_taken = 4;  /* UNGT */
    else if (test_val <= d2) branch_taken = 5; /* UNLE */
    
    checksum += branch_taken * 10000;
    
    /* 7. MIXED INTEGER/FLOAT COMPARISONS */
    
    int int_val = 5;
    if (f1 > int_val) checksum += 100000;      /* May generate different pattern */
    if (int_val <= d1) checksum += 200000;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;
}
