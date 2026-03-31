#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2, int* results) {
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
    results[idx++] = (d1 == d2) ? 11 : 0;    // EQ
    results[idx++] = (d1 != d2) ? 12 : 0;    // NEQ
    
    // Complex conditional expressions to force CMOV generation
    double temp = (d1 < d2) ? d1 : d2;
    results[idx++] = (int)temp;
    
    float ftemp = (f1 > f2) ? f1 : f2;
    results[idx++] = (int)ftemp;
    
    // Nested ternary with mixed comparisons
    int complex = (f1 < f2) ? ((d1 > d2) ? 100 : 200) : ((d1 == d2) ? 300 : 400);
    results[idx++] = complex;
    
    // Switch statement with floating comparisons
    switch(fpclassify(f1)) {
        case FP_NAN:
            results[idx++] = 1000;
            break;
        case FP_INFINITE:
            results[idx++] = 2000;
            break;
        case FP_ZERO:
            results[idx++] = 3000;
            break;
        case FP_SUBNORMAL:
            results[idx++] = 4000;
            break;
        case FP_NORMAL:
            results[idx++] = 5000;
            break;
        default:
            results[idx++] = 6000;
    }
    
    // Loop with floating comparisons
    for (int i = 0; i < 5; i++) {
        if (f1 < f2) {
            results[idx++] = i * 10;
            continue;
        }
        if (d1 > d2) {
            results[idx++] = i * 20;
            break;
        }
        results[idx++] = i * 30;
    }
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct builtin calls that map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;        // GT (unordered)
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0;   // GE (unordered)
    results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;           // LT (unordered)
    results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;      // LE (unordered)
    results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;    // LTGT
    results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;      // UNORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
    results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
    results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
    results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
    
    // Combined expressions
    results[idx++] = __builtin_isunordered(f1, f2) || __builtin_isgreater(f1, f2) ? 100 : 200;
    results[idx++] = !__builtin_isunordered(d1, d2) && __builtin_isless(d1, d2) ? 300 : 400;
    
    // Conditional move with builtins
    double val = __builtin_isunordered(d1, d2) ? 0.0 : (__builtin_isgreater(d1, d2) ? d1 : d2);
    results[idx++] = (int)val;
}

// Test vector comparisons
void test_vector(v4sf vf1, v4sf vf2, v2df vd1, v2df vd2, int* results) {
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
    results[idx++] = (fcmp2[0] != 0.0f) ? 2 : 0;
    results[idx++] = (fcmp1[1] != 0.0f) ? 3 : 0;
    results[idx++] = (fcmp2[1] != 0.0f) ? 4 : 0;
    
    // Double vector comparisons
    v2df dcmp1 = vd1 < vd2;
    v2df dcmp2 = vd1 > vd2;
    v2df dcmp3 = vd1 <= vd2;
    v2df dcmp4 = vd1 >= vd2;
    
    double* dcmp = (double*)&dcmp1;
    results[idx++] = (dcmp[0] != 0.0) ? 5 : 0;
    results[idx++] = (dcmp[1] != 0.0) ? 6 : 0;
    
    // Mixed vector-scalar operations
    for (int i = 0; i < 4; i++) {
        if (fcmp1[i] != 0.0f) {
            results[idx++] = i * 100;
            if (i == 2) goto vector_label;
        }
    }
    
vector_label:
    // Complex control flow with vector results
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        mask |= (fcmp1[i] != 0.0f) ? (1 << i) : 0;
    }
    results[idx++] = mask;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    uint8_t byte_result;
    
    // Test various condition codes via inline assembly
    // These force the assembly printer to resolve condition code names
    
    // UNORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNGT (not less than or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNLE (unordered or less than or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // LTGT (less than or greater than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // Double precision versions
    // UNORDERED for double
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // ORDERED for double
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result;
    
    // Mixed condition codes in complex expressions
    int combined;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "setp %%al\n\t"
        "ucomisd %4, %5\n\t"
        "setnp %%bl\n\t"
        "or %%al, %%bl\n\t"
        "movzbl %%bl, %0"
        : "=r"(combined)
        : "0"(0), "x"(f1), "x"(f2), "x"(d1), "x"(d2)
        : "al", "bl", "cc"
    );
    results[idx++] = combined;
}

int main() {
    // Initialize test data with special values
    float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f};
    double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0};
    
    // Vector data
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    // Results array
    int results[200];
    memset(results, 0, sizeof(results));
    
    // Run tests with various combinations
    int result_idx = 0;
    
    // Test all combinations of scalar values
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (result_idx + 50 < 200) {
                test_scalar_cmps(fvals[i], fvals[j], dvals[i], dvals[j], 
                                &results[result_idx]);
                result_idx += 20;
                
                test_builtins(fvals[i], fvals[j], dvals[i], dvals[j],
                             &results[result_idx]);
                result_idx += 15;
            }
        }
    }
    
    // Test vectors
    test_vector(vf1, vf2, vd1, vd2, &results[result_idx]);
    result_idx += 10;
    
    // Test assembly with interesting value pairs
    test_asm(NAN, 1.0f, NAN, 1.0, &results[result_idx]);
    result_idx += 12;
    test_asm(1.0f, NAN, 1.0, NAN, &results[result_idx]);
    result_idx += 12;
    test_asm(INFINITY, 1.0f, INFINITY, 1.0, &results[result_idx]);
    result_idx += 12;
    test_asm(1.0f, INFINITY, 1.0, INFINITY, &results[result_idx]);
    result_idx += 12;
    test_asm(-INFINITY, INFINITY, -INFINITY, INFINITY, &results[result_idx]);
    result_idx += 12;
    
    // Compute checksum to prevent dead code elimination
    for (int i = 0; i < 200; i++) {
        checksum += results[i];
    }
    
    // Print checksum (prevents optimization)
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
