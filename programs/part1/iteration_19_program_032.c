#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Prevent optimizations from removing critical code
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

// Test 1: Direct unordered comparisons with NaN
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE normal_val = 3.14159;
    VOLATILE_DOUBLE zero_val = 0.0;
    VOLATILE_DOUBLE inf_val = INFINITY;
    
    int results[8] = {0};
    
    // These should generate UNORDERED/ORDERED condition codes
    results[0] = (nan_val != normal_val) ? 1 : 0;    // unordered comparison
    results[1] = (nan_val == nan_val) ? 1 : 0;       // ordered comparison
    results[2] = (normal_val == normal_val) ? 1 : 0; // ordered comparison
    
    // Force multiple comparison types
    results[3] = isunordered(nan_val, normal_val);
    results[4] = isgreater(normal_val, zero_val);
    results[5] = isless(normal_val, inf_val);
    results[6] = !islessequal(nan_val, normal_val);
    results[7] = !isgreaterequal(nan_val, normal_val);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

// Test 2: Inline assembly with %C modifier for condition codes
NOINLINE int test_asm_condition_codes(void) {
    VOLATILE_DOUBLE a = NAN;
    VOLATILE_DOUBLE b = 2.71828;
    VOLATILE_DOUBLE c = 1.41421;
    VOLATILE_DOUBLE d = NAN;
    
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    // Test UNORDERED condition
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(r1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    // Test ORDERED condition  
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(r2)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    // Test UNEQ condition (unordered or equal)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(r3)
        : "x"(d), "x"(d)
        : "cc"
    );
    
    // Test LTGT condition (less than or greater than)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(r4)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4;
}

// Test 3: Array operations with various comparison macros
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[16];
    VOLATILE_DOUBLE arr2[16];
    
    // Initialize arrays with mix of normal values and NaN
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = NAN;
        } else if (i % 4 == 2) {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)i;
        }
    }
    
    int counts[7] = {0}; // For different comparison types
    
    for (int i = 0; i < 16; i++) {
        // Test all the uncovered condition codes through various comparisons
        counts[0] += isunordered(arr1[i], arr2[i]);      // UNORDERED
        counts[1] += !isunordered(arr1[i], arr2[i]);     // ORDERED
        counts[2] += (!isunordered(arr1[i], arr2[i]) && 
                     !(arr1[i] < arr2[i]) && 
                     !(arr1[i] > arr2[i]));              // UNEQ
        
        counts[3] += !(arr1[i] < arr2[i]);               // UNGE (not less than)
        counts[4] += !(arr1[i] <= arr2[i]);              // UNGT (not less or equal)
        counts[5] += (isunordered(arr1[i], arr2[i]) || 
                     (arr1[i] <= arr2[i]));              // UNLE
        counts[6] += (isunordered(arr1[i], arr2[i]) || 
                     (arr1[i] < arr2[i]));               // UNLT
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

// Test 4: Long double (x87) operations
NOINLINE int test_long_double_ops(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_val1 = 3.14159265358979323846L;
    VOLATILE_LONG_DOUBLE ld_val2 = 2.71828182845904523536L;
    VOLATILE_LONG_DOUBLE ld_zero = 0.0L;
    
    int results = 0;
    
    // x87 style comparisons - these often generate different condition codes
    results += (ld_nan != ld_val1) ? 1 : 0;
    results += (ld_val1 == ld_val1) ? 2 : 0;
    results += (ld_val1 > ld_val2) ? 4 : 0;
    results += (ld_val2 < ld_val1) ? 8 : 0;
    results += (ld_nan == ld_nan) ? 16 : 0;  // This should be false for NaN
    
    // Complex expression to force multiple condition codes
    VOLATILE_LONG_DOUBLE temp = ld_val1;
    for (int i = 0; i < 4; i++) {
        temp = temp * ld_val2 - ld_val1;
        if (isunordered(temp, ld_zero)) {
            results += 32;
            break;
        }
        if (!isgreater(temp, ld_zero)) {
            results += 64;
        }
    }
    
    return results;
}

// Test 5: Switch based on floating-point classification
NOINLINE int test_fpclassify_switch(void) {
    VOLATILE_DOUBLE values[8];
    values[0] = NAN;
    values[1] = INFINITY;
    values[2] = -INFINITY;
    values[3] = 0.0;
    values[4] = -0.0;
    values[5] = 1.0;
    values[6] = -1.0;
    values[7] = 2.5;
    
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        switch (fpclassify(values[i])) {
            case FP_NAN:
                result |= 0x01;
                // Force unordered comparison
                if (isunordered(values[i], values[(i+1)%8])) {
                    result |= 0x02;
                }
                break;
            case FP_INFINITE:
                result |= 0x04;
                // Force ordered comparison
                if (!isunordered(values[i], values[(i+2)%8])) {
                    result |= 0x08;
                }
                break;
            case FP_ZERO:
                result |= 0x10;
                // Test UNEQ (unordered or equal)
                if (!isunordered(values[i], values[(i+3)%8]) && 
                    values[i] == values[(i+3)%8]) {
                    result |= 0x20;
                }
                break;
            case FP_NORMAL:
                result |= 0x40;
                // Test UNGE (not less than)
                if (!(values[i] < values[(i+4)%8])) {
                    result |= 0x80;
                }
                break;
            case FP_SUBNORMAL:
                result |= 0x100;
                break;
        }
    }
    
    return result;
}

// Test 6: Direct use of GCC builtins for SSE2 unordered compares
NOINLINE int test_sse2_builtins(void) {
    VOLATILE_DOUBLE a = NAN;
    VOLATILE_DOUBLE b = 1.0;
    VOLATILE_DOUBLE c = 2.0;
    VOLATILE_DOUBLE d = NAN;
    
    int res = 0;
    
    // Use GCC's x86 intrinsics directly
    res += __builtin_ia32_ucomisd(a, b) ? 1 : 0;
    res += __builtin_ia32_ucomisd(b, c) ? 2 : 0;
    res += __builtin_ia32_ucomisd(c, b) ? 4 : 0;
    res += __builtin_ia32_ucomisd(d, d) ? 8 : 0;
    
    // Force conditional moves based on comparison results
    VOLATILE_DOUBLE x = 5.0;
    VOLATILE_DOUBLE y = 10.0;
    
    // This should generate conditional moves with different condition codes
    double cmov_result;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "j%C0 1f\n\t"
        "movsd %1, %0\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "movsd %2, %0\n\t"
        "2:"
        : "=x"(cmov_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    res += (cmov_result == x) ? 16 : 32;
    
    return res;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    // Run all tests
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_ops();
    checksum += test_fpclassify_switch();
    checksum += test_sse2_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    // Additional volatile operations to prevent dead code elimination
    VOLATILE_DOUBLE final_check = NAN;
    if (isunordered(final_check, final_check)) {
        printf("Final unordered check passed\n");
    }
    
    return 0;
}
