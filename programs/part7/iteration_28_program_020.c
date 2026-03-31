#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // NEQ
    
    // Double precision comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex conditional expressions
    results[idx++] = (f1 < f2 && d1 > d2) ? 13 : 0;
    results[idx++] = (f1 >= f2 || d1 <= d2) ? 14 : 0;
    
    // Nested ternary with floating comparisons
    int val = (f1 < f2) ? ((d1 > d2) ? 15 : 16) : ((f1 == f2) ? 17 : 18);
    results[idx++] = val;
    
    // Switch statement based on comparison results
    switch((f1 < f2) + 2*(f1 > f2) + 3*(f1 == f2)) {
        case 0: results[idx++] = 19; break;
        case 1: results[idx++] = 20; break;
        case 2: results[idx++] = 21; break;
        case 3: results[idx++] = 22; break;
        default: results[idx++] = 23; break;
    }
    
    // Loop with floating-point condition
    for (int i = 0; i < 5 && f1 < f2; i++) {
        results[idx++] = 24 + i;
        f1 += 0.1f;  // Modify to potentially exit loop
    }
    
    // Goto with floating condition
    if (d1 > d2) {
        goto label1;
    }
    results[idx++] = 30;
    
label1:
    results[idx++] = 31;
    
    // Store to checksum
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test built-in unordered comparison functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct built-in calls - these should map to specific condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 100 : 0;      // GT (ordered)
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 101 : 0; // GE (ordered)
    results[idx++] = __builtin_isless(f1, f2) ? 102 : 0;         // LT (ordered)
    results[idx++] = __builtin_islessequal(f1, f2) ? 103 : 0;    // LE (ordered)
    results[idx++] = __builtin_islessgreater(f1, f2) ? 104 : 0;  // LTGT
    results[idx++] = __builtin_isunordered(f1, f2) ? 105 : 0;    // UNORDERED
    
    // Double precision versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 106 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 107 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 108 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 109 : 0;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 110 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 111 : 0;
    
    // Combined built-in calls in conditional expressions
    results[idx++] = (__builtin_isgreater(f1, f2) && __builtin_isless(d1, d2)) ? 112 : 0;
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isunordered(d1, d2)) ? 113 : 0;
    
    // Classification functions that may generate condition codes
    results[idx++] = isnan(f1) ? 114 : 0;
    results[idx++] = isinf(f1) ? 115 : 0;
    results[idx++] = isnan(d1) ? 116 : 0;
    results[idx++] = isinf(d1) ? 117 : 0;
    
    // fpclassify - may generate different patterns
    int c1 = fpclassify(f1);
    int c2 = fpclassify(d1);
    results[idx++] = (c1 == FP_NAN) ? 118 : 0;
    results[idx++] = (c2 == FP_INFINITE) ? 119 : 0;
    results[idx++] = (c1 == FP_NORMAL) ? 120 : 0;
    results[idx++] = (c2 == FP_SUBNORMAL) ? 121 : 0;
    
    // Store to checksum
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test vector/SIMD comparisons
void test_vector(float* farr, double* darr, int* results) {
    int idx = 0;
    
    // Vector float comparisons
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    
    // Vector double comparisons
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    // Perform vector comparisons - these may generate packed comparison RTL
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce vector comparisons to scalar (forces condition code generation)
    float* fcmp = (float*)&vcmp_f;
    double* dcmp = (double*)&vcmp_d;
    
    // Check each element - this creates scalar condition code checks
    for (int i = 0; i < 4; i++) {
        results[idx++] = fcmp[i] != 0.0f ? 200 + i : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dcmp[i] != 0.0 ? 204 + i : 0;
    }
    
    // More vector operations
    v4sf vcmp_f2 = vf1 == vf2;
    v2df vcmp_d2 = vd1 != vd2;
    
    float* fcmp2 = (float*)&vcmp_f2;
    double* dcmp2 = (double*)&vcmp_d2;
    
    for (int i = 0; i < 4; i++) {
        results[idx++] = fcmp2[i] != 0.0f ? 206 + i : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dcmp2[i] != 0.0 ? 210 + i : 0;
    }
    
    // Vector built-in (emulated)
    int any_unordered = 0;
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(farr[i], farr[i+4])) {
            any_unordered = 1;
        }
    }
    results[idx++] = any_unordered ? 212 : 0;
    
    // Store to checksum
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    unsigned char byte_result;
    
    // Inline assembly that uses condition code names
    // These should force the assembly printer to resolve symbolic condition codes
    
    // Test various condition codes via SETcc instructions
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 300 : 0;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 301 : 0;
    
    // Test UNORDERED condition code
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 302 : 0;
    
    // Test ORDERED condition code
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 303 : 0;
    
    // Test UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 304 : 0;
    
    // Test UNGE (not less than)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 305 : 0;
    
    // Test UNGT (not less or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 306 : 0;
    
    // Test UNLE (unordered or less or equal)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 307 : 0;
    
    // Test UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = byte_result ? 308 : 0;
    
    // Test LTGT (less or greater, ordered)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte_result ? 309 : 0;
    
    // CMOVcc example using inline assembly
    int val1 = 310, val2 = 311;
    int cmov_result;
    __asm__ volatile (
        "ucomiss %3, %4\n\t"
        "cmova %1, %2\n\t"
        "mov %2, %0"
        : "=r" (cmov_result)
        : "r" (val1), "r" (val2), "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = cmov_result;
    
    // Store to checksum
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

int main() {
    // Initialize test data with normal and special values
    float fvals[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        NAN, INFINITY, -INFINITY, 0.0f
    };
    
    double dvals[] = {
        1.0, 2.0, 3.0, 4.0,
        NAN, INFINITY, -INFINITY, 0.0
    };
    
    // Results arrays
    int results1[50] = {0};
    int results2[50] = {0};
    int results3[50] = {0};
    int results4[50] = {0};
    
    // Run tests with different combinations of values
    // Test 1: Normal values
    test_scalar_cmps(fvals[0], fvals[1], dvals[0], dvals[1], results1);
    test_builtins(fvals[0], fvals[1], dvals[0], dvals[1], results1 + 25);
    
    // Test 2: Special values (NaN, Inf)
    test_scalar_cmps(fvals[8], fvals[9], dvals[4], dvals[5], results2);
    test_builtins(fvals[8], fvals[9], dvals[4], dvals[5], results2 + 25);
    
    // Test 3: Vector operations
    test_vector(fvals, dvals, results3);
    
    // Test 4: Inline assembly
    test_asm(fvals[0], fvals[1], dvals[0], dvals[1], results4);
    test_asm(fvals[8], fvals[9], dvals[4], dvals[5], results4 + 15);
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    // Also print some results to ensure they're used
    printf("Result samples: %d %d %d %d\n", 
           results1[0], results2[0], results3[0], results4[0]);
    
    return 0;
}
