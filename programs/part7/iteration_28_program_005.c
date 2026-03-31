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
    
    /* Basic comparisons that may generate various condition codes */
    results[idx++] = (f1 < f2) ? 1 : 0;      /* May generate UNLT or LT */
    results[idx++] = (f1 > f2) ? 2 : 0;      /* May generate UNGT or GT */
    results[idx++] = (f1 <= f2) ? 3 : 0;     /* May generate UNLE or LE */
    results[idx++] = (f1 >= f2) ? 4 : 0;     /* May generate UNGE or GE */
    results[idx++] = (f1 == f2) ? 5 : 0;     /* May generate UNEQ or EQ */
    results[idx++] = (f1 != f2) ? 6 : 0;     /* May generate LTGT or NE */
    
    /* Double precision comparisons */
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    /* Complex control flow with comparisons */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2) ? 13 : 0;
                break;
            case 1:
                results[idx++] = (f1 > f2) ? 14 : 0;
                if (d1 <= d2) {
                    results[idx++] = 15;
                    continue;
                }
                break;
            case 2:
                results[idx++] = (f1 == f2) ? 16 : 0;
                goto done;
            default:
                results[idx++] = (f1 != f2) ? 17 : 0;
        }
        results[idx++] = 18; /* Fallthrough marker */
    }
done:
    /* Nested if-else chains */
    if (f1 < f2) {
        if (d1 > d2) {
            results[idx++] = 19;
        } else if (d1 == d2) {
            results[idx++] = 20;
        } else {
            results[idx++] = 21;
        }
    } else if (f1 > f2) {
        results[idx++] = 22;
    } else {
        results[idx++] = 23;
    }
}

/* Test built-in unordered comparisons */
void test_builtins(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    /* Built-ins that directly map to condition codes */
    results[idx++] = __builtin_isunordered(f1, f2) ? 24 : 0;    /* UNORDERED */
    results[idx++] = __builtin_isgreater(f1, f2) ? 25 : 0;      /* UNLE? Actually generates GT */
    results[idx++] = __builtin_isless(f1, f2) ? 26 : 0;         /* UNGE? Actually generates LT */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 27 : 0; /* UNLT? Actually generates GE */
    results[idx++] = __builtin_islessequal(f1, f2) ? 28 : 0;    /* UNGT? Actually generates LE */
    
    /* Double precision built-ins */
    results[idx++] = __builtin_isunordered(d1, d2) ? 29 : 0;
    results[idx++] = __builtin_isgreater(d1, d2) ? 30 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 31 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 32 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 33 : 0;
    
    /* Classification functions */
    results[idx++] = isnan(f1) ? 34 : 0;
    results[idx++] = isinf(f1) ? 35 : 0;
    results[idx++] = isnan(d1) ? 36 : 0;
    results[idx++] = isinf(d1) ? 37 : 0;
    
    /* fpclassify usage */
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 38 : 0;
    results[idx++] = (fpclassify(f1) == FP_INFINITE) ? 39 : 0;
    results[idx++] = (fpclassify(d1) == FP_NAN) ? 40 : 0;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 41 : 0;
    
    /* Mixed comparisons in complex expressions */
    int temp = 0;
    for (int i = 0; i < 5; i++) {
        if (__builtin_isunordered(f1 + i, f2 - i)) {
            temp += 42;
            continue;
        }
        if (__builtin_isgreater(d1 * i, d2 / (i + 1))) {
            temp += 43;
            break;
        }
        temp += 44;
    }
    results[idx++] = temp;
}

