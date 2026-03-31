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
    
    // Basic comparisons that may generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // May generate LT or UNLT
    results[idx++] = (f1 > f2) ? 2 : 0;      // May generate GT or UNGT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // May generate LE or UNLE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // May generate GE or UNGE
    results[idx++] = (f1 == f2) ? 5 : 0;     // May generate EQ or UNEQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // May generate NEQ or LTGT
    
    // Double comparisons
    results[idx++] = (d1 < d2) ? 7 : 0;
    results[idx++] = (d1 > d2) ? 8 : 0;
    results[idx++] = (d1 <= d2) ? 9 : 0;
    results[idx++] = (d1 >= d2) ? 10 : 0;
    results[idx++] = (d1 == d2) ? 11 : 0;
    results[idx++] = (d1 != d2) ? 12 : 0;
    
    // Complex expressions with mixed types
    results[idx++] = ((f1 < f2) && (d1 > d2)) ? 13 : 0;
    results[idx++] = ((f1 == f2) || (d1 != d2)) ? 14 : 0;
    
    // Nested ternary with floating comparisons
    int val = (f1 < f2) ? 
              ((d1 > d2) ? 15 : 16) : 
              ((f1 == f2) ? 17 : 18);
    results[idx++] = val;
    
    // Switch statement based on comparison results
    switch((f1 < f2) ? 1 : (f1 > f2) ? 2 : (f1 == f2) ? 3 : 4) {
        case 1: results[idx++] = 19; break;
        case 2: results[idx++] = 20; break;
        case 3: results[idx++] = 21; break;
        default: results[idx++] = 22; break;
    }
}

// Test builtin functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct builtin calls - these often map to specific condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 23 : 0;      // May generate GT/UNGT
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 24 : 0; // May generate GE/UNGE
    results[idx++] = __builtin_isless(f1, f2) ? 25 : 0;         // May generate LT/UNLT
    results[idx++] = __builtin_islessequal(f1, f2) ? 26 : 0;    // May generate LE/UNLE
    results[idx++] = __builtin_islessgreater(f1, f2) ? 27 : 0;  // May generate LTGT
    results[idx++] = __builtin_isunordered(f1, f2) ? 28 : 0;    // May generate UNORDERED
    results[idx++] = !__builtin_isunordered(f1, f2) ? 29 : 0;   // May generate ORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 30 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 31 : 0;
    
    // Combined builtins
    results[idx++] = (__builtin_isgreater(f1, f2) && !__builtin_isunordered(f1, f2)) ? 32 : 0;
    results[idx++] = (__builtin_isless(f1, f2) || __builtin_isunordered(f1, f2)) ? 33 : 0;
    
    // Classification functions
    results[idx++] = isnan(f1) ? 34 : 0;
    results[idx++] = isinf(f1) ? 35 : 0;
    results[idx++] = fpclassify(f1) == FP_NAN ? 36 : 0;
    results[idx++] = fpclassify(f1) == FP_INFINITE ? 37 : 0;
    results[idx++] = fpclassify(f1) == FP_NORMAL ? 38 : 0;
}

// Test vector comparisons
void test_vector(v4sf v1, v4sf v2, v2df dv1, v2df dv2, int* results) {
    int idx = 0;
    
    // Vector comparisons - these may generate packed comparisons
    v4sf cmp1 = v1 < v2;
    v4sf cmp2 = v1 > v2;
    v4sf cmp3 = v1 <= v2;
    v4sf cmp4 = v1 >= v2;
    v4sf cmp5 = v1 == v2;
    v4sf cmp6 = v1 != v2;
    
    // Reduce vector to scalar
    float* fcmp1 = (float*)&cmp1;
    float* fcmp2 = (float*)&cmp2;
    
    results[idx++] = (fcmp1[0] != 0.0f) ? 39 : 0;
    results[idx++] = (fcmp2[1] != 0.0f) ? 40 : 0;
    results[idx++] = (fcmp1[2] != 0.0f) ? 41 : 0;
    results[idx++] = (fcmp2[3] != 0.0f) ? 42 : 0;
    
    // Double vector comparisons
    v2df cmp_d1 = dv1 < dv2;
    v2df cmp_d2 = dv1 > dv2;
    
    double* dcmp1 = (double*)&cmp_d1;
    results[idx++] = (dcmp1[0] != 0.0) ? 43 : 0;
    results[idx++] = (dcmp1[1] != 0.0) ? 44 : 0;
    
    // Mixed vector/scalar
    float f = v1[0];
    results[idx++] = (f < v2[0]) ? 45 : 0;
    results[idx++] = (v1[1] > f) ? 46 : 0;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    unsigned char byte_result;
    
    // Inline assembly with various condition codes
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 47 : 0;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 48 : 0;
    
    // Test unordered comparisons
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 49 : 0;
    
    // Test ordered comparison
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 50 : 0;
    
    // More condition codes
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 51 : 0;
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 52 : 0;
    
    // Using "g" constraint to let compiler choose register
    int int_result;
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=g"(int_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = int_result ? 53 : 0;
}

