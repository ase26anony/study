#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.14f,
    4.0f, 5.0f, NAN, -INFINITY, INFINITY, -2.0f, 0.5f, -0.5f
};

double darr[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.1415926535,
    4.0, 5.0, NAN, -INFINITY, INFINITY, -2.0, 0.5, -0.5
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32] = {0};
    int idx = 0;
    
    // Complex control flow with nested if-else and switch
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i+8];
        double d1 = darr[i];
        double d2 = darr[i+8];
        
        // Use ternary operator to force CMOV/SET generation
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
        
        // Classification functions to generate UNORDERED/ORDERED
        switch (fpclassify(f1)) {
            case FP_NAN:
                results[idx++] = 13;
                break;
            case FP_INFINITE:
                results[idx++] = 14;
                break;
            case FP_ZERO:
                results[idx++] = 15;
                break;
            case FP_SUBNORMAL:
                results[idx++] = 16;
                break;
            default:
                results[idx++] = 17;
                break;
        }
        
        // More complex conditional with isnan/isinf
        if (isnan(f1) || isinf(f1)) {
            results[idx++] = 18;
            goto skip_double_check;
        } else if (isnan(d1) || isinf(d1)) {
            results[idx++] = 19;
        }
    skip_double_check:
        results[idx++] = 20;
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    return sum;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int results[24] = {0};
    int idx = 0;
    
    for (int i = 0; i < 6; i++) {
        float f1 = farr[i];
        float f2 = farr[i+6];
        double d1 = darr[i];
        double d2 = darr[i+6];
        
        // Builtins that directly map to condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // GT (unordered)
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0; // GE (unordered)
        results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         // LT (unordered)
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // LE (unordered)
        results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;  // LTGT
        results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;    // UNORDERED
        
        // Double versions
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
        
        // Combined tests for ORDERED
        results[idx++] = (!__builtin_isunordered(f1, f2)) ? 13 : 0;  // ORDERED
        results[idx++] = (!__builtin_isunordered(d1, d2)) ? 14 : 0;  // ORDERED
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 24; i++) {
        sum += results[i];
    }
    return sum;
}

// Test vector comparisons
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int results[16] = {0};
    int idx = 0;
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf vcmp_lt = vf1 < vf2;    // UNLT/UNLE
    v4sf vcmp_gt = vf1 > vf2;    // UNGT/UNGE
    v4sf vcmp_eq = vf1 == vf2;   // UNEQ
    v4sf vcmp_neq = vf1 != vf2;  // LTGT
    
    v2df vd_cmp_lt = vd1 < vd2;
    v2df vd_cmp_gt = vd1 > vd2;
    v2df vd_cmp_eq = vd1 == vd2;
    v2df vd_cmp_neq = vd1 != vd2;
    
    // Reduce vector results to scalar for checksum
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp_lt[i] ? 1 : 0;
        results[idx++] = vcmp_gt[i] ? 2 : 0;
        results[idx++] = vcmp_eq[i] ? 3 : 0;
        results[idx++] = vcmp_neq[i] ? 4 : 0;
    }
    
    // Double vector reduction
    for (int i = 0; i < 2; i++) {
        results[idx++] = vd_cmp_lt[i] ? 5 : 0;
        results[idx++] = vd_cmp_gt[i] ? 6 : 0;
        results[idx++] = vd_cmp_eq[i] ? 7 : 0;
        results[idx++] = vd_cmp_neq[i] ? 8 : 0;
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += results[i];
    }
    return sum;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    unsigned char results[32] = {0};
    
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i+8];
        double d1 = darr[i];
        double d2 = darr[i+8];
        
        // Inline assembly with various condition codes
        // These force the assembly printer to resolve condition code names
        
        // UNORDERED
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setp %0"
            : "=r"(results[i*4])
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        // ORDERED
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setnp %0"
            : "=r"(results[i*4 + 1])
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        // UNEQ (unordered or equal)
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "sete %0"
            : "=r"(results[i*4 + 2])
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        // UNGE (not less than) - using "ae" condition (above or equal)
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setae %0"
            : "=r"(results[i*4 + 3])
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        // Additional tests with different condition codes
        if (i < 4) {
            // UNGT (not less or equal) - using "a" condition (above)
            unsigned char tmp;
            __asm__ volatile (
                "ucomiss %1, %2\n\t"
                "seta %0"
                : "=r"(tmp)
                : "x"(f1), "x"(f2)
                : "cc"
            );
            results[16 + i] = tmp;
            
            // UNLE (unordered or less or equal) - using "be" condition (below or equal)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setbe %0"
                : "=r"(tmp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            results[20 + i] = tmp;
            
            // UNLT (unordered or less than) - using "b" condition (below)
            __asm__ volatile (
                "ucomiss %1, %2\n\t"
                "setb %0"
                : "=r"(tmp)
                : "x"(f1), "x"(f2)
                : "cc"
            );
            results[24 + i] = tmp;
            
            // LTGT (less or greater) - using "ne" condition (not equal)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setne %0"
                : "=r"(tmp)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            results[28 + i] = tmp;
        }
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    return sum;
}

int main(void) {
    int total_checksum = 0;
    
    // Call all test functions and accumulate results
    total_checksum += test_scalar_cmps();
    total_checksum += test_builtins();
    total_checksum += test_vector();
    total_checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
