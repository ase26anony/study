#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 32

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
volatile int global_checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float* farr, double* darr, int* results) {
    int idx = 0;
    
    // Test all relational operators with mixed float/double
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float f1 = farr[i];
        float f2 = farr[(i + 1) % ARRAY_SIZE];
        double d1 = darr[i];
        double d2 = darr[(i + 1) % ARRAY_SIZE];
        
        // UNORDERED/ORDERED cases (NaN comparisons)
        results[idx++] = (f1 != f1) ? 1 : 0;  // isnan check
        results[idx++] = (d1 != d1) ? 2 : 0;
        results[idx++] = (f1 == f1) ? 3 : 0;  // !isnan check
        
        // UNEQ case (unordered or equal)
        results[idx++] = (f1 == f2) ? 4 : 0;
        results[idx++] = (d1 == d2) ? 5 : 0;
        
        // UNGE case (unordered or greater or equal) -> "nlt"
        results[idx++] = (f1 >= f2) ? 6 : 0;
        results[idx++] = (d1 >= d2) ? 7 : 0;
        
        // UNGT case (unordered or greater) -> "nle"
        results[idx++] = (f1 > f2) ? 8 : 0;
        results[idx++] = (d1 > d2) ? 9 : 0;
        
        // UNLE case (unordered or less or equal)
        results[idx++] = (f1 <= f2) ? 10 : 0;
        results[idx++] = (d1 <= d2) ? 11 : 0;
        
        // UNLT case (unordered or less)
        results[idx++] = (f1 < f2) ? 12 : 0;
        results[idx++] = (d1 < d2) ? 13 : 0;
        
        // LTGT case (less or greater) -> "une"
        results[idx++] = (f1 < f2 || f1 > f2) ? 14 : 0;
        results[idx++] = (d1 < d2 || d1 > d2) ? 15 : 0;
        
        // Complex conditional with mixed types
        results[idx++] = (f1 < d1) ? 16 : 0;
        results[idx++] = (d2 > f2) ? 17 : 0;
    }
}

// Test builtin functions
void test_builtins(float* farr, double* darr, int* results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        float f1 = farr[i];
        float f2 = farr[i + 1];
        double d1 = darr[i];
        double d2 = darr[i + 1];
        
        // __builtin_isunordered - directly maps to UNORDERED
        results[idx++] = __builtin_isunordered(f1, f2) ? 100 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 101 : 0;
        
        // __builtin_isgreater - ordered greater
        results[idx++] = __builtin_isgreater(f1, f2) ? 102 : 0;
        results[idx++] = __builtin_isgreater(d1, d2) ? 103 : 0;
        
        // __builtin_isless - ordered less
        results[idx++] = __builtin_isless(f1, f2) ? 104 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 105 : 0;
        
        // __builtin_isgreaterequal
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 106 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 107 : 0;
        
        // __builtin_islessequal
        results[idx++] = __builtin_islessequal(f1, f2) ? 108 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 109 : 0;
        
        // __builtin_islessgreater - LTGT case
        results[idx++] = __builtin_islessgreater(f1, f2) ? 110 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 111 : 0;
    }
}

// Test vector comparisons
void test_vector(float* farr, double* darr, int* results) {
    v4sf vf1, vf2;
    v2df vd1, vd2;
    
    // Load vectors
    memcpy(&vf1, farr, sizeof(v4sf));
    memcpy(&vf2, farr + 4, sizeof(v4sf));
    memcpy(&vd1, darr, sizeof(v2df));
    memcpy(&vd2, darr + 2, sizeof(v2df));
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce to scalar mask
    int* fcmp = (int*)&vcmp_f;
    int* dcmp = (int*)&vcmp_d;
    
    results[0] = fcmp[0] | fcmp[1] | fcmp[2] | fcmp[3];
    results[1] = dcmp[0] | dcmp[1];
    
    // More vector comparisons
    vcmp_f = vf1 == vf2;
    vcmp_d = vd1 != vd2;
    
    fcmp = (int*)&vcmp_f;
    dcmp = (int*)&vcmp_d;
    
    results[2] = fcmp[0] & fcmp[1] & fcmp[2] & fcmp[3];
    results[3] = dcmp[0] & dcmp[1];
}

