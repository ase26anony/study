#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f,
    4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f
};

double darr[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5,
    4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32] = {0};
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 1];
        double d1 = darr[i];
        double d2 = darr[i + 1];
        
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
        
        // Complex nested if-else with goto for control flow
        if (isnan(f1) || isnan(f2)) {
            results[idx++] = 13;
            goto skip_float;
        } else if (isinf(f1) && isinf(f2)) {
            results[idx++] = 14;
        } else {
            results[idx++] = 15;
        }
    skip_float:
        
        // Switch statement with floating comparisons
        switch (fpclassify(d1)) {
            case FP_NAN:
                results[idx++] = (d1 != d1) ? 16 : 0;  // UNORDERED
                break;
            case FP_INFINITE:
                results[idx++] = (d1 > 0) ? 17 : 0;    // ORDERED
                break;
            case FP_ZERO:
                results[idx++] = (d1 == 0.0) ? 18 : 0; // UNEQ
                break;
            case FP_SUBNORMAL:
                results[idx++] = (d1 >= 0.0) ? 19 : 0; // UNGE
                break;
            default:
                results[idx++] = (d1 <= d2) ? 20 : 0;  // UNLE
                break;
        }
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    return sum;
}

// Test built-in unordered comparison functions
int test_builtins(void) {
    int results[24] = {0};
    int idx = 0;
    
    for (int i = 0; i < 6; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 2];
        double d1 = darr[i];
        double d2 = darr[i + 2];
        
        // Built-in functions that generate specific condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // GT (ordered)
        results[idx++] = __builtin_isless(f1, f2) ? 2 : 0;         // LT (ordered)
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0; // GE (ordered)
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // LE (ordered)
        results[idx++] = __builtin_isunordered(f1, f2) ? 5 : 0;    // UNORDERED
        results[idx++] = !__builtin_isunordered(f1, f2) ? 6 : 0;   // ORDERED
        
        // Double versions
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 11 : 0;
        results[idx++] = !__builtin_isunordered(d1, d2) ? 12 : 0;
        
        // Mixed float/double comparisons
        results[idx++] = (f1 < (float)d2) ? 13 : 0;    // LTGT
        results[idx++] = (f1 > (float)d2) ? 14 : 0;    // UNGT
        results[idx++] = (f1 <= (float)d2) ? 15 : 0;   // UNLE
        results[idx++] = (f1 >= (float)d2) ? 16 : 0;   // UNGE
    }
    
    int sum = 0;
    for (int i = 0; i < 24; i++) {
        sum += results[i];
    }
    return sum;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int results[16] = {0};
    int idx = 0;
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf vcmp_lt = vf1 < vf2;    // LT
    v4sf vcmp_gt = vf1 > vf2;    // GT
    v4sf vcmp_le = vf1 <= vf2;   // LE
    v4sf vcmp_ge = vf1 >= vf2;   // GE
    v4sf vcmp_eq = vf1 == vf2;   // EQ
    v4sf vcmp_neq = vf1 != vf2;  // NEQ
    
    // Reduce vector to scalar mask
    int mask_lt = 0, mask_gt = 0, mask_le = 0, mask_ge = 0, mask_eq = 0, mask_neq = 0;
    for (int i = 0; i < 4; i++) {
        mask_lt |= ((int)vcmp_lt[i] != 0) << i;
        mask_gt |= ((int)vcmp_gt[i] != 0) << i;
        mask_le |= ((int)vcmp_le[i] != 0) << i;
        mask_ge |= ((int)vcmp_ge[i] != 0) << i;
        mask_eq |= ((int)vcmp_eq[i] != 0) << i;
        mask_neq |= ((int)vcmp_neq[i] != 0) << i;
    }
    
    results[idx++] = mask_lt;
    results[idx++] = mask_gt;
    results[idx++] = mask_le;
    results[idx++] = mask_ge;
    results[idx++] = mask_eq;
    results[idx++] = mask_neq;
    
    // Double vector comparisons
    v2df vdcmp_lt = vd1 < vd2;
    v2df vdcmp_gt = vd1 > vd2;
    v2df vdcmp_le = vd1 <= vd2;
    v2df vdcmp_ge = vd1 >= vd2;
    v2df vdcmp_eq = vd1 == vd2;
    v2df vdcmp_neq = vd1 != vd2;
    
    int dmask_lt = 0, dmask_gt = 0, dmask_le = 0, dmask_ge = 0, dmask_eq = 0, dmask_neq = 0;
    for (int i = 0; i < 2; i++) {
        dmask_lt |= ((int)vdcmp_lt[i] != 0) << i;
        dmask_gt |= ((int)vdcmp_gt[i] != 0) << i;
        dmask_le |= ((int)vdcmp_le[i] != 0) << i;
        dmask_ge |= ((int)vdcmp_ge[i] != 0) << i;
        dmask_eq |= ((int)vdcmp_eq[i] != 0) << i;
        dmask_neq |= ((int)vdcmp_neq[i] != 0) << i;
    }
    
    results[idx++] = dmask_lt;
    results[idx++] = dmask_gt;
    results[idx++] = dmask_le;
    results[idx++] = dmask_ge;
    results[idx++] = dmask_eq;
    results[idx++] = dmask_neq;
    
    // Use vector comparison results in conditional expressions
    results[idx++] = (mask_lt != 0) ? 100 : 200;
    results[idx++] = (dmask_gt != 0) ? 300 : 400;
    
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
        double d1 = darr[i];
        double d2 = darr[i + 1];
        float f1 = farr[i];
        float f2 = farr[i + 1];
        
        // Inline assembly that uses condition code names
        // These force the assembly printer to resolve symbolic condition codes
        
        // Test UNORDERED (unord)
        __asm__ volatile (
            "fucomi %%st(1), %%st\n\t"
            "setp %0"
            : "=g" (results[i*4])
            : "t" (d1), "u" (d2)
            : "cc"
        );
        
        // Test ORDERED (ord)
        __asm__ volatile (
            "fucomi %%st(1), %%st\n\t"
            "setnp %0"
            : "=g" (results[i*4 + 1])
            : "t" (f1), "u" (f2)
            : "cc"
        );
        
        // Test UNEQ (ueq) - unordered or equal
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "sete %0"
            : "=g" (results[i*4 + 2])
            : "t" (d1), "u" (d2)
            : "cc"
        );
        
        // Test UNGE (nlt) - not less than (greater or equal or unordered)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnb %0"
            : "=g" (results[i*4 + 3])
            : "t" (f1), "u" (f2)
            : "cc"
        );
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Call all test functions
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
