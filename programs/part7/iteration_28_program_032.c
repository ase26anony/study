#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // UNORDERED cases (using isnan checks)
            if (isnan(f1) || isnan(f2)) {
                result += (f1 < f2) ? 0 : 1;  // Should generate unordered comparisons
            }
            
            // ORDERED cases
            if (!isnan(f1) && !isnan(f2)) {
                result += (f1 <= f2) ? 1 : 0;  // Should generate ordered comparisons
            }
            
            // UNEQ (unordered or equal)
            int uneq_test = (d1 == d2) || (isnan(d1) || isnan(d2));
            result += uneq_test ? 2 : 0;
            
            // UNGE (not less than) - using ternary to force CMOV
            int unge_test = !(d1 < d2);
            result += unge_test ? 3 : 0;
            
            // UNGT (not less or equal)
            int ungt_test = !(d1 <= d2);
            result += ungt_test ? 4 : 0;
            
            // UNLE (unordered or less or equal)
            int unle_test = (d1 <= d2) || (isnan(d1) || isnan(d2));
            result += unle_test ? 5 : 0;
            
            // UNLT (unordered or less than)
            int unlt_test = (d1 < d2) || (isnan(d1) || isnan(d2));
            result += unlt_test ? 6 : 0;
            
            // LTGT (less than or greater than, but not equal and not unordered)
            int ltgt_test = (d1 < d2) || (d1 > d2);
            result += ltgt_test ? 7 : 0;
            
            // Complex control flow with nested if-else
            switch (i % 4) {
                case 0:
                    if (f1 < f2) {
                        result += 11;
                        if (d1 > d2) {
                            result += 13;
                            goto label1;
                        }
                    } else if (f1 == f2) {
                        result += 17;
                    }
                    break;
                case 1:
                    if (f1 >= f2) {
                        result += 19;
                        continue;
                    }
                    result += 23;
                    break;
                label1:
                case 2:
                    result += 29;
                    break;
                default:
                    if (f1 != f2) {
                        result += 31;
                    }
                    break;
            }
        }
    }
    
    return result;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = dvals[i];
        double d2 = dvals[(i + 1) % 8];
        float f1 = fvals[i];
        float f2 = fvals[(i + 3) % 8];
        
        // __builtin_isunordered - directly maps to UNORDERED
        result += __builtin_isunordered(d1, d2) ? 1 : 0;
        
        // __builtin_isgreater - generates GT with ordered check
        result += __builtin_isgreater(f1, f2) ? 2 : 0;
        
        // __builtin_isless - generates LT with ordered check
        result += __builtin_isless(f1, f2) ? 3 : 0;
        
        // __builtin_isgreaterequal
        result += __builtin_isgreaterequal(d1, d2) ? 5 : 0;
        
        // __builtin_islessequal
        result += __builtin_islessequal(d1, d2) ? 7 : 0;
        
        // __builtin_islessgreater - should map to LTGT
        result += __builtin_islessgreater(f1, f2) ? 11 : 0;
        
        // Mixed types in ternary with builtins
        int cmp_result = __builtin_isunordered(d1, d2) ? 100 : 
                        (__builtin_isgreater(d1, d2) ? 200 : 
                        (__builtin_isless(d1, d2) ? 300 : 400));
        result += cmp_result;
    }
    
    return result;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    int result = 0;
    
    v4sf vec1 = {1.0f, 2.0f, NAN, 4.0f};
    v4sf vec2 = {2.0f, 1.0f, 3.0f, NAN};
    v2df dvec1 = {1.0, NAN};
    v2df dvec2 = {NAN, 2.0};
    
    // Vector comparisons generate packed comparison RTL
    v4sf cmp_result = vec1 < vec2;
    v4sf cmp_result2 = vec1 > vec2;
    v4sf cmp_result3 = vec1 <= vec2;
    v4sf cmp_result4 = vec1 >= vec2;
    v4sf cmp_result5 = vec1 == vec2;
    v4sf cmp_result6 = vec1 != vec2;
    
    // Reduce vector to scalar mask
    for (int i = 0; i < 4; i++) {
        result += cmp_result[i] ? (1 << i) : 0;
        result += cmp_result2[i] ? (2 << i) : 0;
        result += cmp_result3[i] ? (3 << i) : 0;
        result += cmp_result4[i] ? (4 << i) : 0;
        result += cmp_result5[i] ? (5 << i) : 0;
        result += cmp_result6[i] ? (6 << i) : 0;
    }
    
    // Double vector comparisons
    v2df dcmp = dvec1 < dvec2;
    v2df dcmp2 = dvec1 > dvec2;
    
    result += dcmp[0] ? 1000 : 0;
    result += dcmp[1] ? 2000 : 0;
    result += dcmp2[0] ? 3000 : 0;
    result += dcmp2[1] ? 4000 : 0;
    
    return result;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    int result = 0;
    uint8_t byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    
    for (int i = 0; i < 8; i++) {
        double d1 = dvals[i];
        double d2 = dvals[(i + 2) % 8];
        
        // Compare and set byte based on condition
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "seta %0\n\t"
            : "=r" (byte1)
            : "x" (d1), "x" (d2)
        );
        
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "setb %0\n\t"
            : "=r" (byte2)
            : "x" (d1), "x" (d2)
        );
        
        // Test unordered condition codes
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %0\n\t"      // UNORDERED (parity flag)
            : "=r" (byte3)
            : "x" (d1), "x" (d2)
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0\n\t"     // ORDERED (no parity)
            : "=r" (byte4)
            : "x" (d1), "x" (d2)
        );
        
        // More condition codes
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"      // EQUAL or UNORDERED (ZF=1)
            : "=r" (byte5)
            : "x" (d1), "x" (d2)
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %0\n\t"     // NOT EQUAL and ORDERED (ZF=0)
            : "=r" (byte6)
            : "x" (d1), "x" (d2)
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %0\n\t"     // BELOW OR EQUAL (CF=1 or ZF=1)
            : "=r" (byte7)
            : "x" (d1), "x" (d2)
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setae %0\n\t"     // ABOVE OR EQUAL (CF=0)
            : "=r" (byte8)
            : "x" (d1), "x" (d2)
        );
        
        result += byte1 + byte2 + byte3 + byte4 + byte5 + byte6 + byte7 + byte8;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    // Initialize special values if not already present
    fvals[2] = NAN;
    fvals[3] = INFINITY;
    fvals[4] = -INFINITY;
    dvals[2] = NAN;
    dvals[3] = INFINITY;
    dvals[4] = -INFINITY;
    
    // Run all tests
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
