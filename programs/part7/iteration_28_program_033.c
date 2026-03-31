#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent optimization
volatile int global_checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Basic comparisons that should generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT or LT
    results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT or GT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE or LE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE or GE
    results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ or EQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT or NE
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Mixed comparisons in complex control flow
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                results[idx++] = (f1 < f2) ? 13 : 0;
                if (d1 > d2) {
                    results[idx++] = 14;
                    break;
                }
                // fall through
            case 1:
                results[idx++] = (f1 == f2) ? 15 : 0;
                continue;
            case 2:
                results[idx++] = (d1 != d2) ? 16 : 0;
                goto done;
        }
        results[idx++] = 99; // Should be skipped by continue/goto
    }
done:
    results[idx++] = 17;
    
    // Complex ternary chains
    int x = (f1 < f2) ? 
            ((d1 > d2) ? 18 : 19) : 
            ((f1 == f2) ? 20 : 21);
    results[idx++] = x;
    
    // Nested conditionals
    if (f1 <= f2) {
        if (d1 >= d2) {
            results[idx++] = 22;
        } else {
            results[idx++] = 23;
        }
    } else {
        results[idx++] = 24;
    }
}

// Test builtin functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // These builtins directly map to condition codes
    results[idx++] = __builtin_isunordered(f1, f2) ? 1 : 0;    // UNORDERED
    results[idx++] = __builtin_isgreater(f1, f2) ? 2 : 0;      // UNLE inverse
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         // UNGE inverse
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // UNGT inverse
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 5 : 0; // UNLT inverse
    
    // Double versions
    results[idx++] = __builtin_isunordered(d1, d2) ? 6 : 0;
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 8 : 0;
    
    // Ordered comparison
    results[idx++] = !__builtin_isunordered(f1, f2) ? 9 : 0;   // ORDERED
    
    // Complex expressions with builtins
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(f1 + i, f2 - i)) {
            results[idx++] = 10 + i;
            continue;
        }
        if (__builtin_isgreater(d1 * i, d2 / (i+1))) {
            results[idx++] = 20 + i;
            break;
        }
        results[idx++] = 30 + i;
    }
    
    // Classification functions
    results[idx++] = isnan(f1) ? 40 : 0;
    results[idx++] = isinf(d1) ? 41 : 0;
    results[idx++] = fpclassify(f2) == FP_NAN ? 42 : 0;
    results[idx++] = fpclassify(d2) == FP_INFINITE ? 43 : 0;
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f2, f1 * 2, f2 / 2};
    v4sf vf2 = {f2, f1, f2 * 2, f1 / 2};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    int idx = 0;
    
    // Vector comparisons - these generate packed comparisons
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 <= vf2;
    v4sf cmp4 = vf1 >= vf2;
    v4sf cmp5 = vf1 == vf2;
    v4sf cmp6 = vf1 != vf2;
    
    // Reduce to scalar
    float* fcmp1 = (float*)&cmp1;
    float* fcmp2 = (float*)&cmp2;
    results[idx++] = (fcmp1[0] != 0.0f) ? 1 : 0;
    results[idx++] = (fcmp2[1] != 0.0f) ? 2 : 0;
    results[idx++] = (fcmp1[2] != 0.0f) ? 3 : 0;
    results[idx++] = (fcmp2[3] != 0.0f) ? 4 : 0;
    
    // Double vector comparisons
    v2df dcmp1 = vd1 < vd2;
    v2df dcmp2 = vd1 > vd2;
    double* dcmp1p = (double*)&dcmp1;
    double* dcmp2p = (double*)&dcmp2;
    
    results[idx++] = (dcmp1p[0] != 0.0) ? 5 : 0;
    results[idx++] = (dcmp2p[1] != 0.0) ? 6 : 0;
    
    // Mixed vector-scalar in loop
    for (int i = 0; i < 2; i++) {
        v4sf temp = vf1 + i;
        v4sf cmp = temp < vf2;
        float* fcmp = (float*)&cmp;
        for (int j = 0; j < 4; j++) {
            if (fcmp[j] != 0.0f) {
                results[idx++] = 7 + i * 4 + j;
                goto next_iter;
            }
        }
        results[idx++] = 99;
        next_iter:;
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    uint8_t byte_result;
    
    // Test various condition codes via inline assembly
    // These force the assembly printer to resolve condition code names
    
    // Float comparisons
    int f_cmp;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNORDERED test
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // Double comparisons
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // Test "unord" and "ord" specifically
    int is_unord, is_ord;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "setp %0\n\t"
        "setnp %1"
        : "=r"(is_unord), "=r"(is_ord)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_unord;
    results[idx++] = is_ord;
    
    // Test "ueq" (unordered or equal)
    int is_ueq;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(is_ueq)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_ueq;
    
    // Test "nlt" (not less than) = UNGE
    int is_nlt;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r"(is_nlt)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_nlt;
    
    // Test "nle" (not less or equal) = UNGT
    int is_nle;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r"(is_nle)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_nle;
    
    // Test "ule" (unordered or less or equal)
    int is_ule;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(is_ule)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_ule;
    
    // Test "ult" (unordered or less than)
    int is_ult;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(is_ult)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_ult;
    
    // Test "une" (unordered or not equal) = LTGT
    int is_une;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(is_une)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = is_une;
}

int main() {
    // Initialize test values including special values
    float f_values[] = {
        1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f,
        3.14f, -2.71f, 1.0e10f, 1.0e-10f
    };
    
    double d_values[] = {
        1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0,
        3.141592653589793, -2.718281828459045,
        1.0e100, 1.0e-100
    };
    
    int results[500];
    memset(results, 0, sizeof(results));
    
    int result_idx = 0;
    
    // Test various combinations of values
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            float f1 = f_values[i];
            float f2 = f_values[j];
            double d1 = d_values[i];
            double d2 = d_values[j];
            
            // Call test functions
            test_scalar_cmps(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
            
            test_builtins(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
            
            test_vector(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
            
            test_asm(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 30;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
