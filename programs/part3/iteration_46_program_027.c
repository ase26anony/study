/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double __attribute__((weak));
extern volatile float external_float __attribute__((weak));

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.234567;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 2.345678f;
}

/* Dummy function to create side-effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink = 0;
    sink = val;
}

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with mix of sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* 1. Standard floating-point comparisons with -ffast-math */
    
    /* UNORDERED: May generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* ORDERED: May generate "ord" */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* UNEQ: unordered or equal - may generate "ueq" */
    if (!(d1 < d2) && !(d1 > d2)) {  /* Equivalent to !(d1 < d2) && !(d1 > d2) */
        checksum += 4;
    }
    
    /* UNGE: not less than - may generate "nlt" */
    if (!(f1 < f2)) {
        checksum += 8;
    }
    
    /* UNGT: not less or equal - may generate "nle" */
    if (!(f1 <= f2)) {
        checksum += 16;
    }
    
    /* UNLE: unordered or less or equal - may generate "ule" */
    if (__builtin_islessequal(d1, d2)) {
        checksum += 32;
    }
    
    /* UNLT: unordered or less than - may generate "ult" */
    if (__builtin_isless(d1, d2)) {
        checksum += 64;
    }
    
    /* LTGT: less or greater (ordered and not equal) - may generate "une" */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* 2. More comparisons using relational operators */
    
    /* Mixed float/double comparisons */
    if ((double)f1 != d1) {
        checksum += 256;
    }
    
    /* Compare with integer */
    int i = 10;
    if (d1 <= i) {
        checksum += 512;
    }
    
    /* Conditional move based on FP comparison */
    double cmov_result = (vd1 >= vd2) ? vd1 : vd2;
    checksum += (int)cmov_result;
    
    /* 3. Vector (SIMD) comparisons */
    
    /* GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* eq_ptr = (float*)&cmp_eq;
    for (int j = 0; j < 4; j++) {
        if (eq_ptr[j] != 0.0f) checksum += 1024;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED test */
    
    /* Check unordered elements */
    float* cmp_ptr = (float*)&sse_cmp;
    for (int j = 0; j < 4; j++) {
        if (cmp_ptr[j] != 0.0f) checksum += 2048;
    }
    
    /* Double precision vector */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_d = (vec_da > vec_db);  /* May use UNGT */
    
    /* 4. Loop with FP condition to generate branch with condition code */
    volatile double arr[4] = {1.5, 2.5, 0.0, 3.5};
    for (int j = 0; j < 4 && (arr[j] != 0.0); j++) {
        checksum += j * 100;
    }
    
    /* 5. Switch based on FP comparison results */
    int fp_compare = 0;
    if (d1 < d2) fp_compare = 1;
    else if (d1 > d2) fp_compare = 2;
    else if (d1 == d2) fp_compare = 3;
    else fp_compare = 4;  /* unordered */
    
    switch (fp_compare) {
        case 1: checksum += 4096; break;
        case 2: checksum += 8192; break;
        case 3: checksum += 16384; break;
        case 4: checksum += 32768; break;  /* UNORDERED case */
    }
    
    /* 6. More builtin usage */
    checksum += __builtin_isunordered(f1, f2) ? 65536 : 0;
    checksum += __builtin_islessgreater(d1, d2) ? 131072 : 0;
    checksum += __builtin_islessequal(f1, f2) ? 262144 : 0;
    checksum += __builtin_isless(d1, d2) ? 524288 : 0;
    checksum += __builtin_isgreater(f1, f2) ? 1048576 : 0;
    checksum += __builtin_isgreaterequal(d1, d2) ? 2097152 : 0;
    
    /* Create observable output */
    printf("Condition code test checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum == 0 ? 1 : 0;  /* Non-zero return if all comparisons false */
}
