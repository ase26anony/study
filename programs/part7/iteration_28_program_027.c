#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 32

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
static float fvals[ARRAY_SIZE];
static double dvals[ARRAY_SIZE];
static int results[ARRAY_SIZE * 4];
static volatile int checksum = 0;

// Initialize arrays with normal and special values
__attribute__((constructor))
static void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fvals[i] = (i * 1.5f) - 15.0f;
        dvals[i] = (i * 2.3) - 25.0;
    }
    
    // Insert special values at strategic positions
    fvals[0] = 0.0f / 0.0f;      // NaN
    fvals[1] = 1.0f / 0.0f;      // +Inf
    fvals[2] = -1.0f / 0.0f;     // -Inf
    fvals[3] = -0.0f;            // Negative zero
    
    dvals[0] = 0.0 / 0.0;        // NaN
    dvals[1] = 1.0 / 0.0;        // +Inf
    dvals[2] = -1.0 / 0.0;       // -Inf
    dvals[3] = -0.0;             // Negative zero
}

// Test scalar comparisons with all relational operators
__attribute__((noinline))
static void test_scalar_cmps() {
    int idx = 0;
    
    // Complex control flow with nested if-else and switch
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float f1 = fvals[i];
        float f2 = fvals[(i + 1) % ARRAY_SIZE];
        double d1 = dvals[i];
        double d2 = dvals[(i + 3) % ARRAY_SIZE];
        
        // Use ternary operator to force CMOV/SET generation
        results[idx++] = (f1 < f2) ? 1 : 0;      // May generate UNLT/UNORDERED
        results[idx++] = (f1 > f2) ? 2 : 0;      // May generate UNGT/UNORDERED
        results[idx++] = (f1 <= f2) ? 3 : 0;     // May generate UNLE/UNORDERED
        results[idx++] = (f1 >= f2) ? 4 : 0;     // May generate UNGE/UNORDERED
        results[idx++] = (f1 == f2) ? 5 : 0;     // May generate UNEQ/UNORDERED
        results[idx++] = (f1 != f2) ? 6 : 0;     // May generate LTGT/UNORDERED
        
        // Double comparisons
        results[idx++] = (d1 < d2) ? 7 : 0;
        results[idx++] = (d1 > d2) ? 8 : 0;
        results[idx++] = (d1 <= d2) ? 9 : 0;
        results[idx++] = (d1 >= d2) ? 10 : 0;
        results[idx++] = (d1 == d2) ? 11 : 0;
        results[idx++] = (d1 != d2) ? 12 : 0;
        
        // Classification functions that may generate ORDERED/UNORDERED
        switch (fpclassify(f1)) {
            case FP_NAN:
                results[idx++] = 100;
                break;
            case FP_INFINITE:
                results[idx++] = 101;
                break;
            case FP_ZERO:
                results[idx++] = 102;
                break;
            case FP_SUBNORMAL:
                results[idx++] = 103;
                break;
            case FP_NORMAL:
                results[idx++] = 104;
                break;
            default:
                goto skip_label;  // Force complex CFG
        }
        
        skip_label:
        // More complex conditional moves
        int* ptr = &results[idx];
        *ptr = (isnan(d1) && !isnan(d2)) ? 200 : 
               (isinf(d1) && !isinf(d2)) ? 201 : 
               (d1 == d2) ? 202 : 0;
        idx++;
        
        // Unordered comparisons with explicit checks
        if (isunordered(f1, f2)) {
            results[idx++] = 300;
            continue;  // Force different basic block structure
        }
        
        if (isnan(f1) || isnan(f2)) {
            results[idx++] = 301;
        }
    }
}

// Test builtin unordered comparison functions
__attribute__((noinline))
static void test_builtins() {
    int idx = ARRAY_SIZE * 2;
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        float f1 = fvals[i];
        float f2 = fvals[i + ARRAY_SIZE/2];
        double d1 = dvals[i];
        double d2 = dvals[i + ARRAY_SIZE/2];
        
        // These builtins directly map to condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // UNLE?
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0; // UNLT?
        results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         // UNGE?
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // UNGT?
        results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;  // UNEQ?
        results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;    // UNORDERED
        
        // Double versions
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
        
        // Mixed float/double comparisons
        results[idx++] = (f1 < (float)d2) ? 13 : 0;
        results[idx++] = ((double)f1 > d2) ? 14 : 0;
        
        // Complex expression with multiple builtins
        int val = __builtin_isunordered(f1, f2) ? 100 :
                  __builtin_isgreater(f1, f2) ? 101 :
                  __builtin_isless(f1, f2) ? 102 : 103;
        results[idx++] = val;
    }
}

