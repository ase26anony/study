#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

static volatile int checksum = 0;

// Test scalar comparisons with mixed float/double types
void test_scalar_cmps(float *farr, double *darr, int *results) {
    int idx = 0;
    
    // Test all relational operators with special values
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[(i+1)&7];
        double d1 = darr[i];
        double d2 = darr[(i+3)&7];
        
        // UNORDERED cases (NaN comparisons)
        results[idx++] = (f1 != f1) ? 1 : 0;           // isnan(f1)
        results[idx++] = (d1 != d1) ? 2 : 0;           // isnan(d1)
        results[idx++] = (f1 < f2) ? 3 : 4;            // Potential UNLT/UNGE
        results[idx++] = (d1 > d2) ? 5 : 6;            // Potential UNGT/UNLE
        
        // ORDERED cases
        results[idx++] = (f1 == f1 && f2 == f2) ? 7 : 8;  // Both ordered
        results[idx++] = (d1 <= d2) ? 9 : 10;             // UNLE/UNGT
        results[idx++] = (f1 >= f2) ? 11 : 12;            // UNGE/UNLT
        
        // UNEQ cases (unordered or equal)
        results[idx++] = !(f1 < f2) && !(f2 < f1) ? 13 : 14;
        results[idx++] = !(d1 > d2) && !(d2 > d1) ? 15 : 16;
        
        // LTGT cases (less or greater, but not equal/unordered)
        results[idx++] = (f1 < f2 || f1 > f2) ? 17 : 18;
        results[idx++] = (d1 < d2 || d1 > d2) ? 19 : 20;
    }
    
    // Complex control flow with nested conditionals
    for (int i = 0; i < 4; i++) {
        float f = farr[i];
        double d = darr[i+4];
        
        switch (fpclassify(f)) {
            case FP_NAN:
                results[idx++] = (d == d) ? 100 : 101;  // ORDERED test
                if (isinf(d)) goto nan_label;
                break;
            case FP_INFINITE:
                results[idx++] = (f > 0) ? 200 : 201;   // UNGT/UNLE
                continue;
            default:
                if (isnan(d)) {
                    results[idx++] = (f < farr[i+1]) ? 300 : 301;  // UNLT/UNGE
                } else {
                    results[idx++] = (d <= darr[i+2]) ? 400 : 401; // UNLE/UNGT
                }
                break;
        }
        
        nan_label:
        results[idx++] = (f != f || d != d) ? 500 : 501;  // UNORDERED test
    }
}

// Test builtin unordered comparison functions
void test_builtins(float *farr, double *darr, int *results) {
    int idx = 0;
    
    for (int i = 0; i < 8; i += 2) {
        float f1 = farr[i];
        float f2 = farr[i+1];
        double d1 = darr[i];
        double d2 = darr[i+1];
        
        // Direct builtin calls that map to condition codes
        results[idx++] = __builtin_isunordered(f1, f2) ? 1 : 0;    // UNORDERED
        results[idx++] = __builtin_isgreater(f1, f2) ? 2 : 0;      // UNLE inverse
        results[idx++] = __builtin_isless(d1, d2) ? 3 : 0;         // UNGE inverse
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // UNGT inverse
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 5 : 0; // UNLT inverse
        
        // Combined tests for ORDERED
        results[idx++] = (!__builtin_isunordered(f1, f2)) ? 6 : 0; // ORDERED
        
        // UNEQ via builtins
        results[idx++] = (!__builtin_isless(f1, f2) && 
                         !__builtin_isgreater(f1, f2)) ? 7 : 0;
        
        // LTGT via builtins
        results[idx++] = (__builtin_isless(d1, d2) || 
                         __builtin_isgreater(d1, d2)) ? 8 : 0;
    }
}

// Test vector comparisons
void test_vector(float *farr, double *darr, int *results) {
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    // Vector comparisons generate packed condition codes
    v4sf vcmpf = vf1 < vf2;
    v2df vcmpd = vd1 > vd2;
    
    // Reduce to scalar - forces condition code materialization
    int maskf = 0, maskd = 0;
    for (int i = 0; i < 4; i++) {
        maskf |= (((int*)&vcmpf)[i] != 0) << i;
    }
    for (int i = 0; i < 2; i++) {
        maskd |= (((int64_t*)&vcmpd)[i] != 0) << i;
    }
    
    results[0] = maskf;
    results[1] = maskd;
    
    // More vector ops with different comparisons
    vcmpf = vf1 >= vf2;  // UNGE/UNLT
    vcmpd = vd1 <= vd2;  // UNLE/UNGT
    
    // Test equality/inequality
    v4sf veqf = vf1 == vf2;  // UNEQ/LTGT
    v2df vneqd = vd1 != vd2; // LTGT/UNEQ
    
    // Mix with scalar operations
    float fsum = 0;
    double dsum = 0;
    for (int i = 0; i < 4; i++) {
        fsum += ((float*)&vcmpf)[i] ? farr[i] : 0;
        if (i < 2) {
            dsum += ((double*)&vcmpd)[i] ? darr[i] : 0;
        }
    }
    
    results[2] = (int)fsum;
    results[3] = (int)dsum;
}

// Test inline assembly with condition codes
void test_asm(float *farr, double *darr, int *results) {
    int idx = 0;
    
    for (int i = 0; i < 4; i++) {
        float f1 = farr[i];
        float f2 = farr[i+4];
        double d1 = darr[i];
        double d2 = darr[i+4];
        
        int byte1, byte2, byte3, byte4;
        
        // Test various condition codes in inline asm
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "seta %0"
            : "=r"(byte1) : "x"(f1), "x"(f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0"
            : "=r"(byte2) : "x"(d1), "x"(d2) : "cc");
        
        // Test UNORDERED/ORDERED explicitly
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setp %0"
            : "=r"(byte3) : "x"(f1), "x"(f2) : "cc");
        
        // Test UNEQ (ZF=1 or PF=1)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0"
            : "=r"(byte4) : "x"(d1), "x"(d2) : "cc");
        
        results[idx++] = byte1;
        results[idx++] = byte2;
        results[idx++] = byte3;
        results[idx++] = byte4;
        
        // More complex asm with multiple condition codes
        int cmov_result;
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "cmova %3, %0\n\t"
            "cmovbe %4, %0"
            : "=r"(cmov_result) 
            : "x"(f1), "x"(f2), "r"(100), "r"(200) 
            : "cc");
        results[idx++] = cmov_result;
    }
}

int main() {
    // Initialize arrays with special values
    float farr[8];
    double darr[8];
    int results[256] = {0};
    
    // Mix of normal and special values
    farr[0] = 1.0f;
    farr[1] = NAN;
    farr[2] = INFINITY;
    farr[3] = -INFINITY;
    farr[4] = 2.0f;
    farr[5] = -NAN;
    farr[6] = 0.0f;
    farr[7] = -1.0f;
    
    darr[0] = 1.0;
    darr[1] = NAN;
    darr[2] = INFINITY;
    darr[3] = -INFINITY;
    darr[4] = 2.0;
    darr[5] = -NAN;
    darr[6] = 0.0;
    darr[7] = -1.0;
    
    // Run all tests
    test_scalar_cmps(farr, darr, results);
    test_builtins(farr, darr, results + 100);
    test_vector(farr, darr, results + 150);
    test_asm(farr, darr, results + 200);
    
    // Compute checksum to prevent dead code elimination
    for (int i = 0; i < 256; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
