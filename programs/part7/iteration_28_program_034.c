#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 32

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent optimization
volatile int global_checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float *fa, double *da, int *results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float f1 = fa[i];
        float f2 = fa[(i + 1) % ARRAY_SIZE];
        double d1 = da[i];
        double d2 = da[(i + 2) % ARRAY_SIZE];
        
        // Generate various condition codes through ternary operators
        // These should compile to CMOV or SET instructions
        
        // UNORDERED cases (NaN comparisons)
        results[idx++] = (f1 != f1) ? 1 : 0;  // isnan check
        results[idx++] = (d1 != d1) ? 2 : 0;
        results[idx++] = (f1 < f2) ? (isnan(f1) ? 3 : 4) : 5;
        
        // ORDERED cases
        results[idx++] = (f1 == f1 && f2 == f2) ? 6 : 0;
        results[idx++] = (d1 == d1 || d2 == d2) ? 7 : 0;
        
        // UNEQ (unordered or equal)
        results[idx++] = !(f1 > f2) && !(f1 < f2) ? 8 : 0;
        results[idx++] = !(d1 > d2) && !(d1 < d2) ? 9 : 0;
        
        // UNGE (not less than)
        results[idx++] = !(f1 < f2) ? 10 : 0;
        results[idx++] = !(d1 < d2) ? 11 : 0;
        
        // UNGT (not less than or equal)
        results[idx++] = !(f1 <= f2) ? 12 : 0;
        results[idx++] = !(d1 <= d2) ? 13 : 0;
        
        // UNLE (unordered or less than or equal)
        results[idx++] = (f1 <= f2) || (f1 != f1) || (f2 != f2) ? 14 : 0;
        results[idx++] = (d1 <= d2) || (d1 != d1) || (d2 != d2) ? 15 : 0;
        
        // UNLT (unordered or less than)
        results[idx++] = (f1 < f2) || (f1 != f1) || (f2 != f2) ? 16 : 0;
        results[idx++] = (d1 < d2) || (d1 != d1) || (d2 != d2) ? 17 : 0;
        
        // LTGT (less than or greater than)
        results[idx++] = (f1 < f2) || (f1 > f2) ? 18 : 0;
        results[idx++] = (d1 < d2) || (d1 > d2) ? 19 : 0;
        
        // Complex nested conditionals to force code generation
        if (f1 < f2) {
            if (d1 > d2) {
                results[idx++] = 20;
            } else if (d1 == d2) {
                results[idx++] = 21;
            } else {
                results[idx++] = isnan(d1) ? 22 : 23;
            }
        } else if (f1 > f2) {
            results[idx++] = isinf(f1) ? 24 : 25;
        } else {
            switch (fpclassify(f1)) {
                case FP_NAN: results[idx++] = 26; break;
                case FP_INFINITE: results[idx++] = 27; break;
                case FP_ZERO: results[idx++] = 28; break;
                case FP_SUBNORMAL: results[idx++] = 29; break;
                default: results[idx++] = 30; break;
            }
        }
    }
}

// Test builtin functions
void test_builtins(float *fa, double *da, int *results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        float f1 = fa[i];
        float f2 = fa[i + 1];
        double d1 = da[i];
        double d2 = da[i + 1];
        
        // Direct use of builtins that generate condition codes
        results[idx++] = __builtin_isunordered(f1, f2) ? 100 : 0;
        results[idx++] = __builtin_isgreater(f1, f2) ? 101 : 0;
        results[idx++] = __builtin_isless(f1, f2) ? 102 : 0;
        results[idx++] = __builtin_islessequal(f1, f2) ? 103 : 0;
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 104 : 0;
        
        results[idx++] = __builtin_isunordered(d1, d2) ? 105 : 0;
        results[idx++] = __builtin_isgreater(d1, d2) ? 106 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 107 : 0;
        
        // Combined builtins for complex conditions
        results[idx++] = __builtin_isunordered(f1, f2) || __builtin_isgreater(d1, d2) ? 108 : 0;
        results[idx++] = !__builtin_isless(f1, f2) && !__builtin_isunordered(d1, d2) ? 109 : 0;
        
        // Nested ternary with builtins
        results[idx++] = __builtin_isunordered(f1, f2) ? 
                        (__builtin_isgreater(d1, d2) ? 110 : 111) :
                        (__builtin_isless(f1, f2) ? 112 : 113);
    }
}

