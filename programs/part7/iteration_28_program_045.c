#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};
int results[256];
int result_idx = 0;

// Test scalar comparisons with all relational operators
void test_scalar_cmps(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    // Complex control flow with nested if-else and switch
    for (int i = 0; i < 8; i++) {
        f1 = fvals[i];
        d1 = dvals[i];
        
        for (int j = 0; j < 8; j++) {
            f2 = fvals[j];
            d2 = dvals[j];
            
            // Mix float and double comparisons
            int r1 = (f1 < f2) ? 1 : 0;           // May generate UNLT/UNORDERED
            int r2 = (d1 > d2) ? 2 : 0;           // May generate UNGT/UNORDERED
            int r3 = (f1 <= f2) ? 3 : 0;          // May generate UNLE
            int r4 = (d1 >= d2) ? 4 : 0;          // May generate UNGE
            int r5 = (f1 == f2) ? 5 : 0;          // May generate UNEQ
            int r6 = (d1 != d2) ? 6 : 0;          // May generate LTGT
            
            // Store results
            results[result_idx++] = r1 + r2 + r3 + r4 + r5 + r6;
            
            // Complex control flow with switch
            switch (i + j) {
                case 0:
                    results[result_idx++] = (f1 < f2) ? 100 : 200;
                    break;
                case 1:
                    results[result_idx++] = (d1 > d2) ? 101 : 201;
                    break;
                case 2:
                    results[result_idx++] = (f1 <= f2) ? 102 : 202;
                    continue;  // Skip next iteration
                case 3:
                    results[result_idx++] = (d1 >= d2) ? 103 : 203;
                    goto skip_point;
                default:
                    results[result_idx++] = (f1 != f2) ? 104 : 204;
            }
            
            skip_point:
            // Use classification functions
            int c1 = isnan(f1) ? 10 : 20;
            int c2 = isinf(d1) ? 30 : 40;
            int c3 = fpclassify(f2) == FP_NAN ? 50 : 60;
            results[result_idx++] = c1 + c2 + c3;
        }
    }
}

// Test built-in unordered comparison functions
void test_builtins(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    for (int i = 0; i < 8; i++) {
        f1 = fvals[i];
        d1 = dvals[i];
        
        for (int j = 0; j < 8; j++) {
            f2 = fvals[j];
            d2 = dvals[j];
            
            // Built-in functions that directly map to condition codes
            int r1 = __builtin_isgreater(d1, d2) ? 1 : 0;      // ORDERED & GT
            int r2 = __builtin_isless(f1, f2) ? 2 : 0;         // ORDERED & LT
            int r3 = __builtin_isunordered(d1, d2) ? 3 : 0;    // UNORDERED
            int r4 = __builtin_isgreaterequal(f1, f2) ? 4 : 0; // ORDERED & GE
            int r5 = __builtin_islessequal(d1, d2) ? 5 : 0;    // ORDERED & LE
            int r6 = __builtin_islessgreater(f1, f2) ? 6 : 0;  // ORDERED & LTGT
            
            // Ternary operator forces CMOV/SET generation
            int val1 = r1 ? 1000 : 2000;
            int val2 = r2 ? 3000 : 4000;
            int val3 = r3 ? 5000 : 6000;
            
            results[result_idx++] = val1 + val2 + val3;
            results[result_idx++] = r4 + r5 + r6;
            
            // Nested ternary with builtins
            int complex = __builtin_isunordered(f1, f2) ? 
                         (__builtin_isgreater(d1, d2) ? 1 : 2) :
                         (__builtin_isless(f1, f2) ? 3 : 4);
            results[result_idx++] = complex;
        }
    }
}

// Test vector/SIMD comparisons
void test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vb = {2.0f, 1.0f, INFINITY, NAN};
    v2df vc = {1.0, NAN};
    v2df vd = {NAN, 1.0};
    
    // Vector comparisons generate packed comparison RTL
    v4sf mask1 = va < vb;    // May generate UNLT
    v4sf mask2 = va > vb;    // May generate UNGT
    v4sf mask3 = va <= vb;   // May generate UNLE
    v4sf mask4 = va >= vb;   // May generate UNGE
    v4sf mask5 = va == vb;   // May generate UNEQ
    v4sf mask6 = va != vb;   // May generate LTGT
    
    v2df mask7 = vc < vd;
    v2df mask8 = vc > vd;
    
    // Reduce vector masks to scalar
    float* m1 = (float*)&mask1;
    float* m2 = (float*)&mask2;
    double* m7 = (double*)&mask7;
    
    for (int i = 0; i < 4; i++) {
        results[result_idx++] = m1[i] != 0.0f ? 1 : 0;
        results[result_idx++] = m2[i] != 0.0f ? 2 : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[result_idx++] = m7[i] != 0.0 ? 3 : 0;
    }
    
    // Vector built-in unordered comparisons
    v4sf vx = {1.0f, NAN, 2.0f, INFINITY};
    v4sf vy = {NAN, 1.0f, INFINITY, 2.0f};
    
    // This may generate different condition codes
    for (int i = 0; i < 4; i++) {
        float x = vx[i];
        float y = vy[i];
        int cmp = __builtin_isunordered(x, y) ? 10 : 
                  __builtin_isgreater(x, y) ? 20 : 30;
        results[result_idx++] = cmp;
    }
}

// Test inline assembly with condition code constraints
void test_asm(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile float c = 2.0f;
    volatile float d = INFINITY;
    
    uint8_t byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    
    // Inline assembly that uses condition code names
    // These force the assembly printer to resolve symbolic condition codes
    
    // Test UNORDERED
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(byte1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    // Test ORDERED
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(byte2)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    // Test UNEQ (unordered or equal)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"
        : "=r"(byte3)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    // Test UNGE (not less than)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r"(byte4)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    // Test UNGT (not less or equal)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %0"
        : "=r"(byte5)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    // Test UNLE (unordered or less or equal)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(byte6)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    // Test UNLT (unordered or less than)
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte7)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    // Test LTGT (less or greater, ordered)
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(byte8)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    results[result_idx++] = byte1;
    results[result_idx++] = byte2;
    results[result_idx++] = byte3;
    results[result_idx++] = byte4;
    results[result_idx++] = byte5;
    results[result_idx++] = byte6;
    results[result_idx++] = byte7;
    results[result_idx++] = byte8;
    
    // More complex inline assembly with "g" constraint
    int x = 0;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "mov $0, %0\n\t"
        "seta %0"
        : "=r"(x)
        : "g"(a), "g"(b)
        : "cc"
    );
    results[result_idx++] = x;
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
    unsigned int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFFFFFF;
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Tests completed. Result count: %d\n", result_idx);
    
    return 0;
}
