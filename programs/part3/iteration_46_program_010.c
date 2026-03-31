/* test_condcodes.c - Target x86 condition code coverage */
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

/* Dummy function to create side effects */
void __attribute__((noinline)) use_result(int val) {
    /* Prevent optimization */
    asm volatile("" : : "r"(val) : "memory");
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    /* Dynamic values from opaque functions */
    double d1 = get_double_input();
    double d2 = d1 + 1.0;
    float f1 = get_float_input();
    float f2 = f1 + 1.0f;
    
    /* ===== 1. Standard floating-point comparisons with -ffast-math ===== */
    
    /* UNORDERED - should generate "unord" */
    if (d1 != d1) { /* NaN check */
        checksum += 1;
    }
    
    /* ORDERED - should generate "ord" */
    if (d1 == d1) { /* Not NaN check */
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) {
        checksum += 4;
    }
    
    /* UNGE - unordered or greater than or equal (nlt) */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - unordered or greater than (nle) */
    if (vd1 > vd2) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less than or equal (ule) */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* UNLT - unordered or less than (ult) */
    if (f1 < f2) {
        checksum += 64;
    }
    
    /* LTGT - less than or greater than (une) */
    if (d1 != d2) {
        checksum += 128;
    }
    
    /* ===== 2. Built-in functions for explicit condition codes ===== */
    
    /* Direct unordered check */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 256;
    }
    
    /* Direct ordered check */
    if (__builtin_isordered(f1, f2)) {
        checksum += 512;
    }
    
    /* Less-greater (LTGT) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 1024;
    }
    
    /* ===== 3. Vector (SIMD) comparisons ===== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_vec = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_vec2 = (vec_a > vec_b);  /* May use UNGT */
    v4sf cmp_vec3 = (vec_a < vec_b);  /* May use UNLT */
    
    /* Extract results to prevent optimization */
    float cmp_result = cmp_vec[0] + cmp_vec[1] + cmp_vec[2] + cmp_vec[3];
    if (cmp_result > 0) {
        checksum += 2048;
    }
    
    /* Double vector comparisons */
    v2df cmp_dvec = (vec_da != vec_db);  /* May use LTGT */
    if (cmp_dvec[0] != 0.0 || cmp_dvec[1] != 0.0) {
        checksum += 4096;
    }
    
    /* ===== 4. Conditional moves based on FP comparisons ===== */
    
    /* Conditional move with double */
    double cond_d = (vd1 >= vd2) ? vd1 : vd2;  /* May use UNGE */
    checksum += (int)(cond_d * 10);
    
    /* Conditional move with float */
    float cond_f = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)(cond_f * 10);
    
    /* Complex conditional expression */
    double complex_cond = (d1 != d2) ? 
                         ((d1 > d2) ? d1 : d2) :  /* May use LTGT and UNGT */
                         ((d1 < d2) ? d1 : d2);   /* May use UNLT */
    checksum += (int)(complex_cond * 10);
    
    /* ===== 5. Loop with FP condition ===== */
    
    volatile float arr[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    for (int i = 0; i < 4 && (arr[i] != 0.0f); ++i) {  /* May use UNEQ */
        checksum += i * 100;
    }
    
    /* ===== 6. Switch based on FP comparison results ===== */
    
    int fp_switch = 0;
    if (d1 < d2) fp_switch = 1;      /* May use UNLT */
    else if (d1 > d2) fp_switch = 2; /* May use UNGT */
    else if (d1 == d2) fp_switch = 3;/* May use UNEQ */
    else fp_switch = 4;              /* May use UNORDERED */
    
    switch (fp_switch) {
        case 1: checksum += 8192; break;
        case 2: checksum += 16384; break;
        case 3: checksum += 32768; break;
        case 4: checksum += 65536; break;
    }
    
    /* ===== 7. Mixed integer/float comparisons ===== */
    
    int int_val = 5;
    if (vd1 < int_val) {  /* Mixed type comparison */
        checksum += 131072;
    }
    
    if (f1 > int_val) {   /* Another mixed type */
        checksum += 262144;
    }
    
    /* ===== 8. Use SSE intrinsics for explicit unordered checks ===== */
    
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* Explicit unordered comparison intrinsic */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    float unord_result;
    _mm_store_ss(&unord_result, cmp_unord);
    if (unord_result != 0.0f) {
        checksum += 524288;
    }
    
    /* Ordered comparison */
    __m128 cmp_ord = _mm_cmpord_ps(sse_a, sse_b);
    float ord_result;
    _mm_store_ss(&ord_result, cmp_ord);
    if (ord_result != 0.0f) {
        checksum += 1048576;
    }
    
    /* ===== Final output to prevent dead code elimination ===== */
    
    use_result(checksum);
    
    /* Print checksum based on all comparisons */
    printf("Condition code test checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-constant value */
}

/* External volatile definitions to prevent optimization */
volatile double external_double = 3.14159;
volatile float external_float = 2.71828f;
