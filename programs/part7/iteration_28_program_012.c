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
    results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT
    results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE
    results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT or UNE
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex control flow with condition codes
    for (int i = 0; i < 10; i++) {
        if (f1 < f2) {
            results[idx] = i;
            idx++;
            if (d1 > d2) {
                continue;
            }
        } else if (f1 > f2) {
            results[idx] = i * 2;
            idx++;
            goto skip_point;
        } else if (f1 == f2) {
            results[idx] = i * 3;
            idx++;
            break;
        }
        skip_point:
        if (i % 2 == 0) {
            results[idx] = (d1 <= d2) ? 100 : 200;
            idx++;
        }
    }
    
    // Switch statement with floating comparisons
    switch (fpclassify(f1)) {
        case FP_NAN:
            results[idx++] = (isnan(f2)) ? 13 : 14;
            break;
        case FP_INFINITE:
            results[idx++] = (isinf(d1)) ? 15 : 16;
            break;
        case FP_ZERO:
            results[idx++] = (d1 == 0.0) ? 17 : 18;
            break;
        default:
            results[idx++] = (f1 == f2) ? 19 : 20;
            if (d1 < d2) {
                results[idx++] = 21;
            }
    }
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct builtin calls that map to condition codes
    results[idx++] = __builtin_isunordered(f1, f2) ? 30 : 31;    // UNORDERED
    results[idx++] = __builtin_isgreater(f1, f2) ? 32 : 33;      // UNLE inverse
    results[idx++] = __builtin_isless(f1, f2) ? 34 : 35;         // UNGE inverse
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 36 : 37; // UNLT inverse
    results[idx++] = __builtin_islessequal(f1, f2) ? 38 : 39;    // UNGT inverse
    
    // Double versions
    results[idx++] = __builtin_isunordered(d1, d2) ? 40 : 41;
    results[idx++] = __builtin_isgreater(d1, d2) ? 42 : 43;
    results[idx++] = __builtin_isless(d1, d2) ? 44 : 45;
    
    // Combined expressions
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isgreater(d1, d2)) ? 46 : 47;
    
    // Nested ternary with builtins
    results[idx++] = __builtin_isunordered(f1, f2) ? 
                    (__builtin_isless(d1, d2) ? 48 : 49) : 
                    (__builtin_isgreater(f1, f2) ? 50 : 51);
}

// Test vector comparisons
void test_vector(float* farr, double* darr, int* results) {
    v4sf vf1 = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vf2 = {farr[4], farr[5], farr[6], farr[7]};
    v2df vd1 = {darr[0], darr[1]};
    v2df vd2 = {darr[2], darr[3]};
    
    // Vector comparisons - these generate packed comparisons
    v4sf vcmp_f = vf1 < vf2;
    v2df vcmp_d = vd1 > vd2;
    
    // Reduce to scalar (forces condition code generation)
    float* fcmp = (float*)&vcmp_f;
    double* dcmp = (double*)&vcmp_d;
    
    results[0] = (fcmp[0] != 0.0f) ? 60 : 61;
    results[1] = (fcmp[1] != 0.0f) ? 62 : 63;
    results[2] = (fcmp[2] != 0.0f) ? 64 : 65;
    results[3] = (fcmp[3] != 0.0f) ? 66 : 67;
    
    results[4] = (dcmp[0] != 0.0) ? 68 : 69;
    results[5] = (dcmp[1] != 0.0) ? 70 : 71;
    
    // Mixed vector-scalar
    results[6] = ((vf1 < vf2)[0] && (vd1 > vd2)[1]) ? 72 : 73;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    unsigned char byte1, byte2, byte3, byte4;
    
    // Inline assembly that uses condition code names
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte2)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    // Test unordered/ordered conditions
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte3)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte4)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    results[0] = byte1;
    results[1] = byte2;
    results[2] = byte3;
    results[3] = byte4;
    
    // More complex assembly with multiple condition codes
    int val1, val2;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "jbe 1f\n\t"
        "movl $100, %0\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "movl $200, %0\n\t"
        "2:\n\t"
        "ucomisd %4, %5\n\t"
        "jne 3f\n\t"
        "movl $300, %1\n\t"
        "jmp 4f\n\t"
        "3:\n\t"
        "movl $400, %1\n\t"
        "4:"
        : "=r"(val1), "=r"(val2)
        : "x"(f1), "x"(f2), "x"(d1), "x"(d2)
        : "cc"
    );
    
    results[4] = val1;
    results[5] = val2;
}

int main() {
    // Initialize test data with special values
    float fvalues[] = {
        1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f
    };
    
    double dvalues[] = {
        1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0
    };
    
    int results[100] = {0};
    int checksum = 0;
    
    // Test all combinations
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == j) continue;
            
            test_scalar_cmps(fvalues[i], fvalues[j], 
                           dvalues[i], dvalues[j], results);
            
            // Update checksum
            for (int k = 0; k < 50; k++) {
                checksum += results[k];
                results[k] = 0;
            }
            
            test_builtins(fvalues[i], fvalues[j], 
                         dvalues[i], dvalues[j], results);
            
            for (int k = 0; k < 50; k++) {
                checksum += results[k];
                results[k] = 0;
            }
            
            test_vector(fvalues, dvalues, results);
            
            for (int k = 0; k < 50; k++) {
                checksum += results[k];
                results[k] = 0;
            }
            
            test_asm(fvalues[i], fvalues[j], 
                    dvalues[i], dvalues[j], results);
            
            for (int k = 0; k < 50; k++) {
                checksum += results[k];
                results[k] = 0;
            }
        }
    }
    
    // Store in volatile to prevent dead code elimination
    global_checksum = checksum;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