// Test inline assembly with condition codes
void test_asm(float* farr, double* darr, int* results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        float f1 = farr[i];
        float f2 = farr[i + 1];
        double d1 = darr[i];
        double d2 = darr[i + 1];
        
        int r1, r2, r3, r4;
        
        // Inline assembly with condition code constraints
        // These force the assembly printer to resolve condition codes
        
        // UNORDERED case
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setp %0"
            : "=r"(r1)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        results[idx++] = r1;
        
        // ORDERED case
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnp %0"
            : "=r"(r2)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        results[idx++] = r2;
        
        // UNEQ case
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "sete %0"
            : "=r"(r3)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        results[idx++] = r3;
        
        // UNGE case -> "nlt"
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnb %0"
            : "=r"(r4)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        results[idx++] = r4;
        
        // More assembly tests with different condition codes
        int r5, r6, r7, r8;
        
        // UNGT case -> "nle"
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setnbe %0"
            : "=r"(r5)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        results[idx++] = r5;
        
        // UNLE case
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setbe %0"
            : "=r"(r6)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        results[idx++] = r6;
        
        // UNLT case
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setb %0"
            : "=r"(r7)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        results[idx++] = r7;
        
        // LTGT case -> "une"
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setne %0"
            : "=r"(r8)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        results[idx++] = r8;
    }
}

// Complex control flow with condition codes
void test_control_flow(float* farr, double* darr, int* results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float f = farr[i];
        double d = darr[i];
        
        // Complex switch with floating comparisons
        switch (i % 8) {
            case 0:
                if (f != f) {  // isnan
                    results[idx++] = 200;
                    goto label1;
                }
                break;
            case 1:
                if (d == d) {  // !isnan
                    results[idx++] = 201;
                    continue;
                }
                break;
            case 2:
                if (f < d) {
                    results[idx++] = 202;
                    for (int j = 0; j < 3; j++) {
                        if (farr[j] > darr[j]) {
                            results[idx++] = 203;
                            break;
                        }
                    }
                }
                break;
            case 3:
                if (d > f) {
                    results[idx++] = 204;
                    while (idx < ARRAY_SIZE && farr[idx % ARRAY_SIZE] <= darr[idx % ARRAY_SIZE]) {
                        idx++;
                    }
                }
                break;
            case 4:
                // Nested conditionals
                results[idx++] = (f >= 0.0f) ? 
                    ((d <= 0.0) ? 205 : 206) : 
                    ((f == -0.0f) ? 207 : 208);
                break;
            case 5:
                // Complex expression
                results[idx++] = (f < 0.0f && d > 0.0) ? 209 : 210;
                break;
            case 6:
                // Classification functions
                results[idx++] = (fpclassify(f) == FP_NAN) ? 211 : 212;
                results[idx++] = (isinf(d)) ? 213 : 214;
                break;
            case 7:
                // Mixed comparisons
                results[idx++] = (f == 0.0f && d != 0.0) ? 215 : 216;
                break;
        }
    label1:
        // Empty label for goto
        ;
    }
}

int main() {
    // Initialize arrays with special values
    float farr[ARRAY_SIZE];
    double darr[ARRAY_SIZE];
    int results[ARRAY_SIZE * 10];  // Large enough for all tests
    
    // Fill arrays with mixed values including NaN, Inf
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float fval = (float)i;
        double dval = (double)i;
        
        // Insert special values at specific indices
        if (i % 7 == 0) fval = NAN;
        if (i % 11 == 0) dval = NAN;
        if (i % 5 == 0) fval = INFINITY;
        if (i % 13 == 0) dval = -INFINITY;
        if (i % 3 == 0) fval = -0.0f;
        if (i % 17 == 0) dval = 0.0;
        
        farr[i] = fval;
        darr[i] = dval;
    }
    
    // Run all tests
    memset(results, 0, sizeof(results));
    test_scalar_cmps(farr, darr, results);
    
    int* r2 = results + ARRAY_SIZE * 8;
    test_builtins(farr, darr, r2);
    
    int* r3 = r2 + ARRAY_SIZE * 3;
    test_vector(farr, darr, r3);
    
    int* r4 = r3 + 4;
    test_asm(farr, darr, r4);
    
    int* r5 = r4 + ARRAY_SIZE * 2;
    test_control_flow(farr, darr, r5);
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE * 10; i++) {
        checksum ^= results[i];
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
