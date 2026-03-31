#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f};
double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Use ternary operators to force CMOV/SET generation
            results[idx++] = (f1 < f2) ? 1 : 0;      // UNLT/UNORDERED
            results[idx++] = (f1 > f2) ? 2 : 0;      // UNGT/UNORDERED  
            results[idx++] = (f1 <= f2) ? 3 : 0;     // UNLE/UNORDERED
            results[idx++] = (f1 >= f2) ? 4 : 0;     // UNGE/UNORDERED
            results[idx++] = (f1 == f2) ? 5 : 0;     // UNEQ/UNORDERED
            results[idx++] = (f1 != f2) ? 6 : 0;     // LTGT/UNORDERED
            
            results[idx++] = (d1 < d2) ? 7 : 0;
            results[idx++] = (d1 > d2) ? 8 : 0;
            results[idx++] = (d1 <= d2) ? 9 : 0;
            results[idx++] = (d1 >= d2) ? 10 : 0;
            results[idx++] = (d1 == d2) ? 11 : 0;
            results[idx++] = (d1 != d2) ? 12 : 0;
        }
    }
    
    // Complex control flow with nested conditionals
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        switch (results[i] % 7) {
            case 0:
                if (fvals[i % 8] < dvals[i % 8]) {
                    sum += 1;
                    goto label1;
                } else if (fvals[i % 8] > dvals[i % 8]) {
                    sum += 2;
                    continue;
                }
                break;
            case 1:
                if (fvals[i % 8] <= dvals[i % 8]) {
                    sum += 3;
                    break;
                }
                // fall through
            case 2:
                sum += (fvals[i % 8] >= dvals[i % 8]) ? 4 : 5;
                continue;
            default:
                sum += 6;
        }
        label1:
        sum += 7;
    }
    
    return sum;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            // Direct builtin calls that map to condition codes
            results[idx++] = __builtin_isgreater(f1, f2) ? 1 : 0;      // UNGT
            results[idx++] = __builtin_isless(f1, f2) ? 2 : 0;         // UNLT
            results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0; // UNGE
            results[idx++] = __builtin_islessequal(f1, f2) ? 4 : 0;    // UNLE
            results[idx++] = __builtin_isunordered(f1, f2) ? 5 : 0;    // UNORDERED
            results[idx++] = !__builtin_isunordered(f1, f2) ? 6 : 0;   // ORDERED
            
            results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
            results[idx++] = __builtin_isless(d1, d2) ? 8 : 0;
            results[idx++] = __builtin_isgreaterequal(d1, d2) ? 9 : 0;
            results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
            results[idx++] = __builtin_isunordered(d1, d2) ? 11 : 0;
            results[idx++] = !__builtin_isunordered(d1, d2) ? 12 : 0;
        }
    }
    
    // Use classification functions
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        switch (fpclassify(fvals[i])) {
            case FP_NAN:
                sum += __builtin_isunordered(fvals[i], fvals[(i+1)%8]) ? 1 : 0;
                break;
            case FP_INFINITE:
                sum += __builtin_isgreater(fvals[i], 0.0f) ? 2 : 0;
                break;
            case FP_ZERO:
                sum += (fvals[i] == 0.0f) ? 3 : 0;  // UNEQ
                break;
            case FP_SUBNORMAL:
                sum += 4;
                break;
            case FP_NORMAL:
                sum += __builtin_isless(fvals[i], 1.0f) ? 5 : 0;
                break;
        }
        
        if (isnan(dvals[i])) {
            sum += __builtin_isunordered(dvals[i], dvals[(i+1)%8]) ? 6 : 0;
        } else if (isinf(dvals[i])) {
            sum += __builtin_isless(dvals[i], 0.0) ? 7 : 0;
        }
    }
    
    return sum + idx;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int results[16];
    int idx = 0;
    
    // Vector comparisons generate packed RTL
    v4sf cmp1 = vf1 < vf2;    // UNLT
    v4sf cmp2 = vf1 > vf2;    // UNGT
    v4sf cmp3 = vf1 <= vf2;   // UNLE
    v4sf cmp4 = vf1 >= vf2;   // UNGE
    v4sf cmp5 = vf1 == vf2;   // UNEQ
    v4sf cmp6 = vf1 != vf2;   // LTGT
    
    v2df cmp7 = vd1 < vd2;
    v2df cmp8 = vd1 > vd2;
    v2df cmp9 = vd1 <= vd2;
    v2df cmp10 = vd1 >= vd2;
    v2df cmp11 = vd1 == vd2;
    v2df cmp12 = vd1 != vd2;
    
    // Reduce vector to scalar mask
    uint32_t mask1, mask2;
    memcpy(&mask1, &cmp1, sizeof(mask1));
    memcpy(&mask2, &cmp2, sizeof(mask2));
    
    results[idx++] = (mask1 & 1) ? 1 : 0;
    results[idx++] = (mask2 & 2) ? 2 : 0;
    
    // Complex control flow with vector results
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        float f = ((float*)&cmp1)[i];
        if (f != 0.0f) {
            switch (i) {
                case 0:
                    sum += ((float*)&cmp3)[i] ? 1 : 0;
                    goto vector_label;
                case 1:
                    sum += ((float*)&cmp4)[i] ? 2 : 0;
                    continue;
                case 2:
                    sum += ((float*)&cmp5)[i] ? 3 : 0;
                    break;
                default:
                    sum += ((float*)&cmp6)[i] ? 4 : 0;
            }
            vector_label:
            sum += 5;
        }
    }
    
    return sum + idx;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i += 2) {
        double a = dvals[i];
        double b = dvals[i + 1];
        float c = fvals[i];
        float d = fvals[i + 1];
        
        // Inline assembly that uses condition code names
        // These force the assembly printer to resolve symbolic condition codes
        
        // Test UNORDERED
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setp %0"
            : "=g" (results[idx++])
            : "t" (a), "u" (b)
            : "cc"
        );
        
        // Test ORDERED  
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnp %0"
            : "=g" (results[idx++])
            : "t" (c), "u" (d)
            : "cc"
        );
        
        // Test UNEQ (equal or unordered)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "sete %0"
            : "=g" (results[idx++])
            : "t" (a), "u" (b)
            : "cc"
        );
        
        // Test UNGE (not less than)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnb %0"
            : "=g" (results[idx++])
            : "t" (c), "u" (d)
            : "cc"
        );
        
        // Test UNGT (not less or equal)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnbe %0"
            : "=g" (results[idx++])
            : "t" (a), "u" (b)
            : "cc"
        );
        
        // Test UNLE (unordered or less or equal)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setbe %0"
            : "=g" (results[idx++])
            : "t" (c), "u" (d)
            : "cc"
        );
        
        // Test UNLT (unordered or less than)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setb %0"
            : "=g" (results[idx++])
            : "t" (a), "u" (b)
            : "cc"
        );
        
        // Test LTGT (not equal and ordered)
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setne %0"
            : "=g" (results[idx++])
            : "t" (c), "u" (d)
            : "cc"
        );
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Call all test functions and accumulate results
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
