#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// Vector types for SIMD comparisons
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent optimization
volatile int global_checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT or LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT or GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE or LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE or GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ or EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT or NE
    
    // Double precision comparisons
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
    
    // Loop with floating comparisons
    for (int i = 0; i < 3; i++) {
        results[idx++] = (f1 < (f2 + i)) ? 21 + i : 0;
        if (d1 > (d2 - i)) {
            results[idx++] = 30 + i;
            continue;
        }
        results[idx++] = 40 + i;
    }
    
    // Switch statement with floating comparisons
    switch (fpclassify(f1)) {
        case FP_NAN:
            results[idx++] = (f1 != f1) ? 50 : 51;  // NaN check
            break;
        case FP_INFINITE:
            results[idx++] = (f1 > 0) ? 52 : 53;
            break;
        case FP_ZERO:
            results[idx++] = (f1 == 0.0f) ? 54 : 55;
            break;
        default:
            results[idx++] = (f1 < 1.0f) ? 56 : 57;
            goto normal_flow;
    }
    
normal_flow:
    // More comparisons after label
    results[idx++] = (d1 != d2) ? 58 : 59;
}

// Test built-in unordered comparison functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct built-in calls that map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 100 : 101;
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 102 : 103;
    results[idx++] = __builtin_isless(f1, f2) ? 104 : 105;
    results[idx++] = __builtin_islessequal(f1, f2) ? 106 : 107;
    results[idx++] = __builtin_islessgreater(f1, f2) ? 108 : 109;
    results[idx++] = __builtin_isunordered(f1, f2) ? 110 : 111;
    
    // Double precision built-ins
    results[idx++] = __builtin_isgreater(d1, d2) ? 112 : 113;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 114 : 115;
    results[idx++] = __builtin_isless(d1, d2) ? 116 : 117;
    results[idx++] = __builtin_islessequal(d1, d2) ? 118 : 119;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 120 : 121;
    results[idx++] = __builtin_isunordered(d1, d2) ? 122 : 123;
    
    // Combined built-in calls in expressions
    results[idx++] = __builtin_isunordered(f1, f2) || __builtin_isgreater(d1, d2) ? 124 : 125;
    results[idx++] = !__builtin_isunordered(f1, f2) && __builtin_islessequal(d1, d2) ? 126 : 127;
    
    // Built-ins in loop conditions
    for (int i = 0; i < 2; i++) {
        if (__builtin_isunordered(f1 + i, f2)) {
            results[idx++] = 130 + i;
            continue;
        }
        if (__builtin_isgreater(d1, d2 + i)) {
            results[idx++] = 140 + i;
            break;
        }
        results[idx++] = 150 + i;
    }
}

// Test vector/SIMD comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f1 + 1.0f, f1 + 2.0f, f1 + 3.0f};
    v4sf vf2 = {f2, f2 - 1.0f, f2 - 2.0f, f2 - 3.0f};
    v2df vd1 = {d1, d1 + 1.0};
    v2df vd2 = {d2, d2 - 1.0};
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf cmp_lt = vf1 < vf2;
    v4sf cmp_gt = vf1 > vf2;
    v4sf cmp_eq = vf1 == vf2;
    v4sf cmp_ne = vf1 != vf2;
    v4sf cmp_le = vf1 <= vf2;
    v4sf cmp_ge = vf1 >= vf2;
    
    v2df dcmp_lt = vd1 < vd2;
    v2df dcmp_gt = vd1 > vd2;
    
    // Reduce vector comparisons to scalar condition codes
    float* fcmp = (float*)&cmp_lt;
    results[0] = (fcmp[0] != 0.0f) ? 200 : 201;
    results[1] = (fcmp[1] != 0.0f) ? 202 : 203;
    results[2] = (fcmp[2] != 0.0f) ? 204 : 205;
    results[3] = (fcmp[3] != 0.0f) ? 206 : 207;
    
    fcmp = (float*)&cmp_gt;
    results[4] = (fcmp[0] != 0.0f) ? 208 : 209;
    
    // Mixed vector and scalar comparisons
    results[5] = ((vf1[0] < vf2[0]) && (vd1[0] > vd2[0])) ? 210 : 211;
    
    // Vector comparison in conditional
    v4sf mask = cmp_eq | cmp_ne;
    results[6] = (mask[0] != 0.0f || mask[1] != 0.0f) ? 212 : 213;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    unsigned char byte_result;
    int int_result;
    
    // Test various condition codes via inline assembly
    // These force the assembly printer to resolve condition code names
    
    // Unordered comparison
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
    );
    results[0] = byte_result ? 300 : 301;
    
    // Ordered comparison
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
    );
    results[1] = byte_result ? 302 : 303;
    
    // Greater than (unordered)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
    );
    results[2] = byte_result ? 304 : 305;
    
    // Less than (unordered)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
    );
    results[3] = byte_result ? 306 : 307;
    
    // Not less than (UNGE)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
    );
    results[4] = byte_result ? 308 : 309;
    
    // Not less or equal (UNGT)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
    );
    results[5] = byte_result ? 310 : 311;
    
    // Unordered or less or equal (UNLE)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
    );
    results[6] = byte_result ? 312 : 313;
    
    // Unordered or less than (UNLT)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnae %0"
        : "=r" (byte_result)
        : "x" (d1), "x" (d2)
    );
    results[7] = byte_result ? 314 : 315;
    
    // Not equal (unordered) - LTGT
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r" (byte_result)
        : "x" (f1), "x" (f2)
    );
    results[8] = byte_result ? 316 : 317;
    
    // Conditional move based on floating comparison
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "cmovb %1, %0"
        : "+r" (int_result)
        : "r" (400), "x" (f1), "x" (f2)
    );
    results[9] = int_result;
}

// Main test driver
int main() {
    // Initialize test values including special floating-point values
    float f_values[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        NAN, INFINITY, -INFINITY,
        3.14f, -2.71f
    };
    
    double d_values[] = {
        1.0, 2.0, 0.0, -1.0,
        NAN, INFINITY, -INFINITY,
        3.1415926535, -2.7182818284
    };
    
    int results[256];
    memset(results, 0, sizeof(results));
    
    // Run tests with various combinations of values
    int result_idx = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j) continue;
            
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            // Test scalar comparisons
            test_scalar_cmps(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
            
            // Test built-ins
            test_builtins(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 20;
            
            // Test vector comparisons
            test_vector(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 10;
            
            // Test inline assembly
            test_asm(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 10;
            
            // Break early to avoid too many iterations
            if (result_idx > 200) break;
        }
        if (result_idx > 200) break;
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
