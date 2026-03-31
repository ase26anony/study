#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global checksum to prevent optimization */
volatile uint64_t checksum = 0;

/* Test scalar floating-point comparisons */
void test_scalar_cmps(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    /* Basic comparisons that may generate various condition codes */
    results[idx++] = (f1 < f2) ? 1 : 0;      /* May generate LT or UNLT */
    results[idx++] = (f1 > f2) ? 2 : 0;      /* May generate GT or UNGT */
    results[idx++] = (f1 <= f2) ? 3 : 0;     /* May generate LE or UNLE */
    results[idx++] = (f1 >= f2) ? 4 : 0;     /* May generate GE or UNGE */
    results[idx++] = (f1 == f2) ? 5 : 0;     /* May generate EQ or UNEQ */
    results[idx++] = (f1 != f2) ? 6 : 0;     /* May generate NEQ or LTGT */
    
    /* Double precision comparisons */
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    /* Complex control flow with nested conditionals */
    for (int i = 0; i < 3; i++) {
        if (f1 < f2) {
            results[idx++] = 100 + i;
            if (d1 > d2) {
                results[idx++] = 200 + i;
                continue;
            }
        } else if (f1 == f2) {
            results[idx++] = 300 + i;
            break;
        } else {
            results[idx++] = 400 + i;
            goto label;
        }
        
        if (d1 != d2) {
            results[idx++] = 500 + i;
        }
        
        label:
        results[idx++] = 600 + i;
    }
    
    /* Switch statement with floating comparisons */
    int case_selector = (int)(f1 * 10) % 4;
    switch (case_selector) {
        case 0:
            results[idx++] = (f1 < f2 && d1 > d2) ? 1000 : 2000;
            break;
        case 1:
            results[idx++] = (f1 == f2 || d1 == d2) ? 3000 : 4000;
            break;
        case 2:
            results[idx++] = (f1 != f2) ? 5000 : 6000;
            break;
        case 3:
            results[idx++] = (f1 >= f2 && d1 <= d2) ? 7000 : 8000;
            break;
    }
}

/* Test built-in unordered comparison functions */
void test_builtins(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    /* These built-ins directly map to condition codes */
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;        /* GT, ordered */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0;   /* GE, ordered */
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;           /* LT, ordered */
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;      /* LE, ordered */
    results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;    /* LTGT, ordered */
    results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;      /* UNORDERED */
    
    /* Double precision versions */
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
    
    /* Classification functions */
    results[idx++] = isnan(f1) ? 13 : 0;
    results[idx++] = isinf(f1) ? 14 : 0;
    results[idx++] = isnan(d1) ? 15 : 0;
    results[idx++] = isinf(d1) ? 16 : 0;
    
    /* fpclassify for more condition codes */
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 17 : 0;
    results[idx++] = (fpclassify(f1) == FP_INFINITE) ? 18 : 0;
    results[idx++] = (fpclassify(f1) == FP_ZERO) ? 19 : 0;
    results[idx++] = (fpclassify(d1) == FP_NAN) ? 20 : 0;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 21 : 0;
    results[idx++] = (fpclassify(d1) == FP_ZERO) ? 22 : 0;
    
    /* Mixed comparisons in complex expressions */
    for (int i = 0; i < 5; i++) {
        if (__builtin_isunordered(f1 + i, f2)) {
            results[idx++] = 100 + i;
            continue;
        }
        
        if (__builtin_isgreater(d1, d2 + i)) {
            results[idx++] = 200 + i;
            if (__builtin_isless(f1, f2)) {
                results[idx++] = 300 + i;
                goto builtin_label;
            }
        }
        
        builtin_label:
        results[idx++] = 400 + i;
    }
}

