#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f};
double darr[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = farr[i];
            float f2 = farr[j];
            double d1 = darr[i];
            double d2 = darr[j];
            
            // Use ternary operators to force CMOV/SET generation
            // This should generate various condition codes
            
            // UNORDERED cases (NaN involved)
            result += (f1 < f2) ? 1 : 0;
            result += (f1 > f2) ? 2 : 0;
            result += (f1 <= f2) ? 4 : 0;
            result += (f1 >= f2) ? 8 : 0;
            result += (f1 == f2) ? 16 : 0;
            result += (f1 != f2) ? 32 : 0;
            
            // Double comparisons
            result += (d1 < d2) ? 64 : 0;
            result += (d1 > d2) ? 128 : 0;
            result += (d1 <= d2) ? 256 : 0;
            result += (d1 >= d2) ? 512 : 0;
            result += (d1 == d2) ? 1024 : 0;
            result += (d1 != d2) ? 2048 : 0;
            
            // Complex nested if-else with goto for control flow diversification
            if (isnan(f1) || isnan(f2)) {
                result += 4096;
                if (isinf(f1)) {
                    goto label1;
                }
            } else if (f1 < f2) {
                result += 8192;
            } else if (f1 > f2) {
                result += 16384;
            } else {
                result += 32768;
            }
            
        label1:
            // Switch statement with floating comparisons in cases
            switch (fpclassify(d1)) {
                case FP_NAN:
                    result += (d1 != d2) ? 65536 : 131072;
                    break;
                case FP_INFINITE:
                    result += (d1 > d2) ? 262144 : 524288;
                    break;
                case FP_ZERO:
                    result += (d1 == d2) ? 1048576 : 2097152;
                    break;
                default:
                    result += (d1 <= d2) ? 4194304 : 8388608;
                    if (d1 >= d2) {
                        result += 16777216;
                    }
                    break;
            }
        }
    }
    
    return result;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = farr[i];
            float f2 = farr[j];
            double d1 = darr[i];
            double d2 = darr[j];
            
            // These builtins directly map to specific condition codes
            
            // __builtin_isunordered - should generate UNORDERED
            result += __builtin_isunordered(f1, f2) ? 1 : 0;
            result += __builtin_isunordered(d1, d2) ? 2 : 0;
            
            // __builtin_isgreater - should generate GT (with unordered handling)
            result += __builtin_isgreater(f1, f2) ? 4 : 0;
            result += __builtin_isgreater(d1, d2) ? 8 : 0;
            
            // __builtin_isgreaterequal - should generate GE
            result += __builtin_isgreaterequal(f1, f2) ? 16 : 0;
            result += __builtin_isgreaterequal(d1, d2) ? 32 : 0;
            
            // __builtin_isless - should generate LT
            result += __builtin_isless(f1, f2) ? 64 : 0;
            result += __builtin_isless(d1, d2) ? 128 : 0;
            
            // __builtin_islessequal - should generate LE
            result += __builtin_islessequal(f1, f2) ? 256 : 0;
            result += __builtin_islessequal(d1, d2) ? 512 : 0;
            
            // __builtin_islessgreater - should generate LTGT
            result += __builtin_islessgreater(f1, f2) ? 1024 : 0;
            result += __builtin_islessgreater(d1, d2) ? 2048 : 0;
            
            // Mixed in conditional expressions
            int temp = 0;
            temp = __builtin_isunordered(f1, f2) ? 
                   (__builtin_isgreater(d1, d2) ? 4096 : 8192) :
                   (__builtin_isless(f1, f2) ? 16384 : 32768);
            result += temp;
        }
    }
    
    return result;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    int result = 0;
    
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    // Vector comparisons generate packed comparison RTL
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce vector to scalar mask
    float* fcmp = (float*)&vcmp_f;
    double* dcmp = (double*)&vcmp_d;
    
    for (int i = 0; i < 4; i++) {
        result += (fcmp[i] != 0.0f) ? (1 << i) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        result += (dcmp[i] != 0.0) ? (1 << (i + 4)) : 0;
    }
    
    // More vector operations
    v4sf vf3 = vf1 <= vf2;
    v2df vd3 = vd1 >= vd2;
    
    fcmp = (float*)&vf3;
    dcmp = (double*)&vd3;
    
    for (int i = 0; i < 4; i++) {
        result += (fcmp[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        result += (dcmp[i] != 0.0) ? (1 << (i + 12)) : 0;
    }
    
    return result;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    int result = 0;
    uint8_t byte_result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double d1 = darr[i];
            double d2 = darr[j];
            
            // Inline assembly that uses condition code names
            // These should trigger the assembly printer to resolve the codes
            
            // Test UNORDERED
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %0"
                : "=r"(byte_result)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += byte_result;
            
            // Test ORDERED
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %0"
                : "=r"(byte_result)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += byte_result * 2;
            
            // Test various condition codes
            int temp;
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "seta %0\n\t"      // Above (greater, unordered)
                : "=r"(temp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += temp * 4;
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %0\n\t"      // Below (less, unordered)
                : "=r"(temp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += temp * 8;
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %0\n\t"      // Equal
                : "=r"(temp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += temp * 16;
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %0\n\t"     // Not equal (including unordered)
                : "=r"(temp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += temp * 32;
            
            // Test with "g" constraint to let compiler choose register
            double cmp_result;
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %0\n\t"     // Below or equal
                : "=g"(byte_result)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            result += byte_result * 64;
        }
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    // Call all test functions and accumulate results
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
