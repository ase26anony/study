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
    
    /* Complex control flow with condition codes */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2) ? 13 : 0;
                break;
            case 1:
                results[idx++] = (d1 > d2) ? 14 : 0;
                break;
            case 2:
                results[idx++] = (f1 == f2) ? 15 : 0;
                if (d1 != d2) {
                    results[idx++] = 16;
                    goto label1;
                }
                break;
        }
    }
label1:
    
    /* Nested if-else chains */
    if (f1 < f2) {
        if (d1 > d2) {
            results[idx++] = 17;
        } else if (d1 <= d2) {
            results[idx++] = 18;
        }
    } else if (f1 > f2) {
        results[idx++] = 19;
    } else if (f1 == f2) {
        results[idx++] = 20;
    }
}

/* Test builtin unordered comparisons */
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    /* Builtins that directly map to condition codes */
    results[idx++] = __builtin_isgreater(f1, f2) ? 21 : 0;      /* UNGT/GT */
    results[idx++] = __builtin_isless(f1, f2) ? 22 : 0;         /* UNLT/LT */
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 23 : 0; /* UNGE/GE */
    results[idx++] = __builtin_islessequal(f1, f2) ? 24 : 0;    /* UNLE/LE */
    results[idx++] = __builtin_isunordered(f1, f2) ? 25 : 0;    /* UNORDERED */
    
    /* Double precision builtins */
    results[idx++] = __builtin_isgreater(d1, d2) ? 26 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 27 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 28 : 0;
    
    /* Classification functions */
    results[idx++] = isnan(f1) ? 29 : 0;
    results[idx++] = isinf(d1) ? 30 : 0;
    results[idx++] = fpclassify(f2) == FP_NAN ? 31 : 0;
    
    /* Mixed comparisons in complex expressions */
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(f1, f2)) {
            results[idx++] = 32 + i;
            continue;
        }
        if (__builtin_isgreater(d1, d2)) {
            results[idx++] = 36 + i;
            break;
        }
    }
}

/* Test vector/SIMD comparisons */
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f2, f1, f2};
    v4sf vf2 = {f2, f1, f2, f1};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    /* Vector comparisons - may generate packed comparisons */
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v2df cmp3 = vd1 <= vd2;
    v2df cmp4 = vd1 >= vd2;
    
    /* Reduce vector comparisons to scalar */
    int mask1 = 0;
    int mask2 = 0;
    int mask3 = 0;
    int mask4 = 0;
    
    for (int i = 0; i < 4; i++) {
        mask1 |= (((int*)&cmp1)[i] != 0) << i;
        mask2 |= (((int*)&cmp2)[i] != 0) << i;
    }
    
    for (int i = 0; i < 2; i++) {
        mask3 |= (((int64_t*)&cmp3)[i] != 0) << i;
        mask4 |= (((int64_t*)&cmp4)[i] != 0) << i;
    }
    
    results[0] = mask1;
    results[1] = mask2;
    results[2] = mask3;
    results[3] = mask4;
    
    /* Conditional moves based on vector comparisons */
    int* ptr = results + 4;
    *ptr = (mask1 & 1) ? 40 : 41;
    ptr++;
    *ptr = (mask3 & 2) ? 42 : 43;
}

/* Test inline assembly with condition codes */
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    unsigned char byte1, byte2, byte3, byte4;
    int idx = 0;
    
    /* Compare floats and set byte based on condition */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r" (byte1)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte1;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r" (byte2)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte2;
    
    /* Test unordered/ordered conditions */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r" (byte3)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte3;
    
    /* Double precision comparisons */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=r" (byte4)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte4;
    
    /* More condition codes using symbolic names */
    int result5, result6;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "setg %0\n\t"
        "setl %1"
        : "=r" (result5), "=r" (result6)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = result5;
    results[idx++] = result6;
}

int main() {
    /* Initialize test data with special values */
    float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f};
    double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0};
    
    int results[100];
    memset(results, 0, sizeof(results));
    
    /* Test various combinations of values */
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            int local_results[50];
            
            /* Call all test functions */
            test_scalar_cmps(f1, f2, d1, d2, local_results);
            test_builtins(f1, f2, d1, d2, local_results + 20);
            test_vector(f1, f2, d1, d2, local_results + 30);
            test_asm(f1, f2, d1, d2, local_results + 40);
            
            /* Update checksum to prevent optimization */
            for (int k = 0; k < 50; k++) {
                checksum ^= local_results[k];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
