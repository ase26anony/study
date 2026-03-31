#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2) {
    int results[16] = {0};
    int idx = 0;
    
    // Generate various condition codes through ternary operators
    results[idx++] = (f1 < f2) ? 1 : 0;      // LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // NEQ
    
    // Double comparisons - different type to ensure coverage
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex expressions to force CMOV generation
    float f3 = (f1 < f2) ? f2 : f1;
    double d3 = (d1 > d2) ? d1 : d2;
    results[idx++] = (int)(f3 * 100);
    results[idx++] = (int)(d3 * 100);
    
    // Nested conditionals with floating comparisons
    for (int i = 0; i < 4; i++) {
        if (f1 < f2) {
            if (d1 > d2) {
                results[0] += i;
            } else if (d1 == d2) {
                results[1] += i;
                goto label1;
            }
        } else if (f1 == f2) {
            results[2] += i;
            continue;
        }
        label1:
        results[3] += i;
    }
    
    // Aggregate results
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test built-in unordered comparison functions
void test_builtins(float f1, float f2, double d1, double d2) {
    int results[12] = {0};
    int idx = 0;
    
    // Direct built-in calls that map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;
    results[idx++] = __builtin_isless(f1, f2) ? 2 : 0;
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0;
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;
    results[idx++] = __builtin_isunordered(f1, f2) ? 5 : 0;
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 6 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 9 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 10 : 0;
    
    // Mixed comparisons in complex expressions
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isgreater(d1, d2)) ? 11 : 0;
    results[idx++] = (__builtin_isless(f1, f2) && __builtin_isunordered(d1, d2)) ? 12 : 0;
    
    // Switch statement with floating comparisons
    int sw = 0;
    switch (__builtin_isunordered(f1, f2) + __builtin_isgreater(d1, d2) * 2) {
        case 0:
            sw = 1;
            break;
        case 1:
            sw = 2;
            break;
        case 2:
            sw = 3;
            break;
        case 3:
            sw = 4;
            break;
        default:
            sw = 5;
    }
    checksum += sw;
    
    // Aggregate results
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

// Test vector/SIMD comparisons
void test_vector(float f1, float f2, double d1, double d2) {
    v4sf vf1 = {f1, f2, f1 * 2, f2 * 2};
    v4sf vf2 = {f2, f1, f2 * 2, f1 * 2};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf cmp_f = (vf1 < vf2) ? vf1 : vf2;
    v4sf cmp_f2 = (vf1 > vf2) ? vf1 : vf2;
    v2df cmp_d = (vd1 <= vd2) ? vd1 : vd2;
    v2df cmp_d2 = (vd1 >= vd2) ? vd1 : vd2;
    
    // Reduce to scalar through conditional moves
    float f_result = (cmp_f[0] < cmp_f[1]) ? cmp_f[0] : cmp_f[1];
    double d_result = (cmp_d[0] > cmp_d[1]) ? cmp_d[0] : cmp_d[1];
    
    // Use isnan/isinf on vector elements
    int nan_inf_results[8] = {0};
    for (int i = 0; i < 4; i++) {
        nan_inf_results[i] = isnan(vf1[i]) ? 1 : (isinf(vf1[i]) ? 2 : 0);
    }
    for (int i = 0; i < 2; i++) {
        nan_inf_results[4 + i] = isnan(vd1[i]) ? 3 : (isinf(vd1[i]) ? 4 : 0);
    }
    
    // Complex loop with vector element comparisons
    for (int i = 0; i < 4; i++) {
        if (vf1[i] < vf2[i]) {
            if (i % 2 == 0) {
                checksum += 1;
                continue;
            } else {
                checksum += 2;
                break;
            }
        } else if (vf1[i] == vf2[i]) {
            checksum += 3;
            goto vector_label;
        }
        vector_label:
        checksum += 4;
    }
    
    checksum += (int)(f_result * 100) + (int)(d_result * 100);
    for (int i = 0; i < 8; i++) {
        checksum += nan_inf_results[i];
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2) {
    uint8_t results[8] = {0};
    
    // Inline assembly that uses condition code names
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0\n\t"
        : "=r"(results[0])
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0\n\t"
        : "=r"(results[1])
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    // Test various condition codes
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0\n\t"
        : "=r"(results[2])
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0\n\t"  // unordered
        : "=r"(results[3])
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0\n\t"
        : "=r"(results[4])
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setae %0\n\t"
        : "=r"(results[5])
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    // Complex asm with multiple condition codes
    uint8_t byte1, byte2;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "seta %0\n\t"
        "ucomisd %4, %5\n\t"
        "setb %1\n\t"
        : "=r"(byte1), "=r"(byte2)
        : "x"(f1), "x"(f2), "x"(d1), "x"(d2)
        : "cc"
    );
    results[6] = byte1;
    results[7] = byte2;
    
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
    }
}

// Test fpclassify and special value handling
void test_classification(float f1, float f2, double d1, double d2) {
    int results[16] = {0};
    int idx = 0;
    
    // Use fpclassify to generate various condition codes
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 1 : 0;
    results[idx++] = (fpclassify(f1) == FP_INFINITE) ? 2 : 0;
    results[idx++] = (fpclassify(f1) == FP_ZERO) ? 3 : 0;
    results[idx++] = (fpclassify(f1) == FP_SUBNORMAL) ? 4 : 0;
    results[idx++] = (fpclassify(f1) == FP_NORMAL) ? 5 : 0;
    
    results[idx++] = (fpclassify(d1) == FP_NAN) ? 6 : 0;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 7 : 0;
    results[idx++] = (fpclassify(d1) == FP_ZERO) ? 8 : 0;
    results[idx++] = (fpclassify(d1) == FP_SUBNORMAL) ? 9 : 0;
    results[idx++] = (fpclassify(d1) == FP_NORMAL) ? 10 : 0;
    
    // Mixed comparisons with classification
    results[idx++] = (isnan(f1) && isinf(d1)) ? 11 : 0;
    results[idx++] = (isnormal(f2) || isnan(d2)) ? 12 : 0;
    
    // Complex conditional with fpclassify
    int class_f1 = fpclassify(f1);
    int class_d1 = fpclassify(d1);
    
    if (class_f1 == FP_NAN) {
        if (class_d1 == FP_INFINITE) {
            results[idx++] = 13;
        } else if (class_d1 == FP_ZERO) {
            results[idx++] = 14;
            goto class_label;
        }
    } else if (class_f1 == FP_INFINITE) {
        results[idx++] = 15;
        continue_label:
        results[idx++] = 16;
    }
    class_label:
    results[idx++] = 17;
    
    // Loop with classification
    for (int i = 0; i < 3; i++) {
        if (fpclassify(f1 + i) == FP_NAN) {
            break;
        } else if (fpclassify(d1 + i) == FP_INFINITE) {
            continue;
        }
        results[idx++] = 18 + i;
    }
    
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
}

int main() {
    // Initialize test values including special values
    float f_values[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f};
    double d_values[] = {3.0, 4.0, NAN, INFINITY, -INFINITY, 0.0};
    
    // Run tests with various combinations
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            test_scalar_cmps(f1, f2, d1, d2);
            test_builtins(f1, f2, d1, d2);
            test_vector(f1, f2, d1, d2);
            test_asm(f1, f2, d1, d2);
            test_classification(f1, f2, d1, d2);
        }
    }
    
    // Additional edge cases
    test_scalar_cmps(0.0f, -0.0f, 0.0, -0.0);
    test_builtins(1.0f/0.0f, -1.0f/0.0f, 1.0/0.0, -1.0/0.0);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