/* Test vector/SIMD comparisons */
void test_vector(float *farr, double *darr, int *results) {
    int idx = 0;
    
    /* Load vectors */
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    /* Vector comparisons - these generate packed comparisons */
    v4sf vcmp_lt = vf1 < vf2;
    v4sf vcmp_gt = vf1 > vf2;
    v4sf vcmp_eq = vf1 == vf2;
    v4sf vcmp_neq = vf1 != vf2;
    
    v2df vcmp_dlt = vd1 < vd2;
    v2df vcmp_dgt = vd1 > vd2;
    v2df vcmp_deq = vd1 == vd2;
    v2df vcmp_dneq = vd1 != vd2;
    
    /* Reduce vector results to scalar for condition codes */
    float *fptr_lt = (float*)&vcmp_lt;
    float *fptr_gt = (float*)&vcmp_gt;
    double *dptr_lt = (double*)&vcmp_dlt;
    double *dptr_gt = (double*)&vcmp_dgt;
    
    /* Extract elements and create scalar condition codes */
    for (int i = 0; i < 4; i++) {
        results[idx++] = (fptr_lt[i] != 0.0f) ? 1000 + i : 0;
        results[idx++] = (fptr_gt[i] != 0.0f) ? 2000 + i : 0;
        results[idx++] = (fptr_lt[i] != 0.0f && fptr_gt[i] != 0.0f) ? 3000 + i : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = (dptr_lt[i] != 0.0) ? 4000 + i : 0;
        results[idx++] = (dptr_gt[i] != 0.0) ? 5000 + i : 0;
    }
    
    /* Nested loops with vector element comparisons */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            if (fptr_lt[i] != 0.0f && dptr_gt[j] != 0.0) {
                results[idx++] = 6000 + i * 10 + j;
                if (i > j) {
                    results[idx++] = 7000 + i * 10 + j;
                    continue;
                }
            } else {
                results[idx++] = 8000 + i * 10 + j;
                break;
            }
        }
    }
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    uint8_t byte_result;
    
    /* Inline assembly that uses condition code names */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* Double precision comparisons */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* Using specific condition code constraints */
    int int_result;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "mov $0, %0\n\t"
        "setne %b0\n\t"
        "setp %b1\n\t"
        "or %b1, %b0"
        : "=r" (int_result), "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = int_result;
    
    /* Test UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* Test ORDERED condition code */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result;
}

int main() {
    /* Initialize test data with special values */
    float fvalues[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        NAN, INFINITY, -INFINITY, 0.0f,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double dvalues[] = {
        1.0, 2.0, 3.0, 4.0,
        NAN, INFINITY, -INFINITY, 0.0,
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Results arrays */
    int results1[200] = {0};
    int results2[200] = {0};
    int results3[200] = {0};
    int results4[200] = {0};
    
    /* Run tests with different value combinations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            test_scalar_cmps(fvalues[i], fvalues[j], dvalues[i], dvalues[j], results1);
            test_builtins(fvalues[i], fvalues[j], dvalues[i], dvalues[j], results2);
            test_vector(fvalues, dvalues, results3);
            test_asm(fvalues[i], fvalues[j], dvalues[i], dvalues[j], results4);
            
            /* Update checksum to prevent dead code elimination */
            for (int k = 0; k < 50; k++) {
                checksum += results1[k] + results2[k] + results3[k] + results4[k];
            }
        }
    }
    
    /* Additional tests with specific value combinations */
    float special_f[] = {NAN, INFINITY, -INFINITY, 0.0f};
    double special_d[] = {NAN, INFINITY, -INFINITY, 0.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Test all combinations of special values */
            test_scalar_cmps(special_f[i], special_f[j], special_d[i], special_d[j], results1);
            test_builtins(special_f[i], special_f[j], special_d[i], special_d[j], results2);
            
            /* Force generation of UNORDERED/ORDERED condition codes */
            if (__builtin_isunordered(special_f[i], special_f[j])) {
                results1[0] = 9999;
            }
            if (!__builtin_isunordered(special_d[i], special_d[j])) {
                results2[0] = 8888;
            }
            
            checksum += results1[0] + results2[0];
        }
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
