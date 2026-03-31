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
                result += (f1 <= f2) ? 2 : 0;  // Should generate ordered comparisons
            }
            
            // UNEQ (unordered or equal)
            int uneq_test = (f1 == f2) ? 4 : 0;
            if (isnan(f1) || isnan(f2)) {
                uneq_test = 4;  // Force UNEQ path
            }
            result += uneq_test;
            
            // UNGE (not less than) - using ternary to force CMOV
            int unge_result = (d1 >= d2) ? 8 : 0;
            result += unge_result;
            
            // UNGT (not less than or equal)
            int ungt_result = (d1 > d2) ? 16 : 0;
            result += ungt_result;
            
            // UNLE (unordered or less than or equal)
            int unle_result;
            if (isnan(d1) || isnan(d2)) {
                unle_result = 32;
            } else {
                unle_result = (d1 <= d2) ? 32 : 0;
            }
            result += unle_result;
            
            // UNLT (unordered or less than)
            int unlt_result;
            if (isnan(d1) || isnan(d2)) {
                unlt_result = 64;
            } else {
                unlt_result = (d1 < d2) ? 64 : 0;
            }
            result += unlt_result;
            
            // LTGT (less than or greater than, but not equal and not unordered)
            int ltgt_result;
            if (!isnan(d1) && !isnan(d2) && d1 != d2) {
                ltgt_result = 128;
            } else {
                ltgt_result = 0;
            }
            result += ltgt_result;
        }
    }
    
    return result;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // __builtin_isunordered - directly maps to UNORDERED
            result += __builtin_isunordered(f1, f2) ? 1 : 0;
            
            // __builtin_isless - should generate ordered comparison
            result += __builtin_isless(f1, f2) ? 2 : 0;
            
            // __builtin_isgreater - should generate ordered comparison
            result += __builtin_isgreater(f1, f2) ? 4 : 0;
            
            // __builtin_islessequal
            result += __builtin_islessequal(d1, d2) ? 8 : 0;
            
            // __builtin_isgreaterequal
            result += __builtin_isgreaterequal(d1, d2) ? 16 : 0;
            
            // __builtin_islessgreater - maps to LTGT
            result += __builtin_islessgreater(d1, d2) ? 32 : 0;
        }
    }
    
    return result;
}

// Test vector comparisons
int test_vector(void) {
    int result = 0;
    
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    // Vector comparisons generate packed RTL
    v4sf vcmp_lt = vf1 < vf2;      // Should involve UNLT/UNORDERED logic
    v4sf vcmp_gt = vf1 > vf2;      // Should involve UNGT/UNORDERED logic
    v4sf vcmp_le = vf1 <= vf2;     // Should involve UNLE/UNORDERED logic
    v4sf vcmp_ge = vf1 >= vf2;     // Should involve UNGE/UNORDERED logic
    v4sf vcmp_eq = vf1 == vf2;     // Should involve UNEQ/UNORDERED logic
    v4sf vcmp_neq = vf1 != vf2;    // Should involve LTGT/UNORDERED logic
    
    // Reduce vector to scalar result
    for (int i = 0; i < 4; i++) {
        result += ((int*)&vcmp_lt)[i] ? (1 << i) : 0;
        result += ((int*)&vcmp_gt)[i] ? (2 << i) : 0;
        result += ((int*)&vcmp_le)[i] ? (4 << i) : 0;
        result += ((int*)&vcmp_ge)[i] ? (8 << i) : 0;
        result += ((int*)&vcmp_eq)[i] ? (16 << i) : 0;
        result += ((int*)&vcmp_neq)[i] ? (32 << i) : 0;
    }
    
    // Double vector comparisons
    v2df vcmp_dlt = vd1 < vd2;
    v2df vcmp_dgt = vd1 > vd2;
    
    result += ((long long*)&vcmp_dlt)[0] ? 64 : 0;
    result += ((long long*)&vcmp_dlt)[1] ? 128 : 0;
    result += ((long long*)&vcmp_dgt)[0] ? 256 : 0;
    result += ((long long*)&vcmp_dgt)[1] ? 512 : 0;
    
    return result;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    int result = 0;
    unsigned char byte_result;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double a = dvals[i];
            double b = dvals[j];
            
            // Test various condition codes via inline assembly
            // These force the assembly printer to resolve condition code names
            
            // UNORDERED
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setp %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result;
            
            // ORDERED
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnp %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 2;
            
            // UNEQ (unordered or equal)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "sete %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 4;
            
            // UNGE (not less than)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnb %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 8;
            
            // UNGT (not less than or equal)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnbe %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 16;
            
            // UNLE (unordered or less than or equal)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setbe %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 32;
            
            // UNLT (unordered or less than)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setb %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 64;
            
            // LTGT (not equal and ordered)
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setne %0"
                : "=r"(byte_result)
                : "x"(a), "x"(b)
                : "cc"
            );
            result += byte_result * 128;
        }
    }
    
    return result;
}

// Complex control flow to force condition code materialization
int test_complex_control_flow(void) {
    int result = 0;
    int counter = 0;
    
    // Nested loops with complex conditions
    for (int i = 0; i < 8; i++) {
        float f = fvals[i];
        
        switch (fpclassify(f)) {
            case FP_NAN:
                for (int j = 0; j < 8; j++) {
                    double d = dvals[j];
                    // Complex condition with UNORDERED
                    if (isnan(f) && isnan(d)) {
                        result += 1;
                        goto label1;
                    } else if (isnan(f) || isnan(d)) {
                        result += 2;
                        continue;
                    } else if (f < d) {  // Should generate UNLT
                        result += 4;
                        break;
                    }
                    label1:
                    counter++;
                }
                break;
                
            case FP_INFINITE:
                for (int j = 0; j < 8; j++) {
                    double d = dvals[j];
                    // Mix of ordered and unordered comparisons
                    int temp = (f > d) ? 8 : 0;      // UNGT
                    temp += (f <= d) ? 16 : 0;       // UNLE
                    temp += (f == d) ? 32 : 0;       // UNEQ
                    temp += (f != d) ? 64 : 0;       // LTGT
                    result += temp;
                    
                    if (temp > 100) {
                        goto early_exit;
                    }
                }
                break;
                
            default:
                // Use ternary operators to force CMOV generation
                for (int j = 0; j < 8; j++) {
                    double d = dvals[j];
                    int cmp_result = (f >= d) ? 128 : 0;      // UNGE
                    cmp_result += (f < d) ? 256 : 0;          // UNLT
                    cmp_result += (f > d) ? 512 : 0;          // UNGT
                    cmp_result += (f <= d) ? 1024 : 0;        // UNLE
                    
                    // Force materialization with volatile
                    volatile int vol_result = cmp_result;
                    result += vol_result;
                }
                break;
        }
        
        if (counter > 20) {
            break;
        }
    }
    
    early_exit:
    return result;
}

int main(void) {
    int checksum = 0;
    
    // Run all tests
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    checksum += test_complex_control_flow();
    
    // Print result to prevent dead code elimination
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
