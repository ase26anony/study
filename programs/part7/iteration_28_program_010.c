#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
static float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
static double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};

// Test scalar comparisons with all relational operators
static int test_scalar_cmps(void) {
    int result = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Complex control flow with nested if-else chains
            if (i % 3 == 0) {
                // Use ternary operators to force CMOV/SET generation
                int r1 = (f1 < f2) ? (i + j) : (i - j);
                int r2 = (d1 > d2) ? (i * j) : (i / (j ? j : 1));
                int r3 = (f1 <= f2) ? r1 : r2;
                int r4 = (d1 >= d2) ? r3 : (r1 + r2);
                
                // Switch statement to diversify control flow
                switch (i & 3) {
                    case 0:
                        result += (f1 == f2) ? r4 : -r4;
                        break;
                    case 1:
                        result += (f1 != f2) ? r4 : r4 / 2;
                        break;
                    case 2:
                        result += (d1 == d2) ? r4 * 2 : r4;
                        break;
                    case 3:
                        result += (d1 != d2) ? r4 + 1 : r4 - 1;
                        // Use goto to create interesting CFG
                        if (r4 > 100) goto skip_point;
                        break;
                }
            } else if (i % 3 == 1) {
                // More comparisons with different types
                int r5 = (f1 < f2 && !isnan(f1) && !isnan(f2)) ? 1 : 0;
                int r6 = (d1 > d2 || isinf(d1) || isinf(d2)) ? 2 : 0;
                result += r5 + r6;
                
                // Use fpclassify for more condition codes
                int c1 = fpclassify(f1);
                int c2 = fpclassify(d2);
                result += (c1 == FP_NAN || c2 == FP_NAN) ? 3 : 0;
                result += (c1 == FP_INFINITE || c2 == FP_INFINITE) ? 5 : 0;
            } else {
                // Loop with continue/break for CFG complexity
                for (int k = 0; k < 4; k++) {
                    if (k == 2) continue;
                    float f3 = fvals[k];
                    double d3 = dvals[7 - k];
                    
                    int r7 = (f3 <= f1) ? k : -k;
                    int r8 = (d3 >= d2) ? (k * 2) : (k / 2);
                    
                    result += r7 + r8;
                    
                    if (result > 1000) break;
                }
            }
            
            skip_point:
            // Empty label for goto target
            ;
        }
    }
    
    return result;
}

// Test built-in unordered comparison functions
static int test_builtins(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Direct use of builtins in conditional expressions
            int r1 = __builtin_isgreater(f1, f2) ? (i + 1) : (j + 1);
            int r2 = __builtin_isless(d1, d2) ? (i * 2) : (j * 2);
            int r3 = __builtin_isunordered(f1, f2) ? 1 : 0;
            int r4 = __builtin_isgreater(d1, d2) ? 2 : 0;
            int r5 = __builtin_islessequal(f1, f2) ? 3 : 0;
            int r6 = __builtin_isgreaterequal(d1, d2) ? 4 : 0;
            int r7 = __builtin_islessgreater(f1, f2) ? 5 : 0;
            
            // Combine results with ternary operators
            result += r1 + r2;
            result += (r3 || r4) ? (r5 + r6) : (r7 * 2);
            
            // Nested ternary with builtins
            int r8 = __builtin_isunordered(d1, d2) ? 
                    (__builtin_isgreater(f1, f2) ? 10 : 20) :
                    (__builtin_isless(d1, d2) ? 30 : 40);
            result += r8;
        }
    }
    
    return result;
}

// Test vector/SIMD comparisons
static int test_vector(void) {
    int result = 0;
    
    // Initialize vectors
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 2.0};
    
    // Vector comparisons - these generate packed comparison RTL
    v4sf cmp_f = vf1 < vf2;
    v2df cmp_d = vd1 > vd2;
    
    // Reduce vector to scalar mask
    int mask_f = 0;
    int mask_d = 0;
    
    // Extract elements (forcing scalar condition code checks)
    float* pf = (float*)&cmp_f;
    double* pd = (double*)&cmp_d;
    
    for (int i = 0; i < 4; i++) {
        mask_f |= (pf[i] != 0.0f) ? (1 << i) : 0;
        if (i < 2) {
            mask_d |= (pd[i] != 0.0) ? (1 << (i + 4)) : 0;
        }
    }
    
    // Use mask in conditional expressions
    result += (mask_f & 1) ? 1 : 0;
    result += (mask_f & 2) ? 2 : 0;
    result += (mask_f & 4) ? 4 : 0;
    result += (mask_f & 8) ? 8 : 0;
    result += (mask_d & 16) ? 16 : 0;
    result += (mask_d & 32) ? 32 : 0;
    
    // More vector operations with control flow
    for (int i = 0; i < 4; i++) {
        v4sf vf3 = {fvals[i], fvals[i+1], fvals[i+2], fvals[i+3]};
        v4sf vf4 = {fvals[7-i], fvals[6-i], fvals[5-i], fvals[4-i]};
        
        v4sf cmp = vf3 <= vf4;
        float* pcmp = (float*)&cmp;
        
        for (int j = 0; j < 4; j++) {
            if (pcmp[j] != 0.0f) {
                result += i + j;
            } else {
                result -= i + j;
            }
        }
    }
    
    return result;
}

// Test inline assembly with condition code constraints
static int test_asm(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Inline assembly that uses condition code names
            unsigned char byte1, byte2, byte3, byte4;
            
            // Compare and set based on various conditions
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "seta %0"
                : "=r"(byte1)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %0"
                : "=r"(byte2)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %0"
                : "=r"(byte3)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            
            // Test unordered/ordered conditions
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %0"
                : "=r"(byte4)
                : "x"(d1), "x"(d2)
                : "cc"
            );
            
            result += byte1 + byte2 * 2 + byte3 * 3 + byte4 * 4;
            
            // More assembly with different condition codes
            unsigned char byte5, byte6;
            float f1 = fvals[i];
            float f2 = fvals[j];
            
            __asm__ volatile (
                "ucomiss %2, %1\n\t"
                "setg %0"
                : "=r"(byte5)
                : "x"(f1), "x"(f2)
                : "cc"
            );
            
            __asm__ volatile (
                "ucomiss %2, %1\n\t"
                "setl %0"
                : "=r"(byte6)
                : "x"(f1), "x"(f2)
                : "cc"
            );
            
            result += byte5 * 5 + byte6 * 6;
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
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