// Test vector comparisons
void test_vector(float *fa, double *da, int *results) {
    v4sf vf1, vf2, vcmp;
    v2df vd1, vd2;
    int idx = 0;
    
    // Load vector data
    for (int i = 0; i < 4; i++) {
        vf1[i] = fa[i];
        vf2[i] = fa[i + 4];
    }
    for (int i = 0; i < 2; i++) {
        vd1[i] = da[i];
        vd2[i] = da[i + 2];
    }
    
    // Vector comparisons - these generate packed comparison RTL
    vcmp = vf1 < vf2;
    results[idx++] = vcmp[0] ? 200 : 0;
    results[idx++] = vcmp[1] ? 201 : 0;
    
    vcmp = vf1 > vf2;
    results[idx++] = vcmp[2] ? 202 : 0;
    results[idx++] = vcmp[3] ? 203 : 0;
    
    vcmp = vf1 <= vf2;
    results[idx++] = vcmp[0] ? 204 : 0;
    
    vcmp = vf1 >= vf2;
    results[idx++] = vcmp[1] ? 205 : 0;
    
    vcmp = vf1 == vf2;
    results[idx++] = vcmp[2] ? 206 : 0;
    
    vcmp = vf1 != vf2;
    results[idx++] = vcmp[3] ? 207 : 0;
    
    // Double vector comparisons
    v2df vdcmp = vd1 < vd2;
    results[idx++] = vdcmp[0] ? 208 : 0;
    results[idx++] = vdcmp[1] ? 209 : 0;
    
    // Mixed comparisons in loop
    for (int i = 0; i < ARRAY_SIZE - 4; i += 4) {
        v4sf temp1, temp2;
        for (int j = 0; j < 4; j++) {
            temp1[j] = fa[i + j];
            temp2[j] = fa[i + j + 1];
        }
        
        vcmp = temp1 < temp2;
        for (int j = 0; j < 4; j++) {
            results[idx++] = vcmp[j] ? (210 + j) : 0;
        }
        
        // Break and continue to create complex CFG
        if (vcmp[0]) {
            results[idx++] = 214;
            continue;
        }
        
        if (vcmp[1] && vcmp[2]) {
            results[idx++] = 215;
            break;
        }
        
        results[idx++] = 216;
    }
}

// Test inline assembly with condition codes
void test_asm(float *fa, double *da, int *results) {
    int idx = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float f1 = fa[i];
        float f2 = fa[(i + 1) % ARRAY_SIZE];
        double d1 = da[i];
        double d2 = da[(i + 2) % ARRAY_SIZE];
        
        unsigned char byte_result;
        
        // Inline assembly that uses condition code names
        // These force the assembly printer to resolve the symbolic names
        
        // UNORDERED
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setp %0"
            : "=r" (byte_result)
            : "x" (f1), "x" (f2)
        );
        results[idx++] = byte_result ? 300 : 0;
        
        // ORDERED
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnp %0"
            : "=r" (byte_result)
            : "x" (d1), "x" (d2)
        );
        results[idx++] = byte_result ? 301 : 0;
        
        // UNEQ (unordered or equal)
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "sete %0"
            : "=r" (byte_result)
            : "x" (f1), "x" (f2)
        );
        results[idx++] = byte_result ? 302 : 0;
        
        // UNGE (not less than)
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnb %0"
            : "=r" (byte_result)
            : "x" (d1), "x" (d2)
        );
        results[idx++] = byte_result ? 303 : 0;
        
        // UNGT (not less than or equal)
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setnbe %0"
            : "=r" (byte_result)
            : "x" (f1), "x" (f2)
        );
        results[idx++] = byte_result ? 304 : 0;
        
        // UNLE (unordered or less than or equal)
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setbe %0"
            : "=r" (byte_result)
            : "x" (d1), "x" (d2)
        );
        results[idx++] = byte_result ? 305 : 0;
        
        // UNLT (unordered or less than)
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setb %0"
            : "=r" (byte_result)
            : "x" (f1), "x" (f2)
        );
        results[idx++] = byte_result ? 306 : 0;
        
        // LTGT (less than or greater than)
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setne %0"
            : "=r" (byte_result)
            : "x" (d1), "x" (d2)
        );
        results[idx++] = byte_result ? 307 : 0;
        
        // Complex asm with goto for CFG complexity
        if (i % 4 == 0) {
            int temp;
            __asm__ volatile (
                "ucomiss %1, %2\n\t"
                "jp 1f\n\t"
                "ja 2f\n\t"
                "mov $1, %0\n\t"
                "jmp 3f\n\t"
                "1:\n\t"
                "mov $2, %0\n\t"
                "jmp 3f\n\t"
                "2:\n\t"
                "mov $3, %0\n\t"
                "3:\n\t"
                : "=r" (temp)
                : "x" (f1), "x" (f2)
            );
            results[idx++] = temp;
        }
    }
}

int main() {
    // Initialize arrays with special floating-point values
    float farr[ARRAY_SIZE];
    double darr[ARRAY_SIZE];
    int results[ARRAY_SIZE * 50];  // Large enough for all tests
    
    // Fill arrays with normal and special values
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (i * 1.1f) - 15.5f;
        darr[i] = (i * 2.3) - 30.7;
        
        // Insert special values at specific positions
        if (i % 5 == 0) farr[i] = NAN;
        if (i % 7 == 0) farr[i] = INFINITY;
        if (i % 9 == 0) farr[i] = -INFINITY;
        if (i % 6 == 0) darr[i] = NAN;
        if (i % 8 == 0) darr[i] = INFINITY;
        if (i % 10 == 0) darr[i] = -INFINITY;
        if (i % 11 == 0) farr[i] = 0.0f;
        if (i % 13 == 0) darr[i] = -0.0;
    }
    
    // Clear results
    memset(results, 0, sizeof(results));
    
    // Run all tests
    test_scalar_cmps(farr, darr, results);
    test_builtins(farr, darr, results + ARRAY_SIZE * 20);
    test_vector(farr, darr, results + ARRAY_SIZE * 30);
    test_asm(farr, darr, results + ARRAY_SIZE * 40);
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE * 50; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
