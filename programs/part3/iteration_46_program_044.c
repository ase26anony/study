/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.234567;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.234567f;
}

/* Dummy function to prevent optimization */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Create side effect */
    volatile static int sink = 0;
    sink += val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    double d3 = d1 + 3.14159;
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_input_float();
    float f2 = get_input_float();
    float f3 = f1 + 2.71828f;
    
    /* 2. Perform relational comparisons with -ffast-math assumptions */
    
    /* UNORDERED - generated when NaNs might be present but -ffast-math allows reordering */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    /* ORDERED - inverse of unordered */
    if (d1 == d3) {  /* May generate UNEQ or ORDERED */
        checksum += 2;
    }
    
    /* UNGE - unordered or greater than or equal */
    if (vd1 >= d2) {  /* May generate UNGE (nlt) */
        checksum += 4;
    }
    
    /* UNGT - unordered or greater than */
    if (d1 > vd2) {  /* May generate UNGT (nle) */
        checksum += 8;
    }
    
    /* UNLE - unordered or less than or equal */
    if (f1 <= vf2) {  /* May generate UNLE */
        checksum += 16;
    }
    
    /* UNLT - unordered or less than */
    if (vf1 < f2) {  /* May generate UNLT (ult) */
        checksum += 32;
    }
    
    /* LTGT - less than or greater than (unordered equal excluded) */
    if (d2 != d3) {  /* May generate LTGT (une) */
        checksum += 64;
    }
    
    /* Mixed float/double comparisons */
    if ((double)f1 > d2) {
        checksum += 128;
    }
    
    /* 3. Explicit built-in unordered checks */
    
    /* Direct UNORDERED test */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 256;
    }
    
    /* LTGT builtin */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 512;
    }
    
    /* Ordered comparison builtins */
    if (__builtin_islessequal(d1, d3)) {  /* May generate UNLE */
        checksum += 1024;
    }
    
    if (__builtin_isgreaterequal(f2, f3)) {  /* May generate UNGE */
        checksum += 2048;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cmov_result2 = (f1 != f2) ? f1 : f2;   /* May use UNEQ/LTGT */
    
    checksum += (int)(cmov_result1 + cmov_result2);
    
    /* 5. Vector (SIMD) comparisons */
    
    /* Initialize vector variables */
    v4sf vec_a = {f1, f2, f3, 4.0f};
    v4sf vec_b = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_c = {get_input_float(), get_input_float(), 
                   get_input_float(), get_input_float()};
    
    /* Vector comparisons - these often generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_c);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_c);      /* May use UNGE */
    
    /* Extract results to scalar checksum */
    float* eq_ptr = (float*)&cmp_eq;
    float* neq_ptr = (float*)&cmp_neq;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_ptr[i] != 0.0f) ? 1 : 0;
        checksum += (neq_ptr[i] != 0.0f) ? 2 : 0;
    }
    
    /* Double precision vectors */
    v2df vec_d1 = {d1, d2};
    v2df vec_d2 = {d2, d3};
    v2df vec_cmp = (vec_d1 > vec_d2);    /* May use UNGT */
    
    double* dbl_ptr = (double*)&vec_cmp;
    checksum += (dbl_ptr[0] != 0.0) ? 4096 : 0;
    checksum += (dbl_ptr[1] != 0.0) ? 8192 : 0;
    
    /* 6. SSE intrinsics for explicit unordered comparisons */
    __m128 sse_a = _mm_set_ps(f1, f2, f3, 4.0f);
    __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* _CMP_UNORD_Q - unordered comparison */
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* _CMP_NEQ_UQ - not equal unordered quiet (may use UNEQ/LTGT) */
    __m128 neq_cmp = _mm_cmpneq_ps(sse_a, sse_b);
    
    /* Extract SSE results */
    float unord_res[4];
    _mm_store_ps(unord_res, unord_cmp);
    for (int i = 0; i < 4; i++) {
        checksum += (unord_res[i] != 0.0f) ? 16384 : 0;
    }
    
    /* 7. Loop with FP condition to generate multiple condition codes */
    double loop_vals[4] = {d1, d2, d3, 0.0};
    for (int i = 0; i < 4 && loop_vals[i] != 0.0; i++) {  /* May use UNEQ/LTGT */
        checksum += i * 100;
        
        /* Nested comparisons */
        if (loop_vals[i] < 0.0) {        /* May use UNLT */
            checksum -= 50;
        } else if (loop_vals[i] >= 1.0) { /* May use UNGE */
            checksum += 50;
        }
    }
    
    /* 8. Switch based on comparison results */
    int cmp_case = 0;
    if (d1 == d2) cmp_case = 1;          /* May use UNEQ */
    else if (d1 > d2) cmp_case = 2;      /* May use UNGT */
    else if (d1 <= d2) cmp_case = 3;     /* May use UNLE */
    
    switch (cmp_case) {
        case 1: checksum += 1000; break;
        case 2: checksum += 2000; break;
        case 3: checksum += 3000; break;
        default: checksum += 4000; break;
    }
    
    /* 9. Complex expression mixing multiple comparisons */
    double complex_result = (d1 < d2) ? 
                           ((f1 > f2) ? d1 : d2) : 
                           ((d1 == d3) ? f1 : f2);
    checksum += (int)complex_result;
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    /* Output result to ensure all code paths are considered */
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-constant value */
}

/* Define volatile externs */
volatile double external_double = 3.141592653589793;
volatile float external_float = 2.718281828459045f;
