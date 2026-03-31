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
            results[idx] = 13;
            if (d1 > d2) {
                results[idx] += 1;
                continue;
            }
        } else if (f1 == f2) {
            results[idx] = 14;
            break;
        } else {
            results[idx] = 15;
        }
        idx++;
        
        // Nested ternary forcing CMOV
        int val = (f1 < f2) ? ((d1 > d2) ? 16 : 17) : 
                  (f1 == f2) ? 18 : 19;
        results[idx++] = val;
    }
    
    // Switch statement with floating comparisons
    switch (fpclassify(f1)) {
        case FP_NAN:
            results[idx++] = 20;
            if (isunordered(f1, f2)) {
                results[idx++] = 21;
            }
            break;
        case FP_INFINITE:
            results[idx++] = 22;
            if (f1 > f2) {
                results[idx++] = 23;
            }
            break;
        case FP_ZERO:
            results[idx++] = 24;
            // Fall through
        default:
            results[idx++] = 25;
            goto end_label;
    }
    
end_label:
    results[idx++] = 26;
}

// Test builtin functions
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Builtins that directly map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 30 : 0;      // UNLE inverse
    results[idx++] = __builtin_isless(f1, f2) ? 31 : 0;         // UNGE inverse
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 32 : 0; // UNLT inverse
    results[idx++] = __builtin_islessequal(f1, f2) ? 33 : 0;    // UNGT inverse
    
    results[idx++] = __builtin_isunordered(f1, f2) ? 34 : 0;    // UNORDERED
    results[idx++] = !__builtin_isunordered(f1, f2) ? 35 : 0;   // ORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 36 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 37 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 38 : 0;
    
    // Mixed float/double comparisons
    results[idx++] = (f1 < (float)d1) ? 39 : 0;
    results[idx++] = ((double)f2 > d2) ? 40 : 0;
    
    // Complex expression with multiple builtins
    for (int i = 0; i < 5; i++) {
        int val = __builtin_isunordered(f1, f2) ? 41 : 
                 (__builtin_isgreater(f1, f2) ? 42 : 43);
        results[idx++] = val;
        
        // Nested condition
        if (__builtin_islessequal(d1, d2) && !__builtin_isunordered(d1, d2)) {
            results[idx++] = 44;
            continue;
        }
        results[idx++] = 45;
    }
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vec1 = {f1, f2, f1 * 2.0f, f2 * 0.5f};
    v4sf vec2 = {f2, f1, f2 * 2.0f, f1 * 0.5f};
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d1};
    
    int idx = 0;
    
    // Vector comparisons - these generate packed comparisons
    v4sf cmp_result = vec1 < vec2;
    v4sf cmp_result2 = vec1 > vec2;
    v4sf cmp_result3 = vec1 == vec2;
    
    // Reduce to scalar
    float* fr = (float*)&cmp_result;
    float* fr2 = (float*)&cmp_result2;
    float* fr3 = (float*)&cmp_result3;
    
    for (int i = 0; i < 4; i++) {
        results[idx++] = fr[i] != 0.0f ? 50 + i : 0;
        results[idx++] = fr2[i] != 0.0f ? 54 + i : 0;
        results[idx++] = fr3[i] != 0.0f ? 58 + i : 0;
    }
    
    // Double vector comparisons
    v2df dcmp_result = dvec1 < dvec2;
    v2df dcmp_result2 = dvec1 > dvec2;
    
    double* dr = (double*)&dcmp_result;
    double* dr2 = (double*)&dcmp_result2;
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dr[i] != 0.0 ? 62 + i : 0;
        results[idx++] = dr2[i] != 0.0 ? 64 + i : 0;
    }
    
    // Mixed vector/scalar comparison
    v4sf vec_scalar_cmp = vec1 < (v4sf){f2, f2, f2, f2};
    float* fscalar = (float*)&vec_scalar_cmp;
    for (int i = 0; i < 4; i++) {
        results[idx++] = fscalar[i] != 0.0f ? 66 + i : 0;
    }
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    int8_t byte_result;
    
    // Test various condition codes in inline assembly
    // Using 'g' constraint to let compiler choose register
    
    // UNORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 70 : 0;
    
    // ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 71 : 0;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 72 : 0;
    
    // UNGE (not less than)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  // Note swapped order for nlt
        "setnb %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 73 : 0;
    
    // UNGT (not less or equal)
    __asm__ volatile (
        "ucomiss %2, %1\n\t"  // Note swapped order for nle
        "setnbe %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 74 : 0;
    
    // UNLE (unordered or less or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 75 : 0;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 76 : 0;
    
    // LTGT (less than or greater than, but not equal and not unordered)
    // This is "not equal and ordered" which is SETNE after ordered check
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=g"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 77 : 0;
    
    // Double precision versions
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=g"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 78 : 0;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=g"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 79 : 0;
}

int main() {
    // Initialize test values including special floating-point values
    float float_vals[] = {
        1.0f, 2.0f, 0.0f, -1.0f, 
        INFINITY, -INFINITY, NAN, 3.14f
    };
    
    double double_vals[] = {
        1.0, 2.0, 0.0, -1.0,
        INFINITY, -INFINITY, NAN, 3.141592653589793
    };
    
    int results[500] = {0};
    int result_idx = 0;
    
    // Run tests with various combinations of values
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 3 == 0) {  // Vary which tests run
                test_scalar_cmps(float_vals[i], float_vals[j],
                                double_vals[i], double_vals[j],
                                &results[result_idx]);
                result_idx += 50;
                
                test_builtins(float_vals[i], float_vals[j],
                             double_vals[i], double_vals[j],
                             &results[result_idx]);
                result_idx += 50;
                
                test_vector(float_vals[i], float_vals[j],
                           double_vals[i], double_vals[j],
                           &results[result_idx]);
                result_idx += 50;
                
                test_asm(float_vals[i], float_vals[j],
                        double_vals[i], double_vals[j],
                        &results[result_idx]);
                result_idx += 50;
            }
        }
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
