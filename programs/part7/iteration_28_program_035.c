#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float fvals[16] = {
    1.0f, 2.0f, 3.0f, NAN,
    -1.0f, INFINITY, -INFINITY, 0.0f,
    1.5f, 2.5f, 3.5f, 4.5f,
    __builtin_nanf(""), __builtin_inff(), -__builtin_inff(), 7.0f
};

double dvals[16] = {
    1.0, 2.0, 3.0, NAN,
    -1.0, INFINITY, -INFINITY, 0.0,
    1.5, 2.5, 3.5, 4.5,
    __builtin_nan(""), __builtin_inf(), -__builtin_inf(), 7.0
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32] = {0};
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[i + 8];
        double d1 = dvals[i];
        double d2 = dvals[i + 8];
        
        // UNORDERED cases (using != comparisons with NaN)
        results[idx++] = (f1 != f1) ? 1 : 0;  // UNORDERED when f1 is NaN
        results[idx++] = (d1 != d1) ? 2 : 0;  // UNORDERED when d1 is NaN
        
        // ORDERED cases
        results[idx++] = (f1 == f1) ? 3 : 0;  // ORDERED when f1 is not NaN
        results[idx++] = (d1 == d1) ? 4 : 0;  // ORDERED when d1 is not NaN
        
        // UNEQ cases (unordered or equal)
        results[idx++] = !(f1 < f2) && !(f1 > f2) ? 5 : 0;
        results[idx++] = !(d1 < d2) && !(d1 > d2) ? 6 : 0;
        
        // UNGE cases (unordered or greater or equal)
        results[idx++] = !(f1 < f2) ? 7 : 0;
        results[idx++] = !(d1 < d2) ? 8 : 0;
        
        // UNGT cases (unordered or greater)
        results[idx++] = !(f1 <= f2) ? 9 : 0;
        results[idx++] = !(d1 <= d2) ? 10 : 0;
        
        // UNLE cases (unordered or less or equal)
        results[idx++] = !(f1 > f2) ? 11 : 0;
        results[idx++] = !(d1 > d2) ? 12 : 0;
        
        // UNLT cases (unordered or less)
        results[idx++] = !(f1 >= f2) ? 13 : 0;
        results[idx++] = !(d1 >= f2) ? 14 : 0;  // Mixed type
        
        // LTGT cases (less or greater, but not equal and not unordered)
        results[idx++] = (f1 < f2 || f1 > f2) ? 15 : 0;
        results[idx++] = (d1 < d2 || d1 > d2) ? 16 : 0;
    }
    
    // Complex control flow with nested if-else and switch
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        switch (i % 8) {
            case 0:  // UNORDERED
                if (results[i] != 0) {
                    sum += results[i];
                    if (sum > 100) goto done;
                } else {
                    continue;
                }
                break;
            case 1:  // ORDERED
                sum += results[i] * 2;
                break;
            case 2:  // UNEQ
                for (int j = 0; j < 3; j++) {
                    sum += results[i] + j;
                }
                break;
            case 3:  // UNGE
                sum += results[i] > 5 ? results[i] : 0;
                break;
            case 4:  // UNGT
                while (sum < 50) {
                    sum += results[i] / 2;
                }
                break;
            case 5:  // UNLE
                do {
                    sum -= results[i];
                } while (sum > 100);
                break;
            case 6:  // UNLT
                sum += results[i] < 10 ? results[i] * 3 : results[i];
                break;
            case 7:  // LTGT
                sum += results[i] | 0x1;
                break;
            default:
                break;
        }
    }
    
done:
    return sum;
}

