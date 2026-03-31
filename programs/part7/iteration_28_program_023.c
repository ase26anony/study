#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT
    results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE
    results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex control flow with condition codes
    for (int i = 0; i < 10; i++) {
        if (f1 < f2) {
            results[idx] = i;
            idx++;
            if (d1 > d2) {
                continue;
            }
        } else if (f1 == f2) {
            results[idx] = i * 2;
            idx++;
            goto skip_point;
        } else {
            results[idx] = i * 3;
            idx++;
        }
        
        if (d1 <= d2) {
            break;
        }
        
    skip_point:
        if (i % 2 == 0) {
            results[idx++] = (f1 >= f2) ? 100 : 101;
        }
    }
    
    // Switch statement with floating comparisons
    int val = (int)f1;
    switch (val & 3) {
        case 0:
            results[idx++] = (f1 < f2) ? 200 : 201;
            break;
        case 1:
            results[idx++] = (f1 > f2) ? 202 : 203;
            break;
        case 2:
            results[idx++] = (f1 == f2) ? 204 : 205;
            break;
        default:
            results[idx++] = (f1 != f2) ? 206 : 207;
            break;
    }
}

// Test built-in unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // These builtins directly map to condition codes
    results[idx++] = __builtin_isunordered(f1, f2) ? 1 : 0;    // UNORDERED
    results[idx++] = __builtin_isgreater(f1, f2) ? 2 : 0;      // UNLE (inverted)
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         // UNGE (inverted)
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 4 : 0; // UNLT (inverted)
    results[idx++] = __builtin_islessequal(f1, f2) ? 5 : 0;    // UNGT (inverted)
    
    // Double versions
    results[idx++] = __builtin_isunordered(d1, d2) ? 6 : 0;
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 8 : 0;
    
    // Mixed float/double comparisons
    results[idx++] = __builtin_isgreater(f1, (float)d1) ? 9 : 0;
    results[idx++] = __builtin_isless((double)f2, d2) ? 10 : 0;
    
    // Classification functions that may generate condition codes
    results[idx++] = isnan(f1) ? 11 : 0;
    results[idx++] = isinf(d1) ? 12 : 0;
    results[idx++] = fpclassify(f2) == FP_NAN ? 13 : 0;
    results[idx++] = fpclassify(d2) == FP_INFINITE ? 14 : 0;
    
    // Complex expression with builtins
    for (int i = 0; i < 5; i++) {
        if (__builtin_isunordered(f1 + i, f2)) {
            results[idx++] = i * 20;
            continue;
        }
        
        if (__builtin_isgreater(d1, d2 + i)) {
            results[idx++] = i * 30;
            if (__builtin_isless(f1, f2)) {
                goto builtin_skip;
            }
        }
        
        results[idx++] = i * 40;
        
    builtin_skip:
        results[idx++] = __builtin_islessequal(f1, f2) ? 50 : 51;
    }
}

// Test vector comparisons
void test_vector(float* farr, double* darr, int* results) {
    int idx = 0;
    
    // Load vectors
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    // Vector comparisons - these generate packed comparisons
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce to scalar (forces condition code generation)
    float* fcmp = (float*)&vcmp_f;
    double* dcmp = (double*)&vcmp_d;
    
    // Check each element - this creates scalar condition code checks
    for (int i = 0; i < 4; i++) {
        results[idx++] = fcmp[i] != 0.0f ? 1 : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dcmp[i] != 0.0 ? 2 : 0;
    }
    
    // More vector operations
    v4sf vcmp_f2 = vf1 == vf2;
    v2df vcmp_d2 = vd1 != vd2;
    
    float* fcmp2 = (float*)&vcmp_f2;
    double* dcmp2 = (double*)&vcmp_d2;
    
    // Nested loops with vector results
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            if (fcmp2[i] != 0.0f && dcmp2[j] != 0.0) {
                results[idx++] = i * 10 + j;
                break;
            } else {
                results[idx++] = -1;
                continue;
            }
        }
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Inline assembly that uses condition code names
    uint8_t byte1, byte2, byte3, byte4;
    
    // Compare floats and set byte based on condition
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
    
    // Test various condition code outputs
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"
        : "=r" (byte3)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte3;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r" (byte4)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = byte4;
    
    // More assembly with different constraints
    int val1, val2;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "movl $0, %0\n\t"
        "movl $1, %1\n\t"
        "cmovnbe %1, %0"
        : "=r" (val1), "=r" (val2)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    results[idx++] = val1;
    
    // Conditional move with double
    __asm__ volatile (
        "ucomisd %2, %3\n\t"
        "movl $0, %0\n\t"
        "movl $1, %1\n\t"
        "cmovnp %1, %0"
        : "=r" (val1), "=r" (val2)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    results[idx++] = val1;
}

int main() {
    // Initialize test data with special values
    float fvals[] = {
        1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.14f
    };
    
    double dvals[] = {
        1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.1415926535
    };
    
    // Results arrays
    int results1[100] = {0};
    int results2[100] = {0};
    int results3[100] = {0};
    int results4[100] = {0};
    
    // Run all tests with different value combinations
    for (int i = 0; i < 8; i += 2) {
        test_scalar_cmps(fvals[i], fvals[i+1], dvals[i], dvals[i+1], results1);
        test_builtins(fvals[i], fvals[i+1], dvals[i], dvals[i+1], results2);
        test_vector(fvals, dvals, results3);
        test_asm(fvals[i], fvals[i+1], dvals[i], dvals[i+1], results4);
        
        // Update checksum to prevent optimization
        for (int j = 0; j < 20; j++) {
            checksum += results1[j] + results2[j] + results3[j] + results4[j];
        }
    }
    
    // Print checksum (prevents dead code elimination)
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
