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
    
    /* 1. Initialize FP variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = 3.14159;
    float f1 = get_float_input();
    float f2 = 2.71828f;
    
    /* 2. Perform relational comparisons with -ffast-math assumptions */
    
    /* UNORDERED: May be generated when NaNs are possible but -ffast-math assumes none */
    if (d1 != d1) { /* NaN check - with -ffast-math may still generate unord */
        checksum += 1;
    }
    
    /* ORDERED: Ordered comparison */
    if (vd1 < vd2) {
        checksum += 2;
    }
    
    /* UNEQ: Unordered or equal */
    if (f1 == f2) {
        checksum += 4;
    }
    
    /* UNGE: Unordered or greater than or equal (not less than) */
    double d3 = (d1 >= d2) ? d1 : d2;  /* May use UNGE -> "nlt" */
    checksum += (int)(d3 * 10);
    
    /* UNGT: Unordered or greater than (not less than or equal) */
    if (d1 > d2) {
        checksum += 8;
    }
    
    /* UNLE: Unordered or less than or equal */
    float f3 = (f1 <= f2) ? f1 : f2;  /* May use UNLE -> "ule" */
    checksum += (int)(f3 * 100);
    
    /* UNLT: Unordered or less than */
    if (vd1 < 0.0) {
        checksum += 16;
    }
    
    /* LTGT: Less than or greater than (unordered or not equal) */
    if (d1 != d2) {  /* With -ffast-math may generate LTGT -> "une" */
        checksum += 32;
    }
    
    /* 3. Explicit built-in functions for specific condition codes */
    
    /* Direct UNORDERED test */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* LTGT builtin */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* Ordered comparison builtins */
    if (__builtin_isless(f1, f2)) {
        checksum += 256;
    }
    
    if (__builtin_islessequal(d1, d2)) {
        checksum += 512;
    }
    
    if (__builtin_isgreater(f1, f2)) {
        checksum += 1024;
    }
    
    if (__builtin_isgreaterequal(d1, d2)) {
        checksum += 2048;
    }
    
    /* 4. Vector (SIMD) comparisons */
    
    /* Initialize vector floats */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons - each may generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float eq_result = __builtin_convertvector(cmp_eq, v4sf)[0];
    float neq_result = __builtin_convertvector(cmp_neq, v4sf)[0];
    checksum += (int)(eq_result * 1000) + (int)(neq_result * 2000);
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    
    /* Extract mask from unordered comparison */
    int unord_mask = _mm_movemask_ps(unord_cmp);
    checksum += unord_mask;
    
    /* 5. Loop with FP condition to generate branch with condition code */
    double arr[4] = {d1, d2, vd1, vd2};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {
        checksum += i * 10;
    }
    
    /* 6. Switch based on FP comparison results */
    int cmp_result = 0;
    cmp_result |= (d1 < d2) ? 1 : 0;
    cmp_result |= (f1 > f2) ? 2 : 0;
    cmp_result |= (vd1 == vd2) ? 4 : 0;
    cmp_result |= (d1 != d2) ? 8 : 0;
    
    switch (cmp_result & 3) {
        case 0:
            checksum += 10000;
            break;
        case 1:
            checksum += 20000;
            break;
        case 2:
            checksum += 30000;
            break;
        case 3:
            checksum += 40000;
            break;
    }
    
    /* 7. Mixed integer/float comparisons */
    int int_val = checksum % 100;
    if (f1 < int_val) {
        checksum += 50000;
    }
    
    if (d1 > int_val) {
        checksum += 60000;
    }
    
    /* 8. Complex conditional expressions */
    double complex_cond = (d1 < d2 && f1 > f2) ? d1 : 
                         (d1 == d2 || f1 != f2) ? d2 : 
                         (d1 >= 0.0 && d2 <= 100.0) ? 0.5 : 1.0;
    checksum += (int)(complex_cond * 100000);
    
    /* Create observable output to prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum > 0 ? 0 : 1;
}

/* External volatile definitions to prevent optimization */
volatile double external_double = 1.23456789;
volatile float external_float = 9.87654321f;
