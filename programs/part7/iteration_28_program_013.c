#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

static int checksum = 0;

// Test scalar floating-point comparisons
void test_scalar_cmps(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Generate UNORDERED (NaN comparisons)
    result += (f1 != f1) ? 1 : 0;           // UNORDERED when f1 is NaN
    result += (d1 != d1) ? 2 : 0;           // UNORDERED when d1 is NaN
    
    // Generate ORDERED
    result += (f1 == f1) ? 4 : 0;           // ORDERED when f1 is not NaN
    result += (d1 == d1) ? 8 : 0;           // ORDERED when d1 is not NaN
    
    // Generate UNEQ (unordered or equal)
    result += (f1 == f2) ? 16 : 0;          // May generate UNEQ
    result += (d1 == d2) ? 32 : 0;          // May generate UNEQ
    
    // Generate UNGE (!(a < b)) - unordered or greater or equal
    result += (f1 >= f2) ? 64 : 0;          // May generate UNGE/nlt
    result += (d1 >= d2) ? 128 : 0;         // May generate UNGE/nlt
    
    // Generate UNGT (!(a <= b)) - unordered or greater
    result += (f1 > f2) ? 256 : 0;          // May generate UNGT/nle
    result += (d1 > d2) ? 512 : 0;          // May generate UNGT/nle
    
    // Generate UNLE (unordered or less or equal)
    result += (f1 <= f2) ? 1024 : 0;        // May generate UNLE/ule
    result += (d1 <= d2) ? 2048 : 0;        // May generate UNLE/ule
    
    // Generate UNLT (unordered or less)
    result += (f1 < f2) ? 4096 : 0;         // May generate UNLT/ult
    result += (d1 < d2) ? 8192 : 0;         // May generate UNLT/ult
    
    // Generate LTGT (ordered and not equal)
    result += (f1 != f2) ? 16384 : 0;       // May generate LTGT/une
    result += (d1 != d2) ? 32768 : 0;       // May generate LTGT/une
    
    checksum += result;
}

// Test built-in unordered comparisons
void test_builtins(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // __builtin_isunordered - directly maps to UNORDERED
    result += __builtin_isunordered(f1, f2) ? 1 : 0;
    result += __builtin_isunordered(d1, d2) ? 2 : 0;
    
    // __builtin_isgreater - ordered and greater (generates UNGT/nle)
    result += __builtin_isgreater(f1, f2) ? 4 : 0;
    result += __builtin_isgreater(d1, d2) ? 8 : 0;
    
    // __builtin_isless - ordered and less (generates UNLT/ult)
    result += __builtin_isless(f1, f2) ? 16 : 0;
    result += __builtin_isless(d1, d2) ? 32 : 0;
    
    // __builtin_isgreaterequal - ordered and >= (generates UNGE/nlt)
    result += __builtin_isgreaterequal(f1, f2) ? 64 : 0;
    result += __builtin_isgreaterequal(d1, d2) ? 128 : 0;
    
    // __builtin_islessequal - ordered and <= (generates UNLE/ule)
    result += __builtin_islessequal(f1, f2) ? 256 : 0;
    result += __builtin_islessequal(d1, d2) ? 512 : 0;
    
    // __builtin_islessgreater - ordered and != (generates LTGT/une)
    result += __builtin_islessgreater(f1, f2) ? 1024 : 0;
    result += __builtin_islessgreater(d1, d2) ? 2048 : 0;
    
    checksum += result;
}

// Test vector comparisons
void test_vector(float f1, float f2, double d1, double d2) {
    v4sf vf1 = {f1, f2, f1, f2};
    v4sf vf2 = {f2, f1, f2, f1};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d2, d1};
    
    // Vector comparisons generate packed RTL that may expand to condition codes
    v4sf cmp_result_f = vf1 < vf2;    // May generate UNLT/ult
    v4sf cmp_result_f2 = vf1 > vf2;   // May generate UNGT/nle
    v4sf cmp_result_f3 = vf1 <= vf2;  // May generate UNLE/ule
    v4sf cmp_result_f4 = vf1 >= vf2;  // May generate UNGE/nlt
    v4sf cmp_result_f5 = vf1 == vf2;  // May generate UNEQ
    v4sf cmp_result_f6 = vf1 != vf2;  // May generate LTGT/une
    
    v2df cmp_result_d = vd1 < vd2;    // May generate UNLT/ult
    v2df cmp_result_d2 = vd1 > vd2;   // May generate UNGT/nle
    v2df cmp_result_d3 = vd1 <= vd2;  // May generate UNLE/ule
    v2df cmp_result_d4 = vd1 >= vd2;  // May generate UNGE/nlt
    v2df cmp_result_d5 = vd1 == vd2;  // May generate UNEQ
    v2df cmp_result_d6 = vd1 != vd2;  // May generate LTGT/une
    
    // Reduce vector results to scalar for checksum
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += cmp_result_f[i] ? (1 << i) : 0;
        result += cmp_result_f2[i] ? (1 << (i + 4)) : 0;
        result += cmp_result_f3[i] ? (1 << (i + 8)) : 0;
        result += cmp_result_f4[i] ? (1 << (i + 12)) : 0;
        result += cmp_result_f5[i] ? (1 << (i + 16)) : 0;
        result += cmp_result_f6[i] ? (1 << (i + 20)) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        result += cmp_result_d[i] ? (1 << (i + 24)) : 0;
        result += cmp_result_d2[i] ? (1 << (i + 26)) : 0;
        result += cmp_result_d3[i] ? (1 << (i + 28)) : 0;
        result += cmp_result_d4[i] ? (1 << (i + 30)) : 0;
    }
    
    checksum += result;
}

