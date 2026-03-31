#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Test scalar floating-point comparisons */
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
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
            goto skip_point;
        } else {
            results[idx++] = 400 + i;
        }
        
        if (d1 <= d2) {
            results[idx++] = 500 + i;
        }
        
        skip_point:
        /* Empty label target */
        ;
    }
    
    /* Switch statement with floating comparisons */
    int case_selector = (int)(f1 * 100) % 5;
    switch (case_selector) {
        case 0:
            results[idx++] = (f1 < f2) ? 1000 : 2000;
            break;
        case 1:
            results[idx++] = (f1 > f2) ? 1001 : 2001;
            break;
        case 2:
            results[idx++] = (f1 <= f2) ? 1002 : 2002;
            break;
        case 3:
            results[idx++] = (f1 >= f2) ? 1003 : 2003;
            break;
        case 4:
            results[idx++] = (f1 == f2) ? 1004 : 2004;
            break;
        default:
            results[idx++] = 3000;
    }
}

/* Test builtin unordered comparisons */
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    /* Builtins that directly map to condition codes */
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      /* GT, unordered false */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0; /* GE, unordered false */
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         /* LT, unordered false */
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    /* LE, unordered false */
    results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;  /* LTGT */
    results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;    /* UNORDERED */
    
    /* Double precision builtins */
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
    
    /* fpclassify for more condition code possibilities */
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 17 : 0;
    results[idx++] = (fpclassify(f1) == FP_INFINITE) ? 18 : 0;
    results[idx++] = (fpclassify(f1) == FP_ZERO) ? 19 : 0;
    results[idx++] = (fpclassify(d1) == FP_NAN) ? 20 : 0;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 21 : 0;
    results[idx++] = (fpclassify(d1) == FP_ZERO) ? 22 : 0;
    
    /* Mixed type comparisons */
    results[idx++] = (f1 < (float)d1) ? 23 : 0;
    results[idx++] = ((double)f1 > d2) ? 24 : 0;
}

/* Test vector/SIMD comparisons */
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f2, f1 * 2.0f, f2 * 2.0f};
    v4sf vf2 = {f2, f1, f2 * 2.0f, f1 * 2.0f};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    /* Vector comparisons - these may generate packed condition codes */
    v4sf cmp_lt = vf1 < vf2;
    v4sf cmp_gt = vf1 > vf2;
    v4sf cmp_eq = vf1 == vf2;
    v4sf cmp_ne = vf1 != vf2;
    v4sf cmp_le = vf1 <= vf2;
    v4sf cmp_ge = vf1 >= vf2;
    
    v2df dcmp_lt = vd1 < vd2;
    v2df dcmp_gt = vd1 > vd2;
    v2df dcmp_eq = vd1 == vd2;
    
    /* Reduce vector results to scalar for checksum */
    float* fptr_lt = (float*)&cmp_lt;
    float* fptr_gt = (float*)&cmp_gt;
    double* dptr_lt = (double*)&dcmp_lt;
    
    results[0] = (int)(fptr_lt[0] + fptr_lt[1] + fptr_lt[2] + fptr_lt[3]);
    results[1] = (int)(fptr_gt[0] + fptr_gt[1] + fptr_gt[2] + fptr_gt[3]);
    results[2] = (int)(dptr_lt[0] + dptr_lt[1]);
    
    /* Conditional move based on vector comparison reduction */
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        mask |= (fptr_lt[i] != 0.0f) ? (1 << i) : 0;
    }
    
    results[3] = mask;
    
    /* Nested loop with vector-derived conditions */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (fptr_lt[i * 2 + j] != 0.0f) {
                results[4 + i * 2 + j] = 100 + i * 10 + j;
                if (dptr_lt[i] != 0.0) {
                    results[8 + i * 2 + j] = 200 + i * 10 + j;
                    continue;
                }
            }
        }
    }
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int r1, r2, r3, r4, r5, r6, r7, r8;
    int idx = 0;
    
    /* Inline assembly that uses condition code names */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r1;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(r2)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r2;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r3;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(r4)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r4;
    
    /* Using 'g' constraint to let compiler choose register */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=g"(r5)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = r5;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setae %0"
        : "=g"(r6)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = r6;
    
    /* Test ordered/unordered specifically */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0\n\t"    /* Ordered: PF=0 */
        : "=r"(r7)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r7;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0\n\t"     /* Unordered: PF=1 */
        : "=r"(r8)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = r8;
    
    /* Conditional move via assembly */
    int cmov_result;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "movl $100, %0\n\t"
        "movl $200, %1\n\t"
        "cmova %1, %0"
        : "=r"(cmov_result), "=r"(r1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = cmov_result;
}

int main() {
    /* Initialize test data with special values */
    float f_values[] = {
        1.0f, 2.0f, 0.0f, -1.0f, 
        NAN, INFINITY, -INFINITY,
        3.14f, -2.71f
    };
    
    double d_values[] = {
        1.0, 2.0, 0.0, -1.0,
        NAN, INFINITY, -INFINITY,
        3.141592653589793, -2.718281828459045
    };
    
    int results[256];
    memset(results, 0, sizeof(results));
    
    /* Run tests with various combinations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j) continue;
            
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            /* Test scalar comparisons */
            test_scalar_cmps(f1, f2, d1, d2, results);
            
            /* Test builtins */
            test_builtins(f1, f2, d1, d2, results + 50);
            
            /* Test vector operations */
            test_vector(f1, f2, d1, d2, results + 100);
            
            /* Test inline assembly */
            test_asm(f1, f2, d1, d2, results + 150);
            
            /* Update checksum to prevent optimization */
            for (int k = 0; k < 200; k++) {
                checksum += results[k];
                checksum ^= (results[k] << (k % 16));
            }
        }
    }
    
    /* Additional tests with specific NaN patterns */
    float quiet_nan = NAN;
    float signaling_nan = __builtin_nansf("");
    double quiet_nan_d = NAN;
    double signaling_nan_d = __builtin_nans("");
    
    test_scalar_cmps(quiet_nan, signaling_nan, quiet_nan_d, signaling_nan_d, results);
    test_builtins(quiet_nan, signaling_nan, quiet_nan_d, signaling_nan_d, results + 50);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
