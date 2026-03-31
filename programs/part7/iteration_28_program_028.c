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
    results[idx++] = (f1 < f2) ? 1 : 0;      // LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // NEQ
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;      // LT
    results[idx++] = (d1 > d2) ? 8 : 0;      // GT
    results[idx++] = (d1 <= d2) ? 9 : 0;     // LE
    results[idx++] = (d1 >= d2) ? 10 : 0;    // GE
    results[idx++] = (d1 == d2) ? 11 : 0;    // EQ
    results[idx++] = (d1 != d2) ? 12 : 0;    // NEQ
    
    // Complex expressions to force condition code materialization
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2 && !isnan(f1)) ? 13 : 0;
                break;
            case 1:
                results[idx++] = (d1 > d2 || isnan(d2)) ? 14 : 0;
                break;
            case 2:
                results[idx++] = (f1 == f2) ? 15 : (isinf(f1) ? 16 : 17);
                break;
        }
    }
    
    // Nested conditionals with goto for complex control flow
    if (f1 < f2) {
        if (d1 > d2) {
            results[idx++] = 18;
            goto label1;
        } else {
            results[idx++] = 19;
        }
    } else {
        results[idx++] = 20;
    }
label1:
    
    // Mixed float/double comparisons
    double f1_as_double = f1;
    float d1_as_float = d1;
    results[idx++] = (f1_as_double < d2) ? 21 : 0;
    results[idx++] = (d1_as_float > f2) ? 22 : 0;
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct builtin calls - these should map to specific condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0;
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;
    results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;
    results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
    
    // Combined builtins in complex expressions
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isgreater(d1, d2)) ? 13 : 0;
    results[idx++] = (!__builtin_isunordered(f1, f2) && __builtin_islessequal(d1, d2)) ? 14 : 0;
    
    // Loop with builtins
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            results[idx++] = __builtin_isunordered(f1 + i, f2) ? 15 + i : 0;
        } else {
            results[idx++] = __builtin_isgreater(d1, d2 + i) ? 20 + i : 0;
        }
    }
}

// Test vector/SIMD comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f1 + 1.0f, f1 + 2.0f, f1 + 3.0f};
    v4sf vf2 = {f2, f2 - 1.0f, f2 - 2.0f, f2 - 3.0f};
    v2df vd1 = {d1, d1 + 1.0};
    v2df vd2 = {d2, d2 - 1.0};
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce vector to scalar (forces condition code checks)
    float* fcmp = (float*)&vcmp_f;
    double* dcmp = (double*)&vcmp_d;
    
    results[0] = (fcmp[0] != 0.0f) ? 1 : 0;
    results[1] = (fcmp[1] != 0.0f) ? 2 : 0;
    results[2] = (fcmp[2] != 0.0f) ? 3 : 0;
    results[3] = (fcmp[3] != 0.0f) ? 4 : 0;
    results[4] = (dcmp[0] != 0.0) ? 5 : 0;
    results[5] = (dcmp[1] != 0.0) ? 6 : 0;
    
    // More vector operations
    v4sf vcmp_f2 = vf1 == vf2;
    v2df vcmp_d2 = vd1 != vd2;
    
    float* fcmp2 = (float*)&vcmp_f2;
    double* dcmp2 = (double*)&vcmp_d2;
    
    for (int i = 0; i < 4; i++) {
        results[6 + i] = (fcmp2[i] != 0.0f) ? 7 + i : 0;
    }
    for (int i = 0; i < 2; i++) {
        results[10 + i] = (dcmp2[i] != 0.0) ? 11 + i : 0;
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    unsigned char byte_result;
    int int_result;
    
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
    results[0] = byte_result;
    
    // ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[1] = byte_result;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[2] = byte_result;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  // Note swapped order for "not less than"
        "setnb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[3] = byte_result;
    
    // UNGT (not less or equal)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "setnbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[4] = byte_result;
    
    // UNLE (unordered or less or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[5] = byte_result;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[6] = byte_result;
    
    // LTGT (less or greater)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[7] = byte_result;
    
    // Double precision versions
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[8] = byte_result;
    
    // CMOV with condition code
    int a = 100, b = 200;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "cmovb %1, %0"
        : "+r"(int_result)
        : "r"(a), "x"(f1), "x"(f2)
        : "cc"
    );
    results[9] = int_result;
}

int main() {
    // Initialize test values including special floating-point values
    float f_values[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        INFINITY, -INFINITY, NAN,
        3.14f, -2.71f
    };
    
    double d_values[] = {
        1.0, 2.0, 0.0, -1.0,
        INFINITY, -INFINITY, NAN,
        3.1415926535, -2.7182818284
    };
    
    int results[256];
    memset(results, 0, sizeof(results));
    
    int checksum = 0;
    
    // Test various combinations of values
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            int local_results[64];
            
            // Call all test functions
            test_scalar_cmps(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 25; k++) checksum += local_results[k];
            
            test_builtins(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 20; k++) checksum += local_results[k];
            
            test_vector(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 12; k++) checksum += local_results[k];
            
            test_asm(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 10; k++) checksum += local_results[k];
        }
    }
    
    // Also test classification functions
    for (int i = 0; i < 9; i++) {
        checksum += fpclassify(f_values[i]);
        checksum += fpclassify(d_values[i]);
        checksum += isnan(f_values[i]) ? 1 : 0;
        checksum += isinf(d_values[i]) ? 1 : 0;
        checksum += isfinite(f_values[i]) ? 1 : 0;
        checksum += isnormal(d_values[i]) ? 1 : 0;
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