/* Test vector/SIMD comparisons */
void test_vector(float *farr, double *darr, int *results) {
    v4sf va = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vb = {farr[4], farr[5], farr[6], farr[7]};
    v2df vda = {darr[0], darr[1]};
    v2df vdb = {darr[2], darr[3]};
    
    /* Vector comparisons generate packed condition codes */
    v4sf vcmp_lt = va < vb;
    v4sf vcmp_gt = va > vb;
    v4sf vcmp_eq = va == vb;
    v4sf vcmp_neq = va != vb;
    v4sf vcmp_le = va <= vb;
    v4sf vcmp_ge = va >= vb;
    
    v2df vdcmp_lt = vda < vdb;
    v2df vdcmp_gt = vda > vdb;
    v2df vdcmp_eq = vda == vdb;
    v2df vdcmp_neq = vda != vdb;
    
    /* Reduce vector comparisons to scalar masks */
    float *f_lt = (float*)&vcmp_lt;
    float *f_gt = (float*)&vcmp_gt;
    double *d_lt = (double*)&vdcmp_lt;
    
    int idx = 0;
    results[idx++] = (f_lt[0] != 0.0f) ? 45 : 0;
    results[idx++] = (f_gt[1] != 0.0f) ? 46 : 0;
    results[idx++] = (d_lt[0] != 0.0) ? 47 : 0;
    
    /* Conditional moves based on vector element comparisons */
    for (int i = 0; i < 4; i++) {
        float a = farr[i];
        float b = farr[i + 4];
        results[idx++] = (a < b) ? (48 + i) : (52 + i);
    }
    
    /* Nested loop with vector element access */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            double a = darr[i];
            double b = darr[j + 2];
            if (a > b) {
                results[idx++] = 56 + i * 2 + j;
                goto vector_label;
            }
            results[idx++] = 60 + i * 2 + j;
        }
    }
vector_label:
    results[idx++] = 64;
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int *results) {
    unsigned char byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    
    /* Inline assembly that uses condition code names */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte2)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(byte3)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=r"(byte4)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    /* Using specific condition code names */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"    /* UNORDERED: parity flag set for NaN */
        : "=r"(byte5)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"   /* ORDERED: parity flag not set */
        : "=r"(byte6)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    /* More complex assembly with multiple condition codes */
    unsigned int flags;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "pushf\n\t"
        "pop %0\n\t"
        "ucomisd %4, %5\n\t"
        "pushf\n\t"
        "pop %1"
        : "=r"(flags), "=r"(byte7)
        : "x"(f1), "x"(f2), "x"(d1), "x"(d2)
        : "cc"
    );
    
    /* Conditional move via assembly */
    int cmov_result;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "movl $100, %0\n\t"
        "cmova %3, %0"
        : "=r"(cmov_result)
        : "x"(f1), "x"(f2), "r"(200)
        : "cc"
    );
    
    results[0] = byte1;
    results[1] = byte2;
    results[2] = byte3;
    results[3] = byte4;
    results[4] = byte5;
    results[5] = byte6;
    results[6] = flags & 0xFF;
    results[7] = byte7;
    results[8] = cmov_result;
}

int main() {
    /* Initialize test data with special values */
    float fvals[8];
    double dvals[4];
    int results[128] = {0};
    
    /* Normal values */
    fvals[0] = 1.0f;
    fvals[1] = 2.0f;
    fvals[2] = -1.5f;
    fvals[3] = 3.14159f;
    
    /* Special values */
    fvals[4] = 0.0f / 0.0f;  /* NaN */
    fvals[5] = 1.0f / 0.0f;  /* +Inf */
    fvals[6] = -1.0f / 0.0f; /* -Inf */
    fvals[7] = 0.0f;
    
    /* Double values */
    dvals[0] = 1.0;
    dvals[1] = NAN;
    dvals[2] = INFINITY;
    dvals[3] = -INFINITY;
    
    /* Run all tests */
    test_scalar_cmps(fvals[0], fvals[1], dvals[0], dvals[2], &results[0]);
    test_scalar_cmps(fvals[4], fvals[5], dvals[1], dvals[3], &results[20]); /* NaN/Inf combos */
    test_builtins(fvals[0], fvals[4], dvals[0], dvals[1], &results[40]); /* Normal vs NaN */
    test_builtins(fvals[5], fvals[6], dvals[2], dvals[3], &results[60]); /* Inf vs -Inf */
    test_vector(fvals, dvals, &results[80]);
    test_asm(fvals[0], fvals[4], dvals[0], dvals[1], &results[100]); /* Normal vs NaN */
    test_asm(fvals[5], fvals[6], dvals[2], dvals[3], &results[110]); /* +Inf vs -Inf */
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        checksum += results[i];
    }
    
    /* Also use the values to affect control flow */
    volatile int dummy = 0;
    for (int i = 0; i < 8; i++) {
        if (isnan(fvals[i])) {
            dummy |= (1 << i);
        }
        if (isinf(dvals[i % 4])) {
            dummy |= (1 << (i + 8));
        }
    }
    
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
