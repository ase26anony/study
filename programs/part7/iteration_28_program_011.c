#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f,
    4.2f, 5.7f, NAN, 100.0f, -200.0f, 0.001f, -0.001f, 99.9f
};

double darr[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5,
    4.2, 5.7, NAN, 100.0, -200.0, 0.001, -0.001, 99.9
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 8];
        double d1 = darr[i];
        double d2 = darr[i + 8];
        
        // Use ternary operator to force CMOV/SET generation
        results[idx++] = (f1 < f2) ? 1 : 0;      // LT
        results[idx++] = (f1 > f2) ? 2 : 0;      // GT
        results[idx++] = (f1 <= f2) ? 3 : 0;     // LE
        results[idx++] = (f1 >= f2) ? 4 : 0;     // GE
        results[idx++] = (f1 == f2) ? 5 : 0;     // EQ
        results[idx++] = (f1 != f2) ? 6 : 0;     // NEQ
        
        results[idx++] = (d1 < d2) ? 7 : 0;
        results[idx++] = (d1 > d2) ? 8 : 0;
        results[idx++] = (d1 <= d2) ? 9 : 0;
        results[idx++] = (d1 >= d2) ? 10 : 0;
        results[idx++] = (d1 == d2) ? 11 : 0;
        results[idx++] = (d1 != d2) ? 12 : 0;
        
        // Classification functions
        results[idx++] = isnan(f1) ? 13 : 0;
        results[idx++] = isinf(f2) ? 14 : 0;
        results[idx++] = fpclassify(d1) == FP_NAN ? 15 : 0;
        results[idx++] = fpclassify(d2) == FP_INFINITE ? 16 : 0;
    }
    
    // Complex control flow with nested if-else
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        if (results[i] > 0) {
            if (results[i] < 5) {
                sum += results[i];
            } else if (results[i] < 10) {
                sum += results[i] * 2;
            } else {
                switch (results[i] % 4) {
                    case 0: sum += 1; break;
                    case 1: sum += 2; break;
                    case 2: sum += 3; break;
                    case 3: sum += 4; break;
                    default: goto skip;
                }
            }
        }
        skip:
        continue;
    }
    
    return sum;
}

// Test built-in unordered comparisons
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[15 - i];
        double d1 = darr[i];
        double d2 = darr[15 - i];
        
        // Built-in functions that generate specific condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // GT (unordered)
        results[idx++] = __builtin_isless(f1, f2) ? 2 : 0;         // LT (unordered)
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0; // GE (unordered)
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // LE (unordered)
        results[idx++] = __builtin_isunordered(f1, f2) ? 5 : 0;    // UNORDERED
        
        results[idx++] = __builtin_isgreater(d1, d2) ? 6 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 10 : 0;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        // Force conditional move generation
        sum = (results[i] > 5) ? (sum + results[i]) : (sum - results[i]);
    }
    
    return sum;
}

// Test vector comparisons
int test_vector(void) {
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    // Vector comparisons generate packed comparison RTL
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce to scalar mask
    int mask_f = 0;
    int mask_d = 0;
    
    float *fptr = (float*)&vcmp_f;
    double *dptr = (double*)&vcmp_d;
    
    for (int i = 0; i < 4; i++) {
        mask_f |= (fptr[i] != 0.0f) ? (1 << i) : 0;
    }
    for (int i = 0; i < 2; i++) {
        mask_d |= (dptr[i] != 0.0) ? (1 << i) : 0;
    }
    
    // Complex control flow with loops and breaks
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        if (mask_f & (1 << (i % 4))) {
            if (i % 3 == 0) {
                sum += i;
                continue;
            } else if (i % 3 == 1) {
                sum += i * 2;
                break;
            }
        }
        
        if (mask_d & (1 << (i % 2))) {
            switch (i % 4) {
                case 0: sum += 100; break;
                case 1: sum += 200; break;
                case 2: sum += 300; break;
                case 3: sum += 400; break;
            }
        }
    }
    
    return sum;
}

// Test inline assembly with condition codes
int test_asm(void) {
    uint8_t results[16] = {0};
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i + 1];
        double d1 = darr[i];
        double d2 = darr[i + 1];
        
        // Inline assembly that uses condition code names
        uint8_t res1, res2, res3, res4;
        
        // Compare floats and set based on condition
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "seta %0"
            : "=r"(res1)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0"
            : "=r"(res2)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        // Test unordered/ordered conditions
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setp %0"
            : "=r"(res3)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0"
            : "=r"(res4)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        results[idx++] = res1;
        results[idx++] = res2;
        results[idx++] = res3;
        results[idx++] = res4;
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += results[i];
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Call all test functions and aggregate results
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Additional complex comparisons in main
    volatile double d1 = NAN;
    volatile double d2 = 1.0;
    volatile float f1 = INFINITY;
    volatile float f2 = -INFINITY;
    
    // Generate various condition codes through complex expressions
    int r1 = (d1 < d2) ? 1 : ((d1 > d2) ? 2 : ((d1 == d2) ? 3 : 4));
    int r2 = (f1 <= f2) ? 5 : ((f1 >= f2) ? 6 : ((f1 != f2) ? 7 : 8));
    int r3 = __builtin_isunordered(d1, d2) ? 9 : 10;
    int r4 = __builtin_isgreater(f1, f2) ? 11 : 12;
    
    checksum += r1 + r2 + r3 + r4;
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