// Test builtin unordered comparison functions
int test_builtins(void) {
    int results = 0;
    
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[15 - i];
        double d1 = dvals[i];
        double d2 = dvals[15 - i];
        
        // __builtin_isunordered - directly maps to UNORDERED
        results += __builtin_isunordered(f1, f2) ? 1 : 0;
        results += __builtin_isunordered(d1, d2) ? 2 : 0;
        
        // __builtin_isgreater - UNGT
        results += __builtin_isgreater(f1, f2) ? 4 : 0;
        results += __builtin_isgreater(d1, d2) ? 8 : 0;
        
        // __builtin_isless - UNLT
        results += __builtin_isless(f1, f2) ? 16 : 0;
        results += __builtin_isless(d1, d2) ? 32 : 0;
        
        // __builtin_isgreaterequal - UNGE
        results += __builtin_isgreaterequal(f1, f2) ? 64 : 0;
        results += __builtin_isgreaterequal(d1, d2) ? 128 : 0;
        
        // __builtin_islessequal - UNLE
        results += __builtin_islessequal(f1, f2) ? 256 : 0;
        results += __builtin_islessequal(d1, d2) ? 512 : 0;
        
        // Using classification functions
        int c1 = fpclassify(f1);
        int c2 = fpclassify(d2);
        results += (c1 == FP_NAN || c2 == FP_NAN) ? 1024 : 0;
        results += (c1 == FP_INFINITE || c2 == FP_INFINITE) ? 2048 : 0;
        
        // Conditional moves using ternary operator (should generate CMOV)
        int cmov_result = 0;
        cmov_result = __builtin_isunordered(f1, f2) ? 1 : cmov_result;
        cmov_result = __builtin_isgreater(f1, f2) ? 2 : cmov_result;
        cmov_result = __builtin_isless(f1, f2) ? 3 : cmov_result;
        results += cmov_result;
    }
    
    return results;
}

// Test vector/SIMD comparisons
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, 4.0f};
    v4sf vf2 = {4.0f, 1.0f, 3.0f, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 2.0};
    
    int mask = 0;
    
    // Vector comparisons generate packed comparison RTL
    v4sf cmp_result_f = vf1 < vf2;  // Should expand to scalar condition checks
    v2df cmp_result_d = vd1 > vd2;
    
    // Reduce vector to scalar mask
    for (int i = 0; i < 4; i++) {
        mask |= (cmp_result_f[i] != 0.0f) ? (1 << i) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        mask |= (cmp_result_d[i] != 0.0) ? (1 << (i + 4)) : 0;
    }
    
    // More vector operations
    v4sf vf3 = vf1 + vf2;
    v4sf vf4 = vf1 * vf2;
    v4sf cmp_result_f2 = vf3 <= vf4;
    v4sf cmp_result_f3 = vf3 >= vf4;
    v4sf cmp_result_f4 = vf3 == vf4;
    v4sf cmp_result_f5 = vf3 != vf4;
    
    // Check all comparison results
    for (int i = 0; i < 4; i++) {
        mask += cmp_result_f2[i] ? 1 : 0;
        mask += cmp_result_f3[i] ? 2 : 0;
        mask += cmp_result_f4[i] ? 4 : 0;
        mask += cmp_result_f5[i] ? 8 : 0;
    }
    
    return mask;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    unsigned char results[16] = {0};
    int sum = 0;
    
    for (int i = 0; i < 8; i++) {
        double d1 = dvals[i];
        double d2 = dvals[i + 8];
        
        // Test various condition codes in inline assembly
        unsigned char cc1, cc2, cc3, cc4, cc5, cc6, cc7, cc8;
        
        // Compare and set based on condition codes
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "seta %0\n\t"      // UNGT
            : "=r"(cc1)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0\n\t"      // UNLT
            : "=r"(cc2)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"      // UNEQ
            : "=r"(cc3)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %0\n\t"      // UNORDERED
            : "=r"(cc4)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %0\n\t"     // UNLE
            : "=r"(cc5)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setae %0\n\t"     // UNGE
            : "=r"(cc6)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %0\n\t"     // LTGT (not equal and ordered)
            : "=r"(cc7)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0\n\t"     // ORDERED
            : "=r"(cc8)
            : "x"(d1), "x"(d2)
            : "cc"
        );
        
        results[i * 2] = cc1 + cc2 + cc3 + cc4;
        results[i * 2 + 1] = cc5 + cc6 + cc7 + cc8;
    }
    
    for (int i = 0; i < 16; i++) {
        sum += results[i];
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Run all tests
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
