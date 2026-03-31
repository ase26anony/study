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
    results[idx++] = (f1 < f2) ? 1 : 0;      /* May generate UNLT/LT */
    results[idx++] = (f1 > f2) ? 2 : 0;      /* May generate UNGT/GT */
    results[idx++] = (f1 <= f2) ? 3 : 0;     /* May generate UNLE/LE */
    results[idx++] = (f1 >= f2) ? 4 : 0;     /* May generate UNGE/GE */
    results[idx++] = (f1 == f2) ? 5 : 0;     /* May generate UNEQ/EQ */
    results[idx++] = (f1 != f2) ? 6 : 0;     /* May generate LTGT/NE */
    
    /* Double precision comparisons */
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    /* Mixed type comparisons */
    results[idx++] = ((double)f1 < d2) ? 13 : 0;
    results[idx++] = (f1 > (float)d2) ? 14 : 0;
    
    /* Complex control flow with comparisons */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2) ? 100 + i : 200 + i;
                break;
            case 1:
                results[idx++] = (f1 > f2) ? 100 + i : 200 + i;
                break;
            case 2:
                results[idx++] = (f1 == f2) ? 100 + i : 200 + i;
                /* Fall through to trigger different basic blocks */
            default:
                results[idx++] = (f1 != f2) ? 300 + i : 400 + i;
                break;
        }
        
        /* Nested if-else with goto */
        if (d1 < d2) {
            results[idx++] = 500;
            if (isnan(d1)) {
                goto skip_point;
            }
        } else if (d1 > d2) {
            results[idx++] = 600;
        } else {
            results[idx++] = 700;
        }
        
        skip_point:
        /* Continue with more comparisons */
        results[idx++] = (isinf(f1) && f1 > f2) ? 800 : 900;
    }
}

/* Test builtin unordered comparisons */
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    /* Builtins that directly map to condition codes */
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      /* UNGT/GT */
    results[idx++] = __builtin_isless(f1, f2) ? 2 : 0;         /* UNLT/LT */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0; /* UNGE/GE */
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    /* UNLE/LE */
    results[idx++] = __builtin_isunordered(f1, f2) ? 5 : 0;    /* UNORDERED */
    
    /* Double precision builtins */
    results[idx++] = __builtin_isgreater(d1, d2) ? 6 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 8 : 0;
    
    /* Combined builtins in complex expressions */
    results[idx++] = (__builtin_isgreater(f1, f2) && !__builtin_isunordered(f1, f2)) ? 9 : 0;
    results[idx++] = (__builtin_isless(d1, d2) || __builtin_isunordered(d1, d2)) ? 10 : 0;
    
    /* Builtins with classification functions */
    results[idx++] = (isnan(f1) && __builtin_isunordered(f1, f2)) ? 11 : 0;
    results[idx++] = (!isinf(d1) && __builtin_isgreater(d1, d2)) ? 12 : 0;
    
    /* Loop with builtin comparisons */
    for (int i = 0; i < 4; i++) {
        float temp = f1 + i;
        results[idx++] = __builtin_isgreater(temp, f2) ? 100 + i : 200 + i;
        
        if (__builtin_isless(d1, d2 + i)) {
            results[idx++] = 300 + i;
            continue;
        }
        results[idx++] = 400 + i;
    }
}

/* Test vector/SIMD comparisons */
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    /* Initialize vectors */
    v4sf vf1 = {f1, f2, f1 + 1.0f, f2 - 1.0f};
    v4sf vf2 = {f2, f1, f2 + 1.0f, f1 - 1.0f};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    /* Vector comparisons - these generate packed comparisons */
    v4sf cmp_lt = vf1 < vf2;
    v4sf cmp_gt = vf1 > vf2;
    v4sf cmp_eq = vf1 == vf2;
    v4sf cmp_ne = vf1 != vf2;
    
    v2df dcmp_lt = vd1 < vd2;
    v2df dcmp_gt = vd1 > vd2;
    
    /* Reduce vector results to scalar for condition code generation */
    float* fcmp_lt = (float*)&cmp_lt;
    float* fcmp_gt = (float*)&cmp_gt;
    
    for (int i = 0; i < 4; i++) {
        /* Use vector comparison results in scalar conditionals */
        results[idx++] = (fcmp_lt[i] != 0.0f) ? 1 : 0;
        results[idx++] = (fcmp_gt[i] != 0.0f) ? 2 : 0;
        results[idx++] = (fcmp_lt[i] != 0.0f && fcmp_gt[i] == 0.0f) ? 3 : 0;
    }
    
    /* Double vector comparisons */
    double* dcmp_lt_p = (double*)&dcmp_lt;
    double* dcmp_gt_p = (double*)&dcmp_gt;
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = (dcmp_lt_p[i] != 0.0) ? 4 : 0;
        results[idx++] = (dcmp_gt_p[i] != 0.0) ? 5 : 0;
        
        /* Complex condition with vector results */
        if (dcmp_lt_p[i] != 0.0 || dcmp_gt_p[i] != 0.0) {
            results[idx++] = 6;
            goto vector_label;
        }
        results[idx++] = 7;
        
        vector_label:
        /* More comparisons in the goto target */
        results[idx++] = (vf1[i] < vf2[i]) ? 8 : 0;
    }
    
    /* Mixed vector-scalar comparisons */
    results[idx++] = ((vf1[0] < vf2[0]) && (vd1[0] > vd2[0])) ? 9 : 0;
    results[idx++] = ((vf1[1] == vf2[1]) || (vd1[1] != vd2[1])) ? 10 : 0;
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    uint8_t byte_result;
    int int_result;
    
    /* Test various condition codes via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "sete %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "al", "cc"
    );
    results[idx++] = byte_result;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* UNGT (not less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* UNLE (unordered or less than or equal) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    /* LTGT (less than or greater than - ordered and not equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0\n\t"
        "setnp %%al\n\t"
        "andb %%al, %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "al", "cc"
    );
    results[idx++] = byte_result;
    
    /* Test with "g" constraint for register selection */
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "seta %0"
        : "=g"(int_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = int_result;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=g"(int_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = int_result;
    
    /* Complex assembly with multiple condition codes */
    int cmp1, cmp2;
    __asm__ volatile (
        "ucomiss %3, %2\n\t"
        "setp %0\n\t"
        "ucomisd %5, %4\n\t"
        "setnp %1"
        : "=r"(cmp1), "=r"(cmp2)
        : "x"(f1), "x"(f2), "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = cmp1;
    results[idx++] = cmp2;
}

int main() {
    /* Initialize test values including special floating-point values */
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
    
    /* Results array - large enough for all tests */
    int results[500];
    memset(results, 0, sizeof(results));
    
    /* Run tests with different value combinations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int base_idx = (i * 5 + j) * 20;
            
            /* Test scalar comparisons */
            test_scalar_cmps(f_values[i], f_values[j], 
                           d_values[i], d_values[j],
                           &results[base_idx]);
            
            /* Test builtins */
            test_builtins(f_values[i], f_values[j],
                         d_values[i], d_values[j],
                         &results[base_idx + 50]);
            
            /* Test vector comparisons */
            test_vector(f_values[i], f_values[j],
                       d_values[i], d_values[j],
                       &results[base_idx + 100]);
            
            /* Test inline assembly */
            test_asm(f_values[i], f_values[j],
                    d_values[i], d_values[j],
                    &results[base_idx + 150]);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 500; i++) {
        checksum += results[i];
    }
    
    /* Print checksum (prevents optimization) */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
