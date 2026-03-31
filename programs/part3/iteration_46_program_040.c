/* test_condcodes.c - Target x86 condition code mnemonics with -ffast-math */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return external_double;
}

float __attribute__((noinline)) get_float_input(void) {
    return external_float;
}

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    /* Create side effect */
    static volatile int sink;
    sink = val;
}

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize FP variables from mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = 3.14159;
    float f1 = get_float_input();
    float f2 = 2.71828f;
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    /* These should generate various condition codes with -ffast-math */
    
    /* UNORDERED / UNEQ patterns */
    if (d1 != d2) {           /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (!(vd1 == vd2)) {      /* Another way to get UNEQ */
        checksum += 2;
    }
    
    /* UNLE / UNLT patterns */
    if (f1 <= f2) {           /* May generate UNLE */
        checksum += 4;
    }
    
    if (vd1 < vd2) {          /* May generate UNLT */
        checksum += 8;
    }
    
    /* UNGE / UNGT patterns */
    if (d1 >= d2) {           /* May generate UNGE */
        checksum += 16;
    }
    
    if (f1 > f2) {            /* May generate UNGT */
        checksum += 32;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    /* Directly map to specific condition codes */
    
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* LTGT condition */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* ORDERED condition (inverse of UNORDERED) */
    if (__builtin_isordered(vd1, vd2)) {
        checksum += 256;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    /* Force materialization of condition codes */
    double cmov_result;
    
    /* UNGE in conditional move */
    cmov_result = (d1 >= d2) ? d1 : d2;
    checksum += (int)cmov_result;
    
    /* UNLE in conditional move */
    float f_cmov = (f1 <= f2) ? f1 : f2;
    checksum += (int)f_cmov;
    
    /* 5. Loop with FP comparison condition */
    /* Prevents dead code elimination */
    double arr[4] = {1.0, 2.0, 3.0, 4.0};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {
        checksum += i * 10;
    }
    
    /* 6. Vector (SIMD) comparisons */
    /* Using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);   /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);    /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);    /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);   /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);   /* May use UNGE */
    v4sf cmp_ne = (vec_a != vec_b);   /* May use LTGT */
    
    /* Extract results to affect checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_ptr[i] != 0.0f) ? 1 : 0;
    }
    
    /* Using x86 intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* UNORDERED comparison intrinsic */
    __m128 unord_mask = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Check if any element is unordered */
    int unord_test = _mm_movemask_ps(unord_mask);
    if (unord_test) {
        checksum += 512;
    }
    
    /* 7. Switch statement with FP comparisons */
    /* Creates multiple emission sites */
    int case_selector = checksum % 4;
    
    switch (case_selector) {
        case 0:
            if (d1 == d2) checksum += 1024;
            break;
        case 1:
            if (d1 < d2) checksum += 2048;
            break;
        case 2:
            if (d1 > d2) checksum += 4096;
            break;
        case 3:
            if (d1 != d2) checksum += 8192;
            break;
    }
    
    /* 8. Mixed integer/float comparisons */
    /* Can generate different RTL patterns */
    int int_val = checksum;
    if ((float)int_val <= f1) {
        checksum += 16384;
    }
    
    if (d1 >= (double)int_val) {
        checksum += 32768;
    }
    
    /* 9. Complex expression with multiple comparisons */
    /* Prevents simple optimization */
    double complex_result = (d1 < d2) ? 
                           ((f1 > f2) ? d1 : d2) : 
                           ((vd1 == vd2) ? f1 : f2);
    checksum += (int)complex_result;
    
    /* Final output to prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
