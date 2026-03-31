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
    
    // Basic comparisons that may generate various condition codes
    results[idx++] = (f1 < f2) ? 1 : 0;      // May generate LT or UNLT
    results[idx++] = (f1 > f2) ? 2 : 0;      // May generate GT or UNGT
    results[idx++] = (f1 <= f2) ? 3 : 0;     // May generate LE or UNLE
    results[idx++] = (f1 >= f2) ? 4 : 0;     // May generate GE or UNGE
    results[idx++] = (f1 == f2) ? 5 : 0;     // May generate EQ or UNEQ
    results[idx++] = (f1 != f2) ? 6 : 0;     // May generate NE or LTGT
    
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
    
    // Switch based on comparison results
    switch((f1 < f2) + 2*(f1 > f2) + 4*(f1 == f2)) {
        case 0: results[idx++] = 19; break;
        case 1: results[idx++] = 20; break;
        case 2: results[idx++] = 21; break;
        case 3: results[idx++] = 22; break;
        case 4: results[idx++] = 23; break;
        default: results[idx++] = 24; break;
    }
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    
    // Direct builtin calls that map to condition codes
    results[idx++] = __builtin_isgreater(f1, f2) ? 25 : 0;      // May generate GT/UNGT
    results[idx++] = __builtin_isless(f1, f2) ? 26 : 0;         // May generate LT/UNLT
    results[idx++] = __builtin_isgreaterequal(f1, f2) ? 27 : 0; // May generate GE/UNGE
    results[idx++] = __builtin_islessequal(f1, f2) ? 28 : 0;    // May generate LE/UNLE
    
    results[idx++] = __builtin_isunordered(f1, f2) ? 29 : 0;    // UNORDERED
    results[idx++] = !__builtin_isunordered(f1, f2) ? 30 : 0;   // ORDERED
    
    // Double versions
    results[idx++] = __builtin_isgreater(d1, d2) ? 31 : 0;
    results[idx++] = __builtin_isless(d1, d2) ? 32 : 0;
    results[idx++] = __builtin_isunordered(d1, d2) ? 33 : 0;
    
    // Combined tests
    results[idx++] = (__builtin_isgreater(f1, f2) && __builtin_isless(d1, d2)) ? 34 : 0;
    results[idx++] = (__builtin_isunordered(f1, f2) || __builtin_isunordered(d1, d2)) ? 35 : 0;
    
    // Loop with builtins
    for (int i = 0; i < 3; i++) {
        if (__builtin_isgreater(f1 + i, f2)) {
            results[idx++] = 36 + i;
            continue;
        }
        if (__builtin_isunordered(f1, f2 + i)) {
            results[idx++] = 39 + i;
            break;
        }
    }
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2, int* results) {
    v4sf vf1 = {f1, f2, f1 + 1.0f, f2 - 1.0f};
    v4sf vf2 = {f2, f1, f2 + 1.0f, f1 - 1.0f};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    int idx = 0;
    
    // Vector comparisons - these generate packed comparisons
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 == vf2;
    v4sf cmp4 = vf1 != vf2;
    
    v2df cmp5 = vd1 < vd2;
    v2df cmp6 = vd1 > vd2;
    
    // Reduce to scalar
    float* fcmp1 = (float*)&cmp1;
    float* fcmp2 = (float*)&cmp2;
    double* dcmp5 = (double*)&cmp5;
    
    results[idx++] = (fcmp1[0] != 0.0f) ? 50 : 0;
    results[idx++] = (fcmp2[1] != 0.0f) ? 51 : 0;
    results[idx++] = (fcmp1[2] != 0.0f) ? 52 : 0;
    results[idx++] = (fcmp2[3] != 0.0f) ? 53 : 0;
    
    results[idx++] = (dcmp5[0] != 0.0) ? 54 : 0;
    results[idx++] = (dcmp5[1] != 0.0) ? 55 : 0;
    
    // Mixed vector/scalar
    results[idx++] = ((vf1 < vf2)[0] && (d1 > d2)) ? 56 : 0;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2, int* results) {
    int idx = 0;
    uint8_t byte_result;
    
    // Test various condition codes in inline assembly
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 100 : 0;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 101 : 0;
    
    // Test unordered/ordered conditions
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = byte_result ? 102 : 0;
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    results[idx++] = byte_result ? 103 : 0;
    
    // More complex assembly with multiple conditions
    int int_result;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "jp 1f\n\t"           // Jump if unordered
        "ja 2f\n\t"           // Jump if above (greater and not equal)
        "mov $1, %0\n\t"
        "jmp 3f\n\t"
        "1:\n\t"
        "mov $2, %0\n\t"      // Unordered case
        "jmp 3f\n\t"
        "2:\n\t"
        "mov $3, %0\n\t"      // Greater case
        "3:\n\t"
        : "=r"(int_result)
        : "0"(0), "x"(f1), "x"(f2)
        : "cc"
    );
    results[idx++] = int_result;
}

// Main test driver
int main() {
    // Initialize test values including special floating-point values
    float fvals[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        INFINITY, -INFINITY, NAN,
        3.14f, -2.71f
    };
    
    double dvals[] = {
        1.0, 2.0, 0.0, -1.0,
        INFINITY, -INFINITY, NAN,
        3.141592653589793, -2.718281828459045
    };
    
    int results[200] = {0};
    int result_idx = 0;
    
    // Test various combinations
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j) continue;
            
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Call test functions
            test_scalar_cmps(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 20;
            
            test_builtins(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 15;
            
            test_vector(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 10;
            
            test_asm(f1, f2, d1, d2, &results[result_idx]);
            result_idx += 6;
        }
    }
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 200; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
