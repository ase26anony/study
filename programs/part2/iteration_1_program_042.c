/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile variables to prevent optimization */
volatile double g_nan = __builtin_nan("");
volatile double g_inf = __builtin_inf();
volatile double g_ninf = -__builtin_inf();
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg = -1.0;

/* Function with complex control flow using goto */
int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    int temp;
    
    /* Matrix of comparisons with all possible outcomes */
    
    /* UNORDERED cases (involving NaN) */
    if (v1 != v1) { /* NaN check */
        result |= 1;
    }
    
    if (v2 == g_nan) { /* Always false, but compiler doesn't know v2 isn't NaN */
        result |= 2;
    }
    
    /* ORDERED cases (both non-NaN) */
    if (v3 < v4 && v3 == v3 && v4 == v4) {
        result |= 4;
    }
    
    /* Complex conditional chain with all comparison operators */
    if (v1 == v2) {
        result |= 8;
        goto label_eq;
    } else if (v1 != v2) {
        result |= 16;
        goto label_neq;
    }
    
label_eq:
    if (v1 < v3) {
        result |= 32;
        goto label_lt;
    }
    
label_neq:
    if (v2 <= v4) {
        result |= 64;
        goto label_le;
    }
    
label_lt:
    if (v3 > v1) {
        result |= 128;
        goto label_gt;
    }
    
label_le:
    if (v4 >= v2) {
        result |= 256;
    }
    
label_gt:
    /* UNEQ: unordered or equal */
    if (!(v1 < v2) && !(v1 > v2)) {
        result |= 512;
    }
    
    /* UNGE: unordered or greater or equal */
    if (!(v1 < v2)) {
        result |= 1024;
    }
    
    /* UNGT: unordered or greater */
    if (!(v1 <= v2)) {
        result |= 2048;
    }
    
    /* UNLE: unordered or less or equal */
    if (!(v1 > v2)) {
        result |= 4096;
    }
    
    /* UNLT: unordered or less */
    if (!(v1 >= v2)) {
        result |= 8192;
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((v1 < v2) || (v1 > v2)) {
        result |= 16384;
    }
    
    /* Conditional expressions (ternary operator) */
    temp = (v1 == v2) ? 1 : 0;
    result += temp;
    
    temp = (v1 != v2) ? 2 : 0;
    result += temp;
    
    temp = (v1 < v2) ? 4 : 0;
    result += temp;
    
    temp = (v1 <= v2) ? 8 : 0;
    result += temp;
    
    temp = (v1 > v2) ? 16 : 0;
    result += temp;
    
    temp = (v1 >= v2) ? 32 : 0;
    result += temp;
    
    /* NaN comparisons */
    temp = (v1 == g_nan) ? 64 : 0;
    result += temp;
    
    temp = (v1 != g_nan) ? 128 : 0;
    result += temp;
    
    temp = (g_nan == g_nan) ? 256 : 0; /* Always false */
    result += temp;
    
    temp = (g_nan != g_nan) ? 512 : 0; /* Always true */
    result += temp;
    
    return result;
}

/* Vectorized FP comparisons */
void vector_fp_comparisons(double *arr1, double *arr2, int *mask, int n) {
    /* Process using vector extensions */
    for (int i = 0; i < n; i += 2) {
        v2df v1 = {arr1[i], arr1[i+1]};
        v2df v2 = {arr2[i], arr2[i+1]};
        
        /* Generate various comparison masks */
        v2di cmp_eq = (v2di)(v1 == v2);
        v2di cmp_neq = (v2di)(v1 != v2);
        v2di cmp_lt = (v2di)(v1 < v2);
        v2di cmp_le = (v2di)(v1 <= v2);
        v2di cmp_gt = (v2di)(v1 > v2);
        v2di cmp_ge = (v2di)(v1 >= v2);
        
        /* Store results */
        memcpy(&mask[i], &cmp_eq, sizeof(cmp_eq));
        memcpy(&mask[i+2], &cmp_neq, sizeof(cmp_neq));
        memcpy(&mask[i+4], &cmp_lt, sizeof(cmp_lt));
        memcpy(&mask[i+6], &cmp_le, sizeof(cmp_le));
        memcpy(&mask[i+8], &cmp_gt, sizeof(cmp_gt));
        memcpy(&mask[i+10], &cmp_ge, sizeof(cmp_ge));
    }
}

