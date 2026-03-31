#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int *results) {
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
    
    // Mixed comparisons in complex control flow
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 != f2 && !isnan(f1)) ? 11 : 0;
                break;
            case 1:
                results[idx++] = (d1 == d2 || isnan(d1)) ? 12 : 0;
                break;
            case 2:
                results[idx++] = (f1 < f2) ? 13 : (f1 > f2) ? 14 : 15;
                break;
        }
    }
    
    // Unordered comparisons using fpclassify
    int c1 = fpclassify(f1);
    int c2 = fpclassify(f2);
    results[idx++] = (c1 == FP_NAN || c2 == FP_NAN) ? 16 : 0;
    results[idx++] = (c1 == FP_INFINITE) ? 17 : 0;
    
    // Complex nested if-else with goto
    if (f1 < f2) {
        goto label1;
    } else if (f1 > f2) {
        results[idx++] = 18;
    } else {
        label1:
        results[idx++] = (d1 <= d2) ? 19 : 20;
    }
}

// Test built-in unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int *results) {
    int idx = 0;
    
    // Direct built-in calls that map to specific condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 21 : 0;      // GT (unordered)
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 22 : 0; // GE (unordered)
    results[idx++] = __builtin_isless(f1, f2) ? 23 : 0;         // LT (unordered)
    results[idx++] = __builtin_islessequal(f1, f2) ? 24 : 0;    // LE (unordered)
    results[idx++] = __builtin_islessgreater(f1, f2) ? 25 : 0;  // LTGT
    results[idx++] = __builtin_isunordered(f1, f2) ? 26 : 0;    // UNORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 27 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 28 : 0;
    
    // Built-ins in conditional expressions
    results[idx++] = __builtin_isunordered(f1, f2) ? 
                    (__builtin_isgreater(d1, d2) ? 29 : 30) : 31;
    
    // Loop with built-in comparisons
    for (int i = 0; i < 2; i++) {
        if (__builtin_isless(f1, f2)) {
            results[idx++] = 32 + i;
            continue;
        }
        results[idx++] = __builtin_isunordered(d1, d2) ? 34 : 35;
        if (i == 0) break;
    }
}

// Test vector/SIMD comparisons
void test_vector(float f1, float f2, double d1, double d2, int *results) {
    v4sf vf1 = {f1, f2, f1 * 2.0f, f2 * 2.0f};
    v4sf vf2 = {f2, f1, f2 * 2.0f, f1 * 2.0f};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    // Vector comparisons - these generate packed comparisons
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 == vf2;
    v4sf cmp4 = vf1 != vf2;
    
    v2df cmp5 = vd1 < vd2;
    v2df cmp6 = vd1 > vd2;
    
    // Reduce vector comparisons to scalar masks
    float *fc1 = (float*)&cmp1;
    float *fc2 = (float*)&cmp2;
    double *dc1 = (double*)&cmp5;
    
    int mask1 = 0, mask2 = 0, mask3 = 0;
    for (int i = 0; i < 4; i++) {
        mask1 |= (fc1[i] != 0.0f) ? (1 << i) : 0;
        mask2 |= (fc2[i] != 0.0f) ? (1 << i) : 0;
    }
    for (int i = 0; i < 2; i++) {
        mask3 |= (dc1[i] != 0.0) ? (1 << (i + 4)) : 0;
    }
    
    results[0] = mask1;
    results[1] = mask2;
    results[2] = mask3;
    
    // Use vector comparison results in conditional moves
    results[3] = (mask1 > 0) ? 36 : 37;
    results[4] = (mask2 == 0) ? 38 : 39;
    results[5] = (mask3 & 1) ? 40 : 41;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int *results) {
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    // Inline assembly that uses condition code names
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r" (r1)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r" (r2)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    
    // Test various condition codes
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r" (r3)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=r" (r4)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    
    // Unordered/ordered tests
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r" (r5)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r" (r6)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    
    // More complex conditions
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r" (r7)
        : "x" (f1), "x" (f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setae %0"
        : "=r" (r8)
        : "x" (d1), "x" (d2)
        : "cc"
    );
    
    results[0] = r1;
    results[1] = r2;
    results[2] = r3;
    results[3] = r4;
    results[4] = r5;
    results[5] = r6;
    results[6] = r7;
    results[7] = r8;
}

int main() {
    // Initialize test values including special floating-point values
    float fvals[] = {
        1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f,
        3.14f, -2.71f, 1.0e10f, 1.0e-10f
    };
    
    double dvals[] = {
        1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0,
        3.141592653589793, -2.718281828459045,
        1.0e100, 1.0e-100
    };
    
    int results[100];
    memset(results, 0, sizeof(results));
    
    // Run tests with various combinations of values
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            int local_results[50];
            
            test_scalar_cmps(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 20; k++) checksum += local_results[k];
            
            test_builtins(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 15; k++) checksum += local_results[k];
            
            test_vector(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 6; k++) checksum += local_results[k];
            
            test_asm(f1, f2, d1, d2, local_results);
            for (int k = 0; k < 8; k++) checksum += local_results[k];
        }
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
