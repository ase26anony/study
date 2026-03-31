#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[16] = {
    1.0f, 2.0f, 3.0f, NAN,
    -1.0f, INFINITY, -INFINITY, 0.0f,
    4.0f, 5.0f, 6.0f, 7.0f,
    8.0f, 9.0f, 10.0f, 11.0f
};

double darr[16] = {
    1.0, 2.0, NAN, 3.0,
    INFINITY, -INFINITY, 4.0, 5.0,
    6.0, 7.0, 8.0, 9.0,
    10.0, 11.0, 12.0, 13.0
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32];
    int idx = 0;
    
    // Complex control flow with nested if-else and switch
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i+1];
        double d1 = darr[i];
        double d2 = darr[i+1];
        
        // Use ternary operators to force CMOV/SET generation
        results[idx++] = (f1 < f2) ? 1 : 0;
        results[idx++] = (f1 > f2) ? 2 : 0;
        results[idx++] = (f1 <= f2) ? 3 : 0;
        results[idx++] = (f1 >= f2) ? 4 : 0;
        results[idx++] = (f1 == f2) ? 5 : 0;
        results[idx++] = (f1 != f2) ? 6 : 0;
        
        // Double comparisons
        results[idx++] = (d1 < d2) ? 7 : 0;
        results[idx++] = (d1 > d2) ? 8 : 0;
        results[idx++] = (d1 <= d2) ? 9 : 0;
        results[idx++] = (d1 >= d2) ? 10 : 0;
        results[idx++] = (d1 == d2) ? 11 : 0;
        results[idx++] = (d1 != d2) ? 12 : 0;
        
        // Classification functions to generate various condition codes
        switch (fpclassify(f1)) {
            case FP_NAN:
                results[idx++] = 13;
                break;
            case FP_INFINITE:
                results[idx++] = 14;
                break;
            case FP_ZERO:
                results[idx++] = 15;
                break;
            case FP_SUBNORMAL:
                results[idx++] = 16;
                break;
            case FP_NORMAL:
                results[idx++] = 17;
                break;
            default:
                results[idx++] = 18;
        }
        
        // More complex conditional expressions
        int temp = 0;
        if (isnan(f1) && !isinf(f2)) {
            temp = 19;
        } else if (isunordered(f1, f2)) {
            temp = 20;
        } else if (f1 != f2 && !isnan(f1)) {
            temp = 21;
        }
        results[idx++] = temp;
    }
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test built-in unordered comparison functions
int test_builtins(void) {
    int results[24];
    int idx = 0;
    
    for (int i = 0; i < 6; i++) {
        float f1 = farr[i];
        float f2 = farr[i+2];
        double d1 = darr[i];
        double d2 = darr[i+2];
        
        // Built-in functions that directly map to condition codes
        results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 2 : 0;
        results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;
        results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;
        results[idx++] = __builtin_islessgreater(f1, f2) ? 5 : 0;
        results[idx++] = __builtin_isunordered(f1, f2) ? 6 : 0;
        
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 11 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 12 : 0;
        
        // Nested ternary with built-ins
        results[idx++] = __builtin_isunordered(f1, f2) ? 
                        (__builtin_isgreater(d1, d2) ? 13 : 14) : 
                        (__builtin_isless(d1, d2) ? 15 : 16);
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    v4sf vf1, vf2, vcmp;
    v2df vd1, vd2;
    int results[16];
    int idx = 0;
    
    // Load vector values
    for (int i = 0; i < 4; i++) {
        vf1[i] = farr[i];
        vf2[i] = farr[i+4];
        vd1[i/2] = darr[i];
        vd2[i/2] = darr[i+4];
    }
    
    // Vector comparisons - these may generate packed comparison RTL
    vcmp = vf1 < vf2;
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp[i] ? 1 : 0;
    }
    
    vcmp = vf1 > vf2;
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp[i] ? 2 : 0;
    }
    
    vcmp = vf1 <= vf2;
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp[i] ? 3 : 0;
    }
    
    vcmp = vf1 >= vf2;
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp[i] ? 4 : 0;
    }
    
    // Reduce vector to scalar mask
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        if (vcmp[i]) mask |= (1 << i);
    }
    results[idx++] = mask;
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    uint8_t results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = darr[i];
        double d2 = darr[i+1];
        float f1 = farr[i];
        float f2 = farr[i+1];
        
        // Inline assembly that uses condition code names
        uint8_t byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
        
        // Test various condition codes
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "seta %0"
            : "=r"(byte1) : "x"(d1), "x"(d2) : "cc");
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setb %0"
            : "=r"(byte2) : "x"(f1), "x"(f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0"
            : "=r"(byte3) : "x"(d1), "x"(d2) : "cc");
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setne %0"
            : "=r"(byte4) : "x"(f1), "x"(f2) : "cc");
        
        // Unordered/ordered tests - these should trigger the uncovered code
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %0"
            : "=r"(byte5) : "x"(d1), "x"(d2) : "cc");
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setnp %0"
            : "=r"(byte6) : "x"(f1), "x"(f2) : "cc");
        
        // More complex condition codes
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setae %0"
            : "=r"(byte7) : "x"(d1), "x"(d2) : "cc");
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setbe %0"
            : "=r"(byte8) : "x"(f1), "x"(f2) : "cc");
        
        results[idx++] = byte1;
        results[idx++] = byte2;
        results[idx++] = byte3;
        results[idx++] = byte4;
        results[idx++] = byte5;
        results[idx++] = byte6;
        results[idx++] = byte7;
        results[idx++] = byte8;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

// Complex control flow test with goto and nested loops
int test_control_flow(void) {
    int result = 0;
    int counter = 0;
    
    for (int i = 0; i < 16; i++) {
        double d = darr[i];
        float f = farr[i];
        
        // Complex nested if-else with goto
        if (isnan(d)) {
            if (isinf(f)) {
                result += 1;
                goto label1;
            } else {
                result += 2;
            }
        } else if (d > 0.0) {
            if (f < 0.0f) {
                result += 3;
                continue;
            }
        }
        
        label1:
        // Switch statement with floating comparisons in cases
        switch (counter % 4) {
            case 0:
                result += (d == darr[0]) ? 4 : 0;
                break;
            case 1:
                result += (f != farr[1]) ? 5 : 0;
                break;
            case 2:
                result += (d <= darr[2]) ? 6 : 0;
                break;
            case 3:
                result += (f >= farr[3]) ? 7 : 0;
                break;
        }
        
        counter++;
        
        // Loop with early break based on floating comparison
        for (int j = 0; j < 4; j++) {
            if (isunordered(d, darr[j])) {
                result += 8;
                break;
            }
            if (__builtin_isgreater(f, farr[j])) {
                result += 9;
                continue;
            }
        }
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    // Initialize arrays with more NaN values
    farr[3] = NAN;
    farr[5] = INFINITY;
    farr[6] = -INFINITY;
    darr[2] = NAN;
    darr[4] = INFINITY;
    darr[5] = -INFINITY;
    
    // Run all tests
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    checksum += test_control_flow();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