// Test vector/SIMD comparisons
__attribute__((noinline))
static void test_vector() {
    v4sf vec1, vec2, vec3;
    v2df dvec1, dvec2;
    
    // Initialize vectors
    for (int i = 0; i < 4; i++) {
        vec1[i] = fvals[i];
        vec2[i] = fvals[i + 4];
        vec3[i] = fvals[i + 8];
    }
    
    for (int i = 0; i < 2; i++) {
        dvec1[i] = dvals[i];
        dvec2[i] = dvals[i + 2];
    }
    
    // Vector comparisons - these may generate packed comparisons
    // that later need scalar condition code extraction
    v4sf cmp_result;
    v2df dbl_cmp_result;
    
    cmp_result = vec1 < vec2;      // May generate UNLT
    cmp_result = vec1 > vec3;      // May generate UNGT
    cmp_result = vec1 <= vec2;     // May generate UNLE
    cmp_result = vec1 >= vec3;     // May generate UNGE
    cmp_result = vec1 == vec2;     // May generate UNEQ
    cmp_result = vec1 != vec3;     // May generate LTGT
    
    dbl_cmp_result = dvec1 < dvec2;
    dbl_cmp_result = dvec1 > dvec2;
    dbl_cmp_result = dvec1 <= dvec2;
    dbl_cmp_result = dvec1 >= dvec2;
    dbl_cmp_result = dvec1 == dvec2;
    dbl_cmp_result = dvec1 != dvec2;
    
    // Reduce vector to scalar mask - forces condition code generation
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        if (cmp_result[i] != 0.0f) {
            mask |= (1 << i);
        }
    }
    
    results[ARRAY_SIZE * 3] = mask;
    
    // More complex vector operations in loop
    for (int i = 0; i < ARRAY_SIZE - 4; i += 4) {
        v4sf v1 = *(v4sf*)&fvals[i];
        v4sf v2 = *(v4sf*)&fvals[i + 4];
        v4sf cmp = v1 < v2;
        
        // Extract individual comparison results
        for (int j = 0; j < 4; j++) {
            results[ARRAY_SIZE * 3 + 1 + i/4] |= (cmp[j] != 0.0f) ? (1 << j) : 0;
        }
    }
}

// Test inline assembly with condition code constraints
__attribute__((noinline))
static void test_asm() {
    unsigned char byte_results[16];
    
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[i + 8];
        double d1 = dvals[i];
        double d2 = dvals[i + 8];
        
        // Test various condition codes via inline assembly
        // These force the assembly printer to resolve condition code names
        
        // UNORDERED
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setp %0"
            : "=r"(byte_results[i*2])
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        // ORDERED
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setnp %0"
            : "=r"(byte_results[i*2 + 1])
            : "x"(f1), "x"(f2)
            : "cc"
        );
        
        // UNEQ (unordered or equal)
        unsigned char tmp;
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "sete %0"
            : "=r"(tmp)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        byte_results[8 + i] = tmp;
        
        // UNGE (not less than) - using "nlt" output
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setnb %0"
            : "=r"(tmp)
            : "x"(f1), "x"(f2)
            : "cc"
        );
        byte_results[12 + i] = tmp;
        
        // UNGT (not less or equal) - using "nle" output
        __asm__ goto (
            "ucomisd %1, %2\n\t"
            "ja %l0"
            : 
            : "x"(d1), "x"(d2)
            : "cc"
            : asm_label
        );
        
        byte_results[14] = 1;
        continue;
        
        asm_label:
        byte_results[14] = 0;
    }
    
    // Aggregate assembly results
    for (int i = 0; i < 16; i++) {
        results[ARRAY_SIZE * 3 + 8 + i] = byte_results[i];
    }
}

// Main test driver
int main() {
    // Initialize
    init_arrays();
    
    // Run all tests
    test_scalar_cmps();
    test_builtins();
    test_vector();
    test_asm();
    
    // Compute checksum to prevent dead code elimination
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        checksum += results[i];
    }
    
    // Also use the values in printf to ensure they're live
    printf("Checksum: %d\n", checksum);
    
    // Additional complex control flow with floating comparisons
    volatile float test_val = 0.0f;
    for (int i = 0; i < 100; i++) {
        test_val += 0.1f;
        
        // This switch with floating comparisons may generate
        // various condition codes for the case comparisons
        switch (i % 8) {
            case 0:
                if (test_val < 1.0f) checksum++;
                break;
            case 1:
                if (test_val > 2.0f) checksum++;
                break;
            case 2:
                if (test_val <= 3.0f) checksum++;
                break;
            case 3:
                if (test_val >= 4.0f) checksum++;
                break;
            case 4:
                if (test_val == 5.0f) checksum++;
                break;
            case 5:
                if (test_val != 6.0f) checksum++;
                break;
            case 6:
                if (isnan(test_val)) checksum++;
                break;
            case 7:
                if (isinf(test_val)) checksum++;
                break;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
