#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

static int checksum = 0;

// Test scalar comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Generate UNORDERED cases (NaN comparisons)
    result += (f1 < f2) ? 1 : 0;      // May generate UNLT
    result += (f1 > f2) ? 2 : 0;      // May generate UNGT
    result += (f1 <= f2) ? 4 : 0;     // May generate UNLE
    result += (f1 >= f2) ? 8 : 0;     // May generate UNGE
    result += (f1 == f2) ? 16 : 0;    // May generate UNEQ
    result += (f1 != f2) ? 32 : 0;    // May generate LTGT (une)
    
    // Double comparisons for different code paths
    result += (d1 < d2) ? 64 : 0;
    result += (d1 > d2) ? 128 : 0;
    result += (d1 <= d2) ? 256 : 0;
    result += (d1 >= d2) ? 512 : 0;
    result += (d1 == d2) ? 1024 : 0;
    result += (d1 != d2) ? 2048 : 0;
    
    // Complex control flow with condition codes
    for (int i = 0; i < 3; i++) {
        if (f1 < f2) {
            result += 4096;
            if (d1 > d2) {
                result += 8192;
                continue;
            }
        } else if (f1 == f2) {
            result += 16384;
            break;
        } else {
            result += 32768;
            goto done;
        }
        result += 65536;
    }
done:
    checksum ^= result;
}

// Test builtin unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Direct builtins that map to condition codes
    result += __builtin_isunordered(f1, f2) ? 1 : 0;    // UNORDERED
    result += __builtin_isgreater(f1, f2) ? 2 : 0;      // ORDERED + GT
    result += __builtin_isless(f1, f2) ? 4 : 0;         // ORDERED + LT
    result += __builtin_islessequal(f1, f2) ? 8 : 0;    // ORDERED + LE
    result += __builtin_isgreaterequal(f1, f2) ? 16 : 0; // ORDERED + GE
    
    // Double versions
    result += __builtin_isunordered(d1, d2) ? 32 : 0;
    result += __builtin_isgreater(d1, d2) ? 64 : 0;
    result += __builtin_isless(d1, d2) ? 128 : 0;
    
    // Mixed in switch statement
    switch (fpclassify(f1)) {
        case FP_NAN:
            result += __builtin_isunordered(f1, f2) ? 256 : 512;
            break;
        case FP_INFINITE:
            result += __builtin_isgreater(f1, f2) ? 1024 : 2048;
            break;
        case FP_ZERO:
            result += __builtin_isless(f1, f2) ? 4096 : 8192;
            break;
        default:
            result += __builtin_islessequal(f1, f2) ? 16384 : 32768;
    }
    
    checksum ^= result;
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2) {
    v4sf vf1 = {f1, f2, f1, f2};
    v4sf vf2 = {f2, f1, f2, f1};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    int result = 0;
    
    // Vector comparisons that may generate condition codes
    v4sf cmp_f = vf1 < vf2;
    v2df cmp_d = vd1 > vd2;
    
    // Reduce to scalar
    float* fp = (float*)&cmp_f;
    double* dp = (double*)&cmp_d;
    
    for (int i = 0; i < 4; i++) {
        if (fp[i] != 0.0f) result += 1 << i;
    }
    for (int i = 0; i < 2; i++) {
        if (dp[i] != 0.0) result += 16 << i;
    }
    
    // Nested loops with vector results
    for (int i = 0; i < 2; i++) {
        v4sf tmp = vf1 + vf2;
        v4sf cmp = tmp < vf1;
        float* cfp = (float*)&cmp;
        
        for (int j = 0; j < 4; j++) {
            if (cfp[j] != 0.0f) {
                result += 64;
                if (j % 2 == 0) continue;
                result += 128;
            }
        }
    }
    
    checksum ^= result;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2) {
    unsigned char byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
    int result = 0;
    
    // Test various condition codes in inline asm
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(byte1)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte2)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    // Test unordered/ordered conditions
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte3)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte4)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    result = byte1 | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
    
    // More complex asm with different constraints
    int val1, val2;
    __asm__ volatile (
        "ucomiss %2, %3\n\t"
        "cmova %4, %0\n\t"
        "cmovb %5, %1"
        : "=r"(val1), "=r"(val2)
        : "x"(f1), "x"(f2), "r"(100), "r"(200)
        : "cc"
    );
    
    result ^= val1 ^ val2;
    checksum ^= result;
}

// Main test driver
int main() {
    // Initialize test values including specials
    float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f};
    double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0};
    
    int num_f = sizeof(fvals) / sizeof(fvals[0]);
    int num_d = sizeof(dvals) / sizeof(dvals[0]);
    
    // Run tests with various combinations
    for (int i = 0; i < num_f; i++) {
        for (int j = 0; j < num_f; j++) {
            if (i == j) continue;
            for (int k = 0; k < num_d; k++) {
                for (int l = 0; l < num_d; l++) {
                    if (k == l) continue;
                    
                    test_scalar_cmps(fvals[i], fvals[j], dvals[k], dvals[l]);
                    test_builtins(fvals[i], fvals[j], dvals[k], dvals[l]);
                    test_vector(fvals[i], fvals[j], dvals[k], dvals[l]);
                    test_asm(fvals[i], fvals[j], dvals[k], dvals[l]);
                    
                    // Early exit after enough combinations
                    if ((i * j * k * l) > 100) goto done_loops;
                }
            }
        }
    }
done_loops:
    
    // Additional targeted tests
    float f_nan = NAN;
    float f_inf = INFINITY;
    double d_nan = NAN;
    double d_inf = INFINITY;
    
    // Test all condition code cases specifically
    test_scalar_cmps(f_nan, f_inf, d_nan, d_inf);
    test_scalar_cmps(f_inf, f_nan, d_inf, d_nan);
    test_scalar_cmps(1.0f, f_nan, 1.0, d_nan);
    test_scalar_cmps(f_nan, 1.0f, d_nan, 1.0);
    
    // Print checksum to prevent optimization
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
