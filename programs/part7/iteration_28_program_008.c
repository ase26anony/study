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
void test_scalar_cmps(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    /* Basic comparisons that should generate various condition codes */
    results[idx++] = (f1 < f2) ? 1 : 0;      /* UNLT or LT */
    results[idx++] = (f1 > f2) ? 2 : 0;      /* UNGT or GT */
    results[idx++] = (f1 <= f2) ? 3 : 0;     /* UNLE or LE */
    results[idx++] = (f1 >= f2) ? 4 : 0;     /* UNGE or GE */
    results[idx++] = (f1 == f2) ? 5 : 0;     /* UNEQ or EQ */
    results[idx++] = (f1 != f2) ? 6 : 0;     /* LTGT or NE */
    
    /* Double precision comparisons */
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    /* Complex control flow with condition codes */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2) ? 13 : 0;
                break;
            case 1:
                results[idx++] = (f1 > f2) ? 14 : 0;
                if (d1 != d2) goto label1;
                break;
            case 2:
            label1:
                results[idx++] = (f1 <= f2) ? 15 : 0;
                continue;
        }
        results[idx++] = (f1 >= f2) ? 16 : 0;
    }
    
    /* Nested if-else chains */
    if (f1 == f2) {
        if (d1 < d2) {
            results[idx++] = 17;
        } else if (d1 > d2) {
            results[idx++] = 18;
        } else {
            results[idx++] = 19;
        }
    } else if (f1 < f2) {
        results[idx++] = 20;
    } else {
        results[idx++] = 21;
    }
}

/* Test built-in unordered comparison functions */
void test_builtins(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    /* Built-ins that directly map to condition codes */
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      /* GT (ordered) */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0; /* GE (ordered) */
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         /* LT (ordered) */
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    /* LE (ordered) */
    results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;  /* LTGT */
    results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;    /* UNORDERED */
    
    /* Double precision built-ins */
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
    
    /* Classification functions */
    results[idx++] = isnan(f1) ? 13 : 0;
    results[idx++] = isinf(f1) ? 14 : 0;
    results[idx++] = fpclassify(f1) == FP_NAN ? 15 : 0;
    results[idx++] = fpclassify(d1) == FP_INFINITE ? 16 : 0;
    
    /* Conditional moves via ternary operators */
    int cmov_result;
    cmov_result = __builtin_isunordered(f1, f2) ? 100 : 200;
    results[idx++] = cmov_result;
    
    cmov_result = __builtin_isgreater(d1, d2) ? 300 : 400;
    results[idx++] = cmov_result;
}

/* Test vector/SIMD comparisons */
void test_vector(float f1, float f2, double d1, double d2, int *results) {
    v4sf vf1 = {f1, f2, f1, f2};
    v4sf vf2 = {f2, f1, f2, f1};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    /* Vector comparisons - these may generate packed comparisons */
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 <= vf2;
    v4sf cmp4 = vf1 >= vf2;
    v4sf cmp5 = vf1 == vf2;
    v4sf cmp6 = vf1 != vf2;
    
    /* Double vector comparisons */
    v2df cmp7 = vd1 < vd2;
    v2df cmp8 = vd1 > vd2;
    v2df cmp9 = vd1 <= vd2;
    v2df cmp10 = vd1 >= vd2;
    v2df cmp11 = vd1 == vd2;
    v2df cmp12 = vd1 != vd2;
    
    /* Reduce vector to scalar for condition code generation */
    float *fptr1 = (float*)&cmp1;
    float *fptr2 = (float*)&cmp2;
    double *dptr1 = (double*)&cmp7;
    
    results[0] = (fptr1[0] != 0.0f) ? 1 : 0;
    results[1] = (fptr2[1] != 0.0f) ? 2 : 0;
    results[2] = (dptr1[0] != 0.0) ? 3 : 0;
    
    /* Loop with vector comparisons */
    for (int i = 0; i < 4; i++) {
        if (fptr1[i] != 0.0f) {
            results[3 + i] = 10 + i;
        } else {
            results[3 + i] = 20 + i;
        }
    }
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int *results) {
    unsigned char byte_result;
    int int_result;
    
    /* Test various condition codes in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[0] = byte_result;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[1] = byte_result;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[2] = byte_result;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  /* Note: swapped operands for nlt */
        "setnb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[3] = byte_result;
    
    /* UNGT (not less or equal) */
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  /* Note: swapped operands for nle */
        "setnbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[4] = byte_result;
    
    /* UNLE (unordered or less or equal) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[5] = byte_result;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[6] = byte_result;
    
    /* LTGT (less or greater, ordered) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[7] = byte_result;
    
    /* Double precision with explicit condition code names */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"  /* above (greater than, ordered) */
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[8] = byte_result;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"  /* below (less than, unordered or ordered) */
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[9] = byte_result;
    
    /* Conditional move via assembly */
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "cmovb %1, %0"
        : "+r"(int_result)
        : "r"(100), "x"(f1), "x"(f2)
        : "cc"
    );
    results[10] = int_result;
}

int main() {
    /* Initialize test values including special floating-point values */
    float f_values[] = {
        1.0f, 2.0f, -1.0f, 0.0f,
        NAN, INFINITY, -INFINITY,
        3.14f, -2.71f
    };
    
    double d_values[] = {
        1.0, 2.0, -1.0, 0.0,
        NAN, INFINITY, -INFINITY,
        3.141592653589793, -2.718281828459045
    };
    
    int results[256];
    memset(results, 0, sizeof(results));
    
    /* Run tests with various combinations of values */
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            /* Call test functions */
            test_scalar_cmps(f1, f2, d1, d2, results);
            test_builtins(f1, f2, d1, d2, results + 50);
            test_vector(f1, f2, d1, d2, results + 100);
            test_asm(f1, f2, d1, d2, results + 150);
            
            /* Update checksum to prevent dead code elimination */
            for (int k = 0; k < 200; k++) {
                checksum += results[k];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