// Test inline assembly with condition codes
void test_asm(float f1, float f2, double d1, double d2) {
    unsigned char byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
    unsigned char byte5 = 0, byte6 = 0, byte7 = 0, byte8 = 0;
    
    // Test various condition codes in inline assembly
    // These force the assembly printer to resolve condition code names
    
    // UNORDERED
    __asm__ volatile (
        "fucomi %%st(1), %%st\n\t"
        "setp %0"
        : "=r" (byte1)
        : 
        : "cc"
    );
    
    // ORDERED
    __asm__ volatile (
        "fucomi %%st(1), %%st\n\t"
        "setnp %0"
        : "=r" (byte2)
        : 
        : "cc"
    );
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "sete %0"
        : "=r" (byte3)
        : 
        : "cc"
    );
    
    // UNGE (not less than)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "setae %0"
        : "=r" (byte4)
        : 
        : "cc"
    );
    
    // UNGT (not less or equal)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "seta %0"
        : "=r" (byte5)
        : 
        : "cc"
    );
    
    // UNLE (unordered or less or equal)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "setbe %0"
        : "=r" (byte6)
        : 
        : "cc"
    );
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "setb %0"
        : "=r" (byte7)
        : 
        : "cc"
    );
    
    // LTGT (unordered or not equal)
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "setne %0"
        : "=r" (byte8)
        : 
        : "cc"
    );
    
    checksum += byte1 + byte2 + byte3 + byte4 + byte5 + byte6 + byte7 + byte8;
}

// Complex control flow to force condition code materialization
void test_control_flow(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // Complex nested if-else with mixed comparisons
    for (int i = 0; i < 10; i++) {
        if (f1 < f2) {                     // May generate UNLT/ult
            result += i;
            if (d1 > d2) {                 // May generate UNGT/nle
                result += i * 2;
                if (f1 == f2) {            // May generate UNEQ
                    result += i * 3;
                    continue;
                }
            } else if (d1 <= d2) {         // May generate UNLE/ule
                result += i * 4;
                goto label1;
            }
        } else if (f1 >= f2) {             // May generate UNGE/nlt
            result += i * 5;
            switch (i % 4) {
                case 0:
                    if (d1 != d2) {        // May generate LTGT/une
                        result += 100;
                    }
                    break;
                case 1:
                    if (f1 != f1) {        // UNORDERED
                        result += 200;
                    }
                    break;
                case 2:
                    if (d1 == d1) {        // ORDERED
                        result += 300;
                    }
                    break;
                default:
                    result += 400;
                    break;
            }
        }
        label1:
        if (i == 5) break;
    }
    
    checksum += result;
}

int main() {
    // Initialize test values including special floating-point values
    float f_values[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        NAN, INFINITY, -INFINITY,
        3.14f, -2.71f
    };
    
    double d_values[] = {
        1.0, 2.0, 0.0, -1.0,
        NAN, INFINITY, -INFINITY,
        3.141592653589793, -2.718281828459045
    };
    
    // Run tests with various combinations
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            test_scalar_cmps(f_values[i], f_values[j], 
                            d_values[i], d_values[j]);
            test_builtins(f_values[i], f_values[j], 
                         d_values[i], d_values[j]);
            test_vector(f_values[i], f_values[j], 
                       d_values[i], d_values[j]);
            test_control_flow(f_values[i], f_values[j], 
                            d_values[i], d_values[j]);
            
            // Test inline assembly less frequently
            if ((i + j) % 3 == 0) {
                test_asm(f_values[i], f_values[j], 
                        d_values[i], d_values[j]);
            }
        }
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
