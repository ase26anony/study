#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar comparisons with all relational operators
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT or LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT or GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE or LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE or GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ or EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT or NE
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex conditional expressions with mixed types
    results[idx++] = (f1 < f2 && d1 > d2) ? 13 : 14;
    results[idx++] = (f1 == f2 || d1 != d2) ? 15 : 16;
    
    // Nested ternary with floating comparisons
    results[idx++] = (f1 < f2) ? ((d1 > d2) ? 17 : 18) : ((d1 == d2) ? 19 : 20);
    
    // Store to global to prevent optimization
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Builtins that directly map to condition codes
    results[idx++] = __builtin_isunordered(f1, f2) ? 21 : 22;    // UNORDERED
    results[idx++] = __builtin_isgreater(f1, f2) ? 23 : 24;      // UNLE (inverted)
    results[idx++] = __builtin_isless(f1, f2) ? 25 : 26;         // UNGE (inverted)
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 27 : 28; // UNLT (inverted)
    results[idx++] = __builtin_islessequal(f1, f2) ? 29 : 30;    // UNGT (inverted)
    
    // Double versions
    results[idx++] = __builtin_isunordered(d1, d2) ? 31 : 32;
    results[idx++] = __builtin_isgreater(d1, d2) ? 33 : 34;
    results[idx++] = __builtin_isless(d1, d2) ? 35 : 36;
    
    // Classification functions
    results[idx++] = isnan(f1) ? 37 : 38;
    results[idx++] = isinf(f1) ? 39 : 40;
    results[idx++] = isnan(d1) ? 41 : 42;
    results[idx++] = isinf(d1) ? 43 : 44;
    
    // fpclassify with comparisons
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 45 : 46;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 47 : 48;
    
    // Complex control flow with builtins
    if (__builtin_isunordered(f1, f2)) {
        results[idx++] = 49;
        if (__builtin_isgreater(d1, d2)) {
            results[idx++] = 50;
        } else {
            results[idx++] = 51;
        }
    } else {
        results[idx++] = 52;
    }
    
    // Store to global
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test vector comparisons
void test_vector(v4sf vf1, v4sf vf2, v2df vd1, v2df vd2, int* results) {
    int idx = 0;
    
    // Vector comparisons
    v4sf vcmp_lt = vf1 < vf2;
    v4sf vcmp_gt = vf1 > vf2;
    v4sf vcmp_eq = vf1 == vf2;
    v4sf vcmp_ne = vf1 != vf2;
    
    // Reduce to scalar mask
    float* fcmp_lt = (float*)&vcmp_lt;
    float* fcmp_gt = (float*)&vcmp_gt;
    
    // Check each element with complex control flow
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                results[idx++] = (fcmp_lt[i] != 0.0f) ? 53 : 54;
                break;
            case 1:
                results[idx++] = (fcmp_gt[i] != 0.0f) ? 55 : 56;
                break;
            case 2:
                results[idx++] = (fcmp_lt[i] != 0.0f && fcmp_gt[i] != 0.0f) ? 57 : 58;
                break;
            case 3:
                results[idx++] = (fcmp_lt[i] == 0.0f || fcmp_gt[i] == 0.0f) ? 59 : 60;
                break;
        }
    }
    
    // Double vector comparisons
    v2df vdcmp = vd1 < vd2;
    double* dcmp = (double*)&vdcmp;
    
    // Loop with break/continue
    for (int i = 0; i < 2; i++) {
        if (dcmp[i] != 0.0) {
            results[idx++] = 61 + i;
            continue;
        } else {
            results[idx++] = 63 + i;
            if (i == 1) break;
        }
    }
    
    // Store to global
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    uint8_t byte_result;
    
    // Test various condition codes via inline assembly
    // Using 'g' constraint to let compiler choose register
    
    // UNORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 65 : 66;
    
    // ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 67 : 68;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 69 : 70;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  // swapped for nlt
        "setnb %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 71 : 72;
    
    // UNGT (not less or equal)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  // swapped for nle
        "setnbe %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 73 : 74;
    
    // UNLE (unordered or less or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 75 : 76;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=g" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 77 : 78;
    
    // LTGT (less than or greater than, but not equal and not unordered)
    // This is "not equal and ordered" which is "setne" after ordered check
    uint8_t ordered, nequal;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "setnp %0\n\t"
        "setne %1"
        : "=g" (ordered), "=g" (nequal)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = (ordered && nequal) ? 79 : 80;
    
    // Double precision version
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=g" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 81 : 82;
    
    // Store to global
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Complex control flow test with nested conditionals
void test_complex_flow(float* farr, double* darr, int size, int* results) {
    int idx = 0;
    
    for (int i = 0; i < size; i++) {
        // Nested if-else with floating comparisons
        if (farr[i] < darr[i]) {
            results[idx++] = 100 + i;
            if (isnan(farr[i])) {
                results[idx++] = 200 + i;
                goto skip_double_check;
            }
        } else if (farr[i] > darr[i]) {
            results[idx++] = 300 + i;
            if (isinf(darr[i])) {
                results[idx++] = 400 + i;
                continue;
            }
        } else if (farr[i] == darr[i]) {
            results[idx++] = 500 + i;
        } else {
            // unordered case
            results[idx++] = 600 + i;
        }
        
        skip_double_check:
        // Switch based on classification
        switch (fpclassify(darr[i])) {
            case FP_NAN:
                results[idx++] = 700 + i;
                break;
            case FP_INFINITE:
                results[idx++] = 800 + i;
                // Fall through
            case FP_ZERO:
                results[idx++] = 900 + i;
                break;
            case FP_SUBNORMAL:
                results[idx++] = 1000 + i;
                break;
            case FP_NORMAL:
                results[idx++] = 1100 + i;
                break;
            default:
                results[idx++] = 1200 + i;
        }
        
        // Early exit on certain conditions
        if (__builtin_isunordered(farr[i], farr[(i+1)%size])) {
            if (i > size/2) break;
        }
    }
    
    // Store to global
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

int main() {
    // Initialize test data with special values
    float fvals[] = {1.0f, -2.0f, NAN, INFINITY, -INFINITY, 0.0f, 3.14f};
    double dvals[] = {1.0, -2.0, NAN, INFINITY, -INFINITY, 0.0, 3.1415926535};
    
    // Vector data
    v4sf vf1 = {1.0f, 2.0f, NAN, 4.0f};
    v4sf vf2 = {0.0f, 2.0f, 3.0f, INFINITY};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {2.0, INFINITY};
    
    // Results arrays
    int results1[50], results2[50], results3[50], results4[50], results5[200];
    memset(results1, 0, sizeof(results1));
    memset(results2, 0, sizeof(results2));
    memset(results3, 0, sizeof(results3));
    memset(results4, 0, sizeof(results4));
    memset(results5, 0, sizeof(results5));
    
    // Run all tests with different combinations of values
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            test_scalar_cmps(fvals[i], fvals[j], dvals[i], dvals[j], results1);
            test_builtins(fvals[i], fvals[j], dvals[i], dvals[j], results2);
            
            if (i < 4 && j < 4) {
                // Adjust vector indices
                v4sf temp_vf1 = {fvals[i], fvals[(i+1)%7], fvals[(i+2)%7], fvals[(i+3)%7]};
                v4sf temp_vf2 = {fvals[j], fvals[(j+1)%7], fvals[(j+2)%7], fvals[(j+3)%7]};
                test_vector(temp_vf1, temp_vf2, vd1, vd2, results3);
            }
            
            test_asm(fvals[i], fvals[j], dvals[i], dvals[j], results4);
        }
    }
    
    // Test complex control flow
    test_complex_flow(fvals, dvals, 7, results5);
    
    // Final checksum computation and output
    int final_checksum = checksum;
    for (int i = 0; i < 50; i++) {
        final_checksum += results1[i] + results2[i] + results3[i] + results4[i];
    }
    for (int i = 0; i < 200; i++) {
        final_checksum += results5[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    return 0;
}
