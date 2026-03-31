#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special values
float farr[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
double darr[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};
int results[256];
int result_idx = 0;

// Test scalar comparisons with all operators
void test_scalar_cmps(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    // Complex control flow with nested if-else and switch
    for (int i = 0; i < 8; i++) {
        f1 = farr[i];
        d1 = darr[i];
        
        for (int j = 0; j < 8; j++) {
            f2 = farr[j];
            d2 = darr[j];
            
            // Store comparison results using ternary operators (forces CMOV/SET)
            int r1 = (f1 < f2) ? 1 : 0;
            int r2 = (f1 > f2) ? 2 : 0;
            int r3 = (f1 <= f2) ? 3 : 0;
            int r4 = (f1 >= f2) ? 4 : 0;
            int r5 = (f1 == f2) ? 5 : 0;
            int r6 = (f1 != f2) ? 6 : 0;
            
            int r7 = (d1 < d2) ? 7 : 0;
            int r8 = (d1 > d2) ? 8 : 0;
            int r9 = (d1 <= d2) ? 9 : 0;
            int r10 = (d1 >= d2) ? 10 : 0;
            int r11 = (d1 == d2) ? 11 : 0;
            int r12 = (d1 != d2) ? 12 : 0;
            
            // Use classification functions
            int r13 = isnan(f1) ? 13 : 0;
            int r14 = isinf(d1) ? 14 : 0;
            int r15 = (fpclassify(f2) == FP_NAN) ? 15 : 0;
            int r16 = (fpclassify(d2) == FP_INFINITE) ? 16 : 0;
            
            // Complex switch to force condition code materialization
            switch ((i + j) % 8) {
                case 0:
                    results[result_idx++] = r1 + r7;
                    break;
                case 1:
                    results[result_idx++] = r2 + r8;
                    if (r3 > 0) goto label1;
                    break;
                case 2:
                    results[result_idx++] = r3 + r9;
                    continue;
                case 3:
                    results[result_idx++] = r4 + r10;
                    break;
                case 4:
                    results[result_idx++] = r5 + r11;
                label1:
                    results[result_idx++] = r13;
                    break;
                case 5:
                    results[result_idx++] = r6 + r12;
                    if (r14 > 0) continue;
                    break;
                case 6:
                    results[result_idx++] = r15;
                    break;
                case 7:
                    results[result_idx++] = r16;
                    break;
                default:
                    break;
            }
        }
    }
}

// Test builtin unordered comparisons
void test_builtins(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    for (int i = 0; i < 8; i++) {
        f1 = farr[i];
        d1 = darr[i];
        
        for (int j = 0; j < 8; j++) {
            f2 = farr[j];
            d2 = darr[j];
            
            // Use all __builtin_is* functions
            int r1 = __builtin_isgreater(f1, f2) ? 1 : 0;
            int r2 = __builtin_isgreaterequal(f1, f2) ? 2 : 0;
            int r3 = __builtin_isless(f1, f2) ? 3 : 0;
            int r4 = __builtin_islessequal(f1, f2) ? 4 : 0;
            int r5 = __builtin_islessgreater(f1, f2) ? 5 : 0;
            int r6 = __builtin_isunordered(f1, f2) ? 6 : 0;
            
            int r7 = __builtin_isgreater(d1, d2) ? 7 : 0;
            int r8 = __builtin_isgreaterequal(d1, d2) ? 8 : 0;
            int r9 = __builtin_isless(d1, d2) ? 9 : 0;
            int r10 = __builtin_islessequal(d1, d2) ? 10 : 0;
            int r11 = __builtin_islessgreater(d1, d2) ? 11 : 0;
            int r12 = __builtin_isunordered(d1, d2) ? 12 : 0;
            
            // Mix with regular comparisons in ternary
            int r13 = (__builtin_isunordered(f1, f2) || f1 == f2) ? 13 : 0;
            int r14 = (!__builtin_isunordered(d1, d2) && d1 <= d2) ? 14 : 0;
            
            results[result_idx++] = r1 + r2 + r3 + r4 + r5 + r6;
            results[result_idx++] = r7 + r8 + r9 + r10 + r11 + r12;
            results[result_idx++] = r13 + r14;
        }
    }
}

// Test vector comparisons
void test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    // Vector comparisons generate packed RTL
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 <= vf2;
    v4sf cmp4 = vf1 >= vf2;
    v4sf cmp5 = vf1 == vf2;
    v4sf cmp6 = vf1 != vf2;
    
    v2df cmp7 = vd1 < vd2;
    v2df cmp8 = vd1 > vd2;
    v2df cmp9 = vd1 <= vd2;
    v2df cmp10 = vd1 >= vd2;
    v2df cmp11 = vd1 == vd2;
    v2df cmp12 = vd1 != vd2;
    
    // Reduce to scalar mask
    int mask1 = 0, mask2 = 0;
    for (int i = 0; i < 4; i++) {
        mask1 |= (((int*)&cmp1)[i] != 0) << i;
        mask2 |= (((int*)&cmp5)[i] != 0) << i;
    }
    
    results[result_idx++] = mask1;
    results[result_idx++] = mask2;
    
    // Use vector results in conditional
    int r = (mask1 > 0) ? 100 : 200;
    results[result_idx++] = r;
}

// Test inline assembly with condition codes
void test_asm(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile float c = 2.0f;
    volatile float d = INFINITY;
    
    uint8_t byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    
    // Test various condition codes in inline asm
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r"(byte1) : "x"(a), "x"(b) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte2) : "x"(c), "x"(d) : "cc");
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"
        : "=r"(byte3) : "x"(a), "x"(a) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setp %0"
        : "=r"(byte4) : "x"(c), "x"(b) : "cc");
    
    // Use symbolic condition codes that map to the uncovered cases
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setg %0"
        : "=g"(byte5) : "x"(a), "x"(a+1.0) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setl %0"
        : "=g"(byte6) : "x"(c), "x"(d) : "cc");
    
    // Test unordered/ordered conditions
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=g"(byte7) : "x"(b), "x"(b) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=g"(byte8) : "x"(c), "x"(c) : "cc");
    
    results[result_idx++] = byte1 + byte2 + byte3 + byte4;
    results[result_idx++] = byte5 + byte6 + byte7 + byte8;
}

int main(void) {
    // Initialize results array
    memset(results, 0, sizeof(results));
    result_idx = 0;
    
    // Run all tests
    test_scalar_cmps();
    test_builtins();
    test_vector();
    test_asm();
    
    // Calculate checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed. Result count: %d\n", result_idx);
    
    return 0;
}