/* Float vector comparisons */
void float_vector_comparisons(float *arr1, float *arr2, int *mask, int n) {
    for (int i = 0; i < n; i += 4) {
        v4sf v1 = {arr1[i], arr1[i+1], arr1[i+2], arr1[i+3]};
        v4sf v2 = {arr2[i], arr2[i+1], arr2[i+2], arr2[i+3]};
        
        v4si cmp_eq = (v4si)(v1 == v2);
        v4si cmp_neq = (v4si)(v1 != v2);
        v4si cmp_lt = (v4si)(v1 < v2);
        v4si cmp_le = (v4si)(v1 <= v2);
        v4si cmp_gt = (v4si)(v1 > v2);
        v4si cmp_ge = (v4si)(v1 >= v2);
        
        memcpy(&mask[i], &cmp_eq, sizeof(cmp_eq));
        memcpy(&mask[i+4], &cmp_neq, sizeof(cmp_neq));
        memcpy(&mask[i+8], &cmp_lt, sizeof(cmp_lt));
        memcpy(&mask[i+12], &cmp_le, sizeof(cmp_le));
        memcpy(&mask[i+16], &cmp_gt, sizeof(cmp_gt));
        memcpy(&mask[i+20], &cmp_ge, sizeof(cmp_ge));
    }
}

/* Inline assembly that uses FP condition codes */
int fp_asm_conditions(double a, double b) {
    volatile double x = a;
    volatile double y = b;
    int result = 0;
    unsigned char cc;
    
    /* Test UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 0);
    
    /* Test ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 1);
    
    /* Test less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 2);
    
    /* Test less or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 3);
    
    /* Test equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 4);
    
    /* Test not equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 5);
    
    /* Test greater or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 6);
    
    /* Test greater */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc << 7);
    
    return result;
}

/* Conditional move using FP comparisons */
double fp_conditional_move(double a, double b, double c, double d) {
    volatile double x = a;
    volatile double y = b;
    double result;
    
    /* Force conditional move generation */
    result = (x < y) ? c : d;
    result += (x <= y) ? c : d;
    result += (x > y) ? c : d;
    result += (x >= y) ? c : d;
    result += (x == y) ? c : d;
    result += (x != y) ? c : d;
    
    /* NaN comparisons for unordered cases */
    result += (x == g_nan) ? c : d;
    result += (x != g_nan) ? c : d;
    result += (g_nan == g_nan) ? c : d;
    result += (g_nan != g_nan) ? c : d;
    
    return result;
}

int main() {
    /* Initialize test values */
    double normal_vals[] = {0.0, 1.0, -1.0, 2.5, -3.75, 100.0, -100.0};
    double special_vals[] = {__builtin_nan(""), __builtin_inf(), -__builtin_inf()};
    
    int total_result = 0;
    
    /* Test all combinations of values */
    for (int i = 0; i < sizeof(normal_vals)/sizeof(normal_vals[0]); i++) {
        for (int j = 0; j < sizeof(special_vals)/sizeof(special_vals[0]); j++) {
            for (int k = 0; k < sizeof(normal_vals)/sizeof(normal_vals[0]); k++) {
                for (int l = 0; l < sizeof(special_vals)/sizeof(special_vals[0]); l++) {
                    total_result ^= fp_comparison_stress(
                        normal_vals[i], 
                        special_vals[j], 
                        normal_vals[k], 
                        special_vals[l]
                    );
                }
            }
        }
    }
    
    /* Test vectorized comparisons */
    double arr1[16], arr2[16];
    int mask[64];
    
    for (int i = 0; i < 16; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = (i % 2 == 0) ? i * 0.5 : i * 0.5 + 0.25;
    }
    /* Insert some NaN and Inf values */
    arr1[3] = __builtin_nan("");
    arr2[7] = __builtin_inf();
    arr1[11] = -__builtin_inf();
    
    vector_fp_comparisons(arr1, arr2, mask, 8);
    
    for (int i = 0; i < 16; i++) {
        total_result += mask[i];
    }
    
    /* Test float vector comparisons */
    float farr1[32], farr2[32];
    int fmask[96];
    
    for (int i = 0; i < 32; i++) {
        farr1[i] = i * 0.25f;
        farr2[i] = (i % 3 == 0) ? i * 0.25f : i * 0.25f + 0.125f;
    }
    farr1[5] = __builtin_nanf("");
    farr2[13] = __builtin_inff();
    farr1[21] = -__builtin_inff();
    
    float_vector_comparisons(farr1, farr2, fmask, 8);
    
    for (int i = 0; i < 32; i++) {
        total_result += fmask[i];
    }
    
    /* Test inline assembly conditions */
    for (int i = 0; i < sizeof(normal_vals)/sizeof(normal_vals[0]); i++) {
        for (int j = 0; j < sizeof(special_vals)/sizeof(special_vals[0]); j++) {
            total_result += fp_asm_conditions(normal_vals[i], special_vals[j]);
            total_result += fp_asm_conditions(special_vals[j], normal_vals[i]);
        }
    }
    
    /* Test conditional moves */
    for (int i = 0; i < sizeof(normal_vals)/sizeof(normal_vals[0]); i++) {
        for (int j = 0; j < sizeof(normal_vals)/sizeof(normal_vals[0]); j++) {
            if (i != j) {
                double cmov_result = fp_conditional_move(
                    normal_vals[i],
                    normal_vals[j],
                    normal_vals[(i+1) % 7],
                    normal_vals[(j+1) % 7]
                );
                total_result += (int)cmov_result;
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
