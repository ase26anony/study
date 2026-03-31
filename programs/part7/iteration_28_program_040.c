#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent optimization
volatile int global_checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // NEQ
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex expressions that might generate UNORDERED/ORDERED
    results[idx++] = (!(f1 < f2) && !isnan(f1) && !isnan(f2)) ? 13 : 0;
    results[idx++] = (isnan(f1) || isnan(f2)) ? 14 : 0;
    
    // Mixed float/double comparisons
    results[idx++] = ((double)f1 < d2) ? 15 : 0;
    results[idx++] = (f1 < (float)d2) ? 16 : 0;
    
    // Classification-based conditions
    results[idx++] = (fpclassify(f1) == FP_NAN) ? 17 : 0;
    results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 18 : 0;
    results[idx++] = (fpclassify(f2) == FP_ZERO) ? 19 : 0;
    
    // Nested ternary with complex conditions
    results[idx++] = (f1 < f2) ? 20 : ((f1 > f2) ? 21 : ((f1 == f2) ? 22 : 23));
    
    // Loop with varying comparisons
    for (int i = 0; i < 5; i++) {
        float temp = f1 + i;
        results[idx++] = (temp < f2) ? (24 + i) : 0;
        if (temp > f2) break;
        if (isnan(temp)) continue;
    }
}

// Test builtin functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // These builtins directly map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 30 : 0;
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 31 : 0;
    results[idx++] = __builtin_isless(f1, f2) ? 32 : 0;
    results[idx++] = __builtin_islessequal(f1, f2) ? 33 : 0;
    results[idx++] = __builtin_islessgreater(f1, f2) ? 34 : 0;  // LTGT
    
    results[idx++] = __builtin_isunordered(f1, f2) ? 35 : 0;    // UNORDERED
    results[idx++] = !__builtin_isunordered(f1, f2) ? 36 : 0;   // ORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 37 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 38 : 0;
    
    // Combined conditions
    results[idx++] = (__builtin_isgreater(f1, f2) && __builtin_isless(d1, d2)) ? 39 : 0;
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isunordered(d1, d2)) ? 40 : 0;
    
    // Switch statement with builtin conditions
    switch (__builtin_isunordered(f1, f2) ? 1 : 
            __builtin_isgreater(f1, f2) ? 2 : 
            __builtin_isless(f1, f2) ? 3 : 0) {
        case 1: results[idx++] = 41; break;
        case 2: results[idx++] = 42; break;
        case 3: results[idx++] = 43; break;
        default: results[idx++] = 44; break;
    }
    
    // Goto with condition
    if (__builtin_isunordered(f1, f2)) {
        results[idx++] = 45;
        goto unordered_label;
    }
    results[idx++] = 46;
    
unordered_label:
    // Complex control flow
    for (int i = 0; i < 3; i++) {
        if (__builtin_isgreater(f1 + i, f2)) {
            results[idx++] = 47 + i;
            continue;
        }
        if (__builtin_isunordered(f1 + i, f2)) {
            results[idx++] = 50 + i;
            break;
        }
    }
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vec1 = {f1, f2, f1 + 1.0f, f2 - 1.0f};
    v4sf vec2 = {f2, f1, f2 + 1.0f, f1 - 1.0f};
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d1};
    
    int idx = 0;
    
    // Vector comparisons - these generate packed comparisons
    v4sf cmp_result = vec1 < vec2;
    v4sf cmp_result2 = vec1 > vec2;
    v4sf cmp_result3 = vec1 == vec2;
    
    // Reduce to scalar
    float* fr = (float*)&cmp_result;
    results[idx++] = (fr[0] != 0.0f) ? 60 : 0;
    results[idx++] = (fr[1] != 0.0f) ? 61 : 0;
    results[idx++] = (fr[2] != 0.0f) ? 62 : 0;
    results[idx++] = (fr[3] != 0.0f) ? 63 : 0;
    
    // Double vector comparisons
    v2df dcmp_result = dvec1 < dvec2;
    double* dr = (double*)&dcmp_result;
    results[idx++] = (dr[0] != 0.0) ? 64 : 0;
    results[idx++] = (dr[1] != 0.0) ? 65 : 0;
    
    // Mixed comparisons in loop
    for (int i = 0; i < 4; i++) {
        v4sf temp = vec1 + (float)i;
        v4sf cmp = temp < vec2;
        float* fcmp = (float*)&cmp;
        results[idx++] = (fcmp[i] != 0.0f) ? (66 + i) : 0;
    }
    
    // Vector builtin-like test
    int any_nan = 0;
    for (int i = 0; i < 4; i++) {
        if (isnan(fr[i])) {
            any_nan = 1;
            break;
        }
    }
    results[idx++] = any_nan ? 70 : 0;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    int byte_result;
    
    // Test various condition codes via inline assembly
    // These should force the assembly printer to resolve condition code names
    
    // UNORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 80 : 0;
    
    // ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 81 : 0;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 82 : 0;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 83 : 0;
    
    // UNGT (not less than or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 84 : 0;
    
    // UNLE (unordered or less than or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setna %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 85 : 0;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 86 : 0;
    
    // LTGT (less than or greater than - unordered equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 87 : 0;
    
    // Double precision versions
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 88 : 0;
    
    // Test with "g" constraint (let compiler choose register)
    int reg_var;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "set%1 %0"
        : "=r"(byte_result), "=r"(reg_var)
        : "x"(f1), "x"(f2), "1"('p')  // condition code 'p' for parity
        : "cc"
    );
    results[idx++] = byte_result ? 89 : 0;
}

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
    
    int results[200] = {0};
    int result_idx = 0;
    
    // Test various combinations of values
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            // Call test functions
            test_scalar_cmps(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
            
            test_builtins(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 20;
            
            test_vector(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 15;
            
            test_asm(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 10;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
