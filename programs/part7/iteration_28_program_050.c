#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float farr[8] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
double darr[8] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};

/* Test scalar comparisons with all relational operators */
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Mix float and double comparisons */
            float f1 = farr[i];
            float f2 = farr[j];
            double d1 = darr[i];
            double d2 = darr[j];
            
            /* Use ternary operators to force CMOV/SET generation */
            results[idx++] = (f1 < f2) ? 1 : 0;      /* UNLT or LT */
            results[idx++] = (f1 > f2) ? 2 : 0;      /* UNGT or GT */
            results[idx++] = (f1 <= f2) ? 3 : 0;     /* UNLE or LE */
            results[idx++] = (f1 >= f2) ? 4 : 0;     /* UNGE or GE */
            results[idx++] = (f1 == f2) ? 5 : 0;     /* UNEQ or EQ */
            results[idx++] = (f1 != f2) ? 6 : 0;     /* LTGT or NE */
            
            /* Double comparisons */
            results[idx++] = (d1 < d2) ? 7 : 0;
            results[idx++] = (d1 > d2) ? 8 : 0;
            results[idx++] = (d1 <= d2) ? 9 : 0;
            results[idx++] = (d1 >= d2) ? 10 : 0;
            results[idx++] = (d1 == d2) ? 11 : 0;
            results[idx++] = (d1 != d2) ? 12 : 0;
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test built-in unordered comparisons */
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    /* Complex control flow with nested if-else */
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        double d1 = darr[i];
        
        /* Test all __builtin_is* functions */
        results[idx++] = __builtin_isunordered(f1, farr[(i+1)%8]) ? 1 : 0;
        results[idx++] = __builtin_isgreater(f1, farr[(i+2)%8]) ? 2 : 0;
        results[idx++] = __builtin_isless(f1, farr[(i+3)%8]) ? 3 : 0;
        results[idx++] = __builtin_isgreaterequal(f1, farr[(i+4)%8]) ? 4 : 0;
        results[idx++] = __builtin_islessequal(f1, farr[(i+5)%8]) ? 5 : 0;
        
        /* Double versions */
        results[idx++] = __builtin_isunordered(d1, darr[(i+1)%8]) ? 6 : 0;
        results[idx++] = __builtin_isgreater(d1, darr[(i+2)%8]) ? 7 : 0;
        results[idx++] = __builtin_isless(d1, darr[(i+3)%8]) ? 8 : 0;
        
        /* Classification functions */
        results[idx++] = isnan(f1) ? 9 : 0;
        results[idx++] = isinf(d1) ? 10 : 0;
        results[idx++] = (fpclassify(f1) == FP_NAN) ? 11 : 0;
        results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 12 : 0;
    }
    
    /* Switch statement with condition codes */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        switch (results[i] % 8) {
            case 0: sum += results[i]; break;
            case 1: sum += results[i] * 2; break;
            case 2: sum += results[i] * 3; break;
            case 3: sum += results[i] * 4; break;
            case 4: sum += results[i] * 5; break;
            case 5: sum += results[i] * 6; break;
            case 6: sum += results[i] * 7; break;
            case 7: sum += results[i] * 8; break;
            default: goto end_loop;
        }
    }
end_loop:
    
    return sum;
}

/* Test vector/SIMD comparisons */
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int results[16];
    int idx = 0;
    
    /* Vector comparisons - these generate packed comparison RTL */
    v4sf vcmp_lt = vf1 < vf2;
    v4sf vcmp_gt = vf1 > vf2;
    v4sf vcmp_eq = vf1 == vf2;
    v4sf vcmp_neq = vf1 != vf2;
    
    v2df vdcmp_lt = vd1 < vd2;
    v2df vdcmp_gt = vd1 > vd2;
    
    /* Reduce vector to scalar mask */
    float* fptr_lt = (float*)&vcmp_lt;
    float* fptr_gt = (float*)&vcmp_gt;
    
    for (int i = 0; i < 4; i++) {
        results[idx++] = fptr_lt[i] != 0.0f ? 1 : 0;
        results[idx++] = fptr_gt[i] != 0.0f ? 2 : 0;
        results[idx++] = ((float*)&vcmp_eq)[i] != 0.0f ? 3 : 0;
        results[idx++] = ((float*)&vcmp_neq)[i] != 0.0f ? 4 : 0;
    }
    
    double* dptr_lt = (double*)&vdcmp_lt;
    double* dptr_gt = (double*)&vdcmp_gt;
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dptr_lt[i] != 0.0 ? 5 : 0;
        results[idx++] = dptr_gt[i] != 0.0 ? 6 : 0;
    }
    
    /* Compute checksum with loop unrolling hint */
    int sum = 0;
    for (int i = 0; i < idx; i += 2) {
        sum += results[i];
        if (i + 1 < idx) {
            sum += results[i + 1];
        }
    }
    
    return sum;
}

/* Test inline assembly with condition codes */
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = darr[i];
        double d2 = darr[(i+1)%8];
        
        /* Compare and set flags */
        int cmp_result;
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "seta %0\n\t"      /* Above (greater, not unordered) */
            : "=r" (cmp_result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = cmp_result;
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0\n\t"      /* Below (less, not unordered) */
            : "=r" (cmp_result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = cmp_result;
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"      /* Equal (and ordered) */
            : "=r" (cmp_result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = cmp_result;
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %0\n\t"      /* Parity (unordered) */
            : "=r" (cmp_result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = cmp_result;
        
        /* Test all condition code variants */
        unsigned char cc_result;
        float f1 = farr[i];
        float f2 = farr[(i+2)%8];
        
        /* Using symbolic condition code names */
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "set%3 %0\n\t"
            : "=r" (cc_result)
            : "x" (f1), "x" (f2), "i" (/* condition code inserted here */ "a")
            : "cc"
        );
        results[idx++] = cc_result;
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "set%3 %0\n\t"
            : "=r" (cc_result)
            : "x" (f1), "x" (f2), "i" (/* condition code inserted here */ "b")
            : "cc"
        );
        results[idx++] = cc_result;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    /* Initialize arrays with special values */
    farr[2] = NAN;
    farr[3] = INFINITY;
    farr[4] = -INFINITY;
    
    darr[2] = NAN;
    darr[3] = INFINITY;
    darr[4] = -INFINITY;
    
    /* Run all tests */
    total += test_scalar_cmps();
    total += test_builtins();
    total += test_vector();
    total += test_asm();
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
