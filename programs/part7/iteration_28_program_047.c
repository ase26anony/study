#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float farr[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f};
double darr[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int result = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = farr[i];
            float f2 = farr[j];
            double d1 = darr[i];
            double d2 = darr[j];
            
            // UNORDERED cases (using != and == with NaN)
            result += (f1 != f2) ? 1 : 0;
            result += (d1 == d2) ? 2 : 0;
            
            // ORDERED cases
            result += (!isnan(f1) && !isnan(f2)) ? 4 : 0;
            result += (!isnan(d1) && !isnan(d2)) ? 8 : 0;
            
            // UNEQ (unordered or equal)
            result += ((f1 == f2) || isnan(f1) || isnan(f2)) ? 16 : 0;
            result += ((d1 == d2) || isnan(d1) || isnan(d2)) ? 32 : 0;
            
            // UNGE (not less than)
            result += (!(f1 < f2)) ? 64 : 0;
            result += (!(d1 < d2)) ? 128 : 0;
            
            // UNGT (not less than or equal)
            result += (!(f1 <= f2)) ? 256 : 0;
            result += (!(d1 <= d2)) ? 512 : 0;
            
            // UNLE (unordered or less than or equal)
            result += ((f1 <= f2) || isnan(f1) || isnan(f2)) ? 1024 : 0;
            result += ((d1 <= d2) || isnan(d1) || isnan(d2)) ? 2048 : 0;
            
            // UNLT (unordered or less than)
            result += ((f1 < f2) || isnan(f1) || isnan(f2)) ? 4096 : 0;
            result += ((d1 < d2) || isnan(d1) || isnan(d2)) ? 8192 : 0;
            
            // LTGT (less than or greater than, but not equal and not unordered)
            result += ((f1 < f2) || (f1 > f2)) ? 16384 : 0;
            result += ((d1 < d2) || (d1 > d2)) ? 32768 : 0;
            
            // Complex nested if-else with goto
            if (isnan(f1)) {
                goto skip_float;
            } else if (isinf(f1)) {
                if (f1 > 0) {
                    result += 65536;
                } else {
                    result += 131072;
                }
            }
        skip_float:
            
            // Switch statement with floating comparisons
            switch (fpclassify(d1)) {
                case FP_NAN:
                    result += 262144;
                    break;
                case FP_INFINITE:
                    result += (d1 > 0) ? 524288 : 1048576;
                    break;
                case FP_ZERO:
                    result += 2097152;
                    // Fall through
                default:
                    if (d1 < d2) result += 4194304;
                    else if (d1 > d2) result += 8388608;
                    else result += 16777216;
                    break;
            }
        }
    }
    
    return result;
}

// Test built-in unordered comparison functions
int test_builtins(void) {
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = farr[i];
            float f2 = farr[j];
            double d1 = darr[i];
            double d2 = darr[j];
            
            // __builtin_isunordered - directly maps to UNORDERED
            result += __builtin_isunordered(f1, f2) ? 1 : 0;
            result += __builtin_isunordered(d1, d2) ? 2 : 0;
            
            // __builtin_isgreater - ordered and greater than
            result += __builtin_isgreater(f1, f2) ? 4 : 0;
            result += __builtin_isgreater(d1, d2) ? 8 : 0;
            
            // __builtin_isless - ordered and less than
            result += __builtin_isless(f1, f2) ? 16 : 0;
            result += __builtin_isless(d1, d2) ? 32 : 0;
            
            // __builtin_isgreaterequal - ordered and greater than or equal
            result += __builtin_isgreaterequal(f1, f2) ? 64 : 0;
            result += __builtin_isgreaterequal(d1, d2) ? 128 : 0;
            
            // __builtin_islessequal - ordered and less than or equal
            result += __builtin_islessequal(f1, f2) ? 256 : 0;
            result += __builtin_islessequal(d1, d2) ? 512 : 0;
            
            // __builtin_islessgreater - ordered and not equal (LTGT)
            result += __builtin_islessgreater(f1, f2) ? 1024 : 0;
            result += __builtin_islessgreater(d1, d2) ? 2048 : 0;
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
    
    // Vector comparisons generate packed comparison RTL
    v4sf cmp_f = (vf1 < vf2);
    v2df cmp_d = (vd1 > vd2);
    
    // Reduce vector to scalar mask
    int mask_f = 0;
    int mask_d = 0;
    
    for (int i = 0; i < 4; i++) {
        mask_f |= (((int*)&cmp_f)[i] != 0) << i;
    }
    
    for (int i = 0; i < 2; i++) {
        mask_d |= (((int64_t*)&cmp_d)[i] != 0) << i;
    }
    
    result = mask_f + mask_d * 16;
    
    // More complex vector operations
    v4sf vf3 = vf1 + vf2;
    v4sf cmp_f2 = (vf3 <= vf1);
    
    // Use ternary operator with vector comparison result
    v4sf select_f = (cmp_f2 != (v4sf){0}) ? vf1 : vf2;
    
    // Accumulate results
    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += ((float*)&select_f)[i];
    }
    
    result += (int)sum * 256;
    
    return result;
}

// Test inline assembly with condition codes
int test_asm(void) {
    int result = 0;
    uint8_t byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = darr[i];
        double d2 = darr[(i + 1) % 8];
        
        // Inline assembly with condition code constraints
        // These force the assembly printer to resolve condition codes
        
        // UNORDERED
        __asm__ volatile (
            "fucomi %%st(1), %%st\n\t"
            "setp %0"
            : "=r" (byte1)
            : "t" (d1), "u" (d2)
            : "cc"
        );
        
        // ORDERED
        __asm__ volatile (
            "fucomi %%st(1), %%st\n\t"
            "setnp %0"
            : "=r" (byte2)
            : "t" (d1), "u" (d2)
            : "cc"
        );
        
        // UNEQ (unordered or equal)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "sete %0\n\t"
            "fstp %%st(0)"
            : "=r" (byte3)
            : "t" (d1), "u" (d2)
            : "cc", "st"
        );
        
        // UNGE (not less than)
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setnb %0"
            : "=r" (byte4)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        result += byte1 + (byte2 << 8) + (byte3 << 16) + (byte4 << 24);
        
        // Test more condition codes with float
        float f1 = farr[i];
        float f2 = farr[(i + 2) % 8];
        
        uint8_t byte5 = 0, byte6 = 0;
        
        // UNGT (not less than or equal)
        __asm__ volatile (
            "comiss %1, %2\n\t"
            "setnbe %0"
            : "=r" (byte5)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        
        // UNLE (unordered or less than or equal)
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setbe %0"
            : "=r" (byte6)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        
        result += (byte5 << 4) + (byte6 << 12);
    }
    
    return result;
}

// Main function with control flow diversification
int main(void) {
    int checksum = 0;
    
    // Loop with complex control flow
    for (int iteration = 0; iteration < 3; iteration++) {
        switch (iteration) {
            case 0:
                checksum ^= test_scalar_cmps();
                if (checksum & 1) {
                    goto next_iteration;
                }
                break;
                
            case 1:
                checksum ^= test_builtins();
                if (checksum & 2) {
                    continue;
                }
                // Fall through
                
            case 2:
                checksum ^= test_vector();
                checksum ^= test_asm();
                break;
                
            default:
                // Unreachable
                break;
        }
        
    next_iteration:
        // Empty label for goto
        ;
    }
    
    // Final computation with conditional move
    double final_val = darr[0];
    int final_result = (final_val > 0.0) ? checksum : ~checksum;
    
    // Use ternary operator that might generate CMOV
    final_result = (isnan(final_val)) ? 0xDEADBEEF : final_result;
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