// Complex control flow with floating comparisons
void test_control_flow(float* farr, double* darr, int size, int* results) {
    int idx = 0;
    
    for (int i = 0; i < size - 1; i++) {
        // Loop with floating comparisons
        if (farr[i] < farr[i + 1]) {
            results[idx++] = 54;
            continue;
        } else if (farr[i] > farr[i + 1]) {
            results[idx++] = 55;
            if (darr[i] != darr[i + 1]) {
                results[idx++] = 56;
                goto skip_point;
            }
        } else if (farr[i] == farr[i + 1]) {
            results[idx++] = 57;
        } else {
            // Unordered case
            results[idx++] = 58;
        }
        
        skip_point:
        // Nested loop with more comparisons
        for (int j = 0; j < 2; j++) {
            if (isnan(farr[i])) {
                results[idx++] = 59;
                break;
            } else if (isinf(darr[j])) {
                results[idx++] = 60;
                continue;
            }
        }
    }
    
    // Switch with floating point conditions
    float f = farr[0];
    double d = darr[0];
    
    int case_val = (f < 0.0f) ? 0 : 
                   (f > 0.0f) ? 1 : 
                   (f == 0.0f) ? 2 : 3;
    
    switch(case_val) {
        case 0:
            results[idx++] = (d < 0.0) ? 61 : 62;
            break;
        case 1:
            results[idx++] = (d > 0.0) ? 63 : 64;
            break;
        case 2:
            results[idx++] = (d == 0.0) ? 65 : 66;
            break;
        default:
            results[idx++] = __builtin_isunordered(f, 0.0f) ? 67 : 68;
            break;
    }
}

int main() {
    // Initialize test data with special values
    float fvals[] = {
        1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f,
        3.14f, -2.71f, 100.0f, -100.0f, 1.0e-10f, 1.0e10f
    };
    
    double dvals[] = {
        1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0,
        3.141592653589793, -2.718281828459045,
        1.0e-100, 1.0e100, -1.0e-100, -1.0e100
    };
    
    // Vector data
    v4sf v1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf v2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df dv1 = {1.0, NAN};
    v2df dv2 = {NAN, 1.0};
    
    int results[200] = {0};
    int result_idx = 0;
    
    // Run all tests with various value combinations
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            test_scalar_cmps(fvals[i], fvals[j], dvals[i], dvals[j], 
                           &results[result_idx]);
            result_idx += 20;
            
            test_builtins(fvals[i], fvals[j], dvals[i], dvals[j],
                         &results[result_idx]);
            result_idx += 20;
        }
    }
    
    // Test vectors
    test_vector(v1, v2, dv1, dv2, &results[result_idx]);
    result_idx += 15;
    
    // Test inline assembly
    for (int i = 0; i < 3; i++) {
        test_asm(fvals[i], fvals[i+1], dvals[i], dvals[i+1],
                &results[result_idx]);
        result_idx += 10;
    }
    
    // Test control flow
    test_control_flow(fvals, dvals, 
                     sizeof(fvals)/sizeof(fvals[0]),
                     &results[result_idx]);
    result_idx += 30;
    
    // Calculate checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < result_idx && i < 200; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    global_checksum = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed. Condition codes tested:\n");
    printf("- UNORDERED (via __builtin_isunordered, isnan, setp)\n");
    printf("- ORDERED (via !__builtin_isunordered, setnp)\n");
    printf("- UNEQ (via == with NaN possibility)\n");
    printf("- UNGE (via >= with NaN, __builtin_isgreaterequal)\n");
    printf("- UNGT (via > with NaN, __builtin_isgreater)\n");
    printf("- UNLE (via <= with NaN, __builtin_islessequal)\n");
    printf("- UNLT (via < with NaN, __builtin_isless)\n");
    printf("- LTGT (via != with NaN, __builtin_islessgreater)\n");
    
    return 0;
}
