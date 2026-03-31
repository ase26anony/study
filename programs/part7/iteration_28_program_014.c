#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent optimization
volatile int global_checksum = 0;

// Test scalar comparisons with all condition codes
int test_scalar_cmps(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Test all floating-point comparisons that generate different condition codes
    // Using ternary operators to force CMOV/SET generation
    
    // UNORDERED case (NaN comparisons)
    result += (f1 != f1) ? 1 : 0;  // isnan check
    result += (d1 != d1) ? 2 : 0;
    
    // ORDERED case
    result += (f1 == f1) ? 4 : 0;  // not NaN
    result += (d1 == d1) ? 8 : 0;
    
    // UNEQ (unordered or equal)
    result += !(f1 < f2) && !(f1 > f2) ? 16 : 0;
    result += !(d1 < d2) && !(d1 > d2) ? 32 : 0;
    
    // UNGE (not less than)
    result += !(f1 < f2) ? 64 : 0;
    result += !(d1 < d2) ? 128 : 0;
    
    // UNGT (not less than or equal)
    result += !(f1 <= f2) ? 256 : 0;
    result += !(d1 <= d2) ? 512 : 0;
    
    // UNLE (unordered or less than or equal)
    result += (f1 <= f2) || (f1 != f1) || (f2 != f2) ? 1024 : 0;
    result += (d1 <= d2) || (d1 != d1) || (d2 != d2) ? 2048 : 0;
    
    // UNLT (unordered or less than)
    result += (f1 < f2) || (f1 != f1) || (f2 != f2) ? 4096 : 0;
    result += (d1 < d2) || (d1 != d1) || (d2 != d2) ? 8192 : 0;
    
    // LTGT (less than or greater than)
    result += (f1 < f2) || (f1 > f2) ? 16384 : 0;
    result += (d1 < d2) || (d1 > d2) ? 32768 : 0;
    
    return result;
}

// Test builtin unordered comparisons
int test_builtins(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Complex control flow with switch to force condition code materialization
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                // __builtin_isunordered directly maps to UNORDERED
                result += __builtin_isunordered(f1, f2) ? 1 : 0;
                result += __builtin_isunordered(d1, d2) ? 2 : 0;
                break;
            case 1:
                // __builtin_isgreater generates GT condition
                result += __builtin_isgreater(f1, f2) ? 4 : 0;
                result += __builtin_isgreater(d1, d2) ? 8 : 0;
                
                // __builtin_isless generates LT condition
                result += __builtin_isless(f1, f2) ? 16 : 0;
                result += __builtin_isless(d1, d2) ? 32 : 0;
                break;
            case 2:
                // __builtin_isgreaterequal
                result += __builtin_isgreaterequal(f1, f2) ? 64 : 0;
                result += __builtin_isgreaterequal(d1, d2) ? 128 : 0;
                
                // __builtin_islessequal
                result += __builtin_islessequal(f1, f2) ? 256 : 0;
                result += __builtin_islessequal(d1, d2) ? 512 : 0;
                break;
        }
        
        // Nested if-else with goto to create complex CFG
        if (__builtin_isunordered(f1, f2)) {
            result += 1024;
            if (__builtin_isunordered(d1, d2)) {
                result += 2048;
                goto skip_point;
            }
        } else {
            result += 4096;
        }
        
        skip_point:
        continue;
    }
    
    return result;
}

// Test vector comparisons
int test_vector(v4sf vf1, v4sf vf2, v2df vd1, v2df vd2) {
    int result = 0;
    
    // Vector comparisons generate packed comparison RTL
    v4sf cmp_f = vf1 < vf2;
    v2df cmp_d = vd1 > vd2;
    
    // Reduce vector to scalar mask
    float* fptr = (float*)&cmp_f;
    double* dptr = (double*)&cmp_d;
    
    for (int i = 0; i < 4; i++) {
        result += fptr[i] != 0.0f ? (1 << i) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        result += dptr[i] != 0.0 ? (1 << (i + 4)) : 0;
    }
    
    // More vector comparisons for different condition codes
    cmp_f = vf1 <= vf2;
    cmp_d = vd1 >= vd2;
    
    fptr = (float*)&cmp_f;
    dptr = (double*)&cmp_d;
    
    for (int i = 0; i < 4; i++) {
        result += fptr[i] != 0.0f ? (1 << (i + 8)) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        result += dptr[i] != 0.0 ? (1 << (i + 12)) : 0;
    }
    
    return result;
}

// Test inline assembly with condition codes
int test_asm(float f1, float f2, double d1, double d2) {
    int result = 0;
    unsigned char byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
    
    // Inline assembly that uses condition code names
    // These force the assembly printer to resolve symbolic condition codes
    
    // Test UNORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    result += byte1;
    
    // Test ORDERED
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte2)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    result += byte2 * 2;
    
    // Test various condition codes with different comparisons
    int temp;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "setae %0"
        : "=r"(temp)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    result += temp * 4;
    
    // UNGT (not less than or equal)
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(temp)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    result += temp * 8;
    
    // UNLE
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(temp)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    result += temp * 16;
    
    // UNLT
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(temp)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    result += temp * 32;
    
    // LTGT (unordered or not equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(temp)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    result += temp * 64;
    
    return result;
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
        3.141592653589793, -2.718281828459045
    };
    
    // Initialize vectors
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int checksum = 0;
    
    // Run tests with various value combinations
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (i == j) continue;
            
            // Test scalar comparisons
            checksum ^= test_scalar_cmps(
                f_values[i], f_values[j],
                d_values[i], d_values[j]
            );
            
            // Test builtins
            checksum ^= test_builtins(
                f_values[i], f_values[j],
                d_values[i], d_values[j]
            );
            
            // Test inline assembly
            checksum ^= test_asm(
                f_values[i], f_values[j],
                d_values[i], d_values[j]
            );
        }
    }
    
    // Test vector comparisons
    checksum ^= test_vector(vf1, vf2, vd1, vd2);
    
    // Modify vector values and test again
    vf1[0] = -INFINITY;
    vf2[1] = -INFINITY;
    vd1[0] = INFINITY;
    vd2[1] = -INFINITY;
    
    checksum ^= test_vector(vf1, vf2, vd1, vd2);
    
    // Additional tests with classification functions
    for (int i = 0; i < 9; i++) {
        // Use fpclassify to generate different condition codes
        int f_class = fpclassify(f_values[i]);
        int d_class = fpclassify(d_values[i]);
        
        // Force condition code generation through switch
        switch (f_class) {
            case FP_NAN:
                checksum += 1;
                break;
            case FP_INFINITE:
                checksum += 2;
                break;
            case FP_ZERO:
                checksum += 4;
                break;
            case FP_SUBNORMAL:
                checksum += 8;
                break;
            case FP_NORMAL:
                checksum += 16;
                break;
        }
        
        switch (d_class) {
            case FP_NAN:
                checksum += 32;
                break;
            case FP_INFINITE:
                checksum += 64;
                break;
            case FP_ZERO:
                checksum += 128;
                break;
            case FP_SUBNORMAL:
                checksum += 256;
                break;
            case FP_NORMAL:
                checksum += 512;
                break;
        }
        
        // Test isnan and isinf
        checksum += isnan(f_values[i]) ? 1024 : 0;
        checksum += isinf(d_values[i]) ? 2048 : 0;
    }
    
    // Store to volatile global to prevent dead code elimination
    global_checksum = checksum;
    
    // Print result
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
