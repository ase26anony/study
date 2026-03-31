#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f,
    4.2f, 5.7f, NAN, -INFINITY, 7.8f, 8.9f, 9.1f, 10.2f
};

double darr[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5,
    4.2, 5.7, NAN, -INFINITY, 7.8, 8.9, 9.1, 10.2
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32];
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 8];
        double d1 = darr[i];
        double d2 = darr[i + 8];
        
        // Use ternary operators to force CMOV/SET generation
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
        
        // Complex conditional chains
        if (isnan(f1) || isnan(f2)) {
            results[idx++] = 13;
            goto skip_float;
        }
        
        if (f1 < f2) {
            results[idx++] = 14;
        } else if (f1 > f2) {
            results[idx++] = 15;
        } else if (f1 == f2) {
            results[idx++] = 16;
        } else {
            results[idx++] = 17;  // unordered
        }
        
    skip_float:
        // Switch statement with floating comparisons
        switch (fpclassify(d1)) {
            case FP_NAN:
                results[idx++] = (d2 > 0.0) ? 18 : 19;
                break;
            case FP_INFINITE:
                results[idx++] = (d1 < d2) ? 20 : 21;
                break;
            case FP_ZERO:
                results[idx++] = (d1 == d2) ? 22 : 23;
                break;
            default:
                results[idx++] = (d1 != d2) ? 24 : 25;
                break;
        }
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test built-in unordered comparison functions
int test_builtins(void) {
    int results[24];
    int idx = 0;
    
    for (int i = 0; i < 6; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 6];
        double d1 = darr[i];
        double d2 = darr[i + 6];
        
        // Built-in functions that generate specific condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // GT (ordered)
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0; // GE (ordered)
        results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         // LT (ordered)
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // LE (ordered)
        results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;  // LTGT
        results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;    // UNORDERED
        
        // Double versions
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
        
        // Nested conditionals with builtins
        if (__builtin_isunordered(f1, f2)) {
            results[idx++] = 13;
            continue;
        }
        
        if (__builtin_isgreater(f1, f2)) {
            results[idx++] = 14;
        } else if (__builtin_isless(f1, f2)) {
            results[idx++] = 15;
        } else {
            results[idx++] = 16;  // equal (ordered)
        }
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test vector comparisons
int test_vector(void) {
    v4sf vf1, vf2;
    v2df vd1, vd2;
    
    // Initialize vectors
    for (int i = 0; i < 4; i++) {
        vf1[i] = farr[i];
        vf2[i] = farr[i + 4];
    }
    for (int i = 0; i < 2; i++) {
        vd1[i] = darr[i];
        vd2[i] = darr[i + 2];
    }
    
    int results[16];
    int idx = 0;
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf vcmp_f = (vf1 < vf2);
    v2df vcmp_d = (vd1 > vd2);
    
    // Reduce vector to scalar mask
    int mask_f = 0, mask_d = 0;
    for (int i = 0; i < 4; i++) {
        mask_f |= (vcmp_f[i] != 0.0f) ? (1 << i) : 0;
    }
    for (int i = 0; i < 2; i++) {
        mask_d |= (vcmp_d[i] != 0.0) ? (1 << i) : 0;
    }
    
    results[idx++] = mask_f;
    results[idx++] = mask_d;
    
    // More vector operations
    vcmp_f = (vf1 <= vf2);
    vcmp_d = (vd1 >= vd2);
    
    mask_f = mask_d = 0;
    for (int i = 0; i < 4; i++) {
        mask_f |= (vcmp_f[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    for (int i = 0; i < 2; i++) {
        mask_d |= (vcmp_d[i] != 0.0) ? (1 << (i + 2)) : 0;
    }
    
    results[idx++] = mask_f;
    results[idx++] = mask_d;
    
    // Vector equality/inequality
    vcmp_f = (vf1 == vf2);
    vcmp_d = (vd1 != vd2);
    
    mask_f = mask_d = 0;
    for (int i = 0; i < 4; i++) {
        mask_f |= (vcmp_f[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    for (int i = 0; i < 2; i++) {
        mask_d |= (vcmp_d[i] != 0.0) ? (1 << (i + 4)) : 0;
    }
    
    results[idx++] = mask_f;
    results[idx++] = mask_d;
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test inline assembly with condition codes
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = darr[i];
        double d2 = darr[i + 8];
        
        // Compare and set based on condition codes
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "seta %0\n\t"        // above (greater than, ordered)
            : "=r" (results[idx++])
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setb %0\n\t"        // below (less than, ordered)
            : "=r" (results[idx++])
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "sete %0\n\t"        // equal (ordered)
            : "=r" (results[idx++])
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setp %0\n\t"        // parity (unordered)
            : "=r" (results[idx++])
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        // Using condition code names directly
        unsigned char cc_result;
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%3 %0"
            : "=r" (cc_result)
            : "x" (d1), "x" (d2), "i" (""), "i" ("a"), "i" ("b"), 
              "i" ("e"), "i" ("g"), "i" ("l"), "i" ("o"), "i" ("p"),
              "i" ("s")
        );
        results[idx++] = cc_result;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Main function with complex control flow
int main(void) {
    int total = 0;
    
    // Loop with varied control flow
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                total += test_scalar_cmps();
                if (total > 1000) goto early_exit;
                break;
            case 1:
                total += test_builtins();
                // Fall through intentionally
            case 2:
                total += test_vector();
                total += test_asm();
                break;
            default:
                continue;
        }
        
        // Nested loop with floating comparisons
        for (int i = 0; i < 4; i++) {
            double d = darr[i];
            if (isnan(d)) {
                total += 1;
                break;
            } else if (isinf(d)) {
                total += 2;
                continue;
            } else if (d < 0.0) {
                total += 3;
            } else if (d > 0.0) {
                total += 4;
            } else {
                total += 5;  // zero
            }
        }
    }
    
early_exit:
    
    // Final checks with complex conditionals
    float f1 = farr[0];
    float f2 = farr[1];
    double d1 = darr[0];
    double d2 = darr[1];
    
    // Force generation of various condition codes through ternary operators
    int final_check = 
        (f1 < f2) ? 1 : 
        (f1 > f2) ? 2 : 
        (f1 == f2) ? 3 : 
        (isunordered(f1, f2)) ? 4 : 0;
    
    final_check += 
        (d1 < d2) ? 5 : 
        (d1 > d2) ? 6 : 
        (d1 == d2) ? 7 : 
        (isunordered(d1, d2)) ? 8 : 0;
    
    total += final_check;
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
