#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special values
float farr[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.14f,
    4.0f, 5.0f, NAN, -INFINITY, INFINITY, -2.0f, 0.5f, -0.5f
};

double darr[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.1415926535,
    4.0, 5.0, NAN, -INFINITY, INFINITY, -2.0, 0.5, -0.5
};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[32];
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i+8];
        double d1 = darr[i];
        double d2 = darr[i+8];
        
        // UNORDERED cases (NaN involved)
        results[idx++] = (f1 != f1) ? 1 : 0;  // isnan check
        results[idx++] = (d1 != d1) ? 2 : 0;
        
        // ORDERED cases
        results[idx++] = (f1 == f1 && f2 == f2) ? 3 : 0;
        results[idx++] = (d1 == d1 && d2 == d2) ? 4 : 0;
        
        // UNEQ (unordered or equal)
        results[idx++] = (!(f1 < f2) && !(f1 > f2)) ? 5 : 0;
        results[idx++] = (!(d1 < d2) && !(d1 > d2)) ? 6 : 0;
        
        // UNGE (not less than)
        results[idx++] = (!(f1 < f2)) ? 7 : 0;
        results[idx++] = (!(d1 < d2)) ? 8 : 0;
        
        // UNGT (not less than or equal)
        results[idx++] = (!(f1 <= f2)) ? 9 : 0;
        results[idx++] = (!(d1 <= d2)) ? 10 : 0;
        
        // UNLE (unordered or less than or equal)
        results[idx++] = ((f1 <= f2) || (f1 != f1) || (f2 != f2)) ? 11 : 0;
        results[idx++] = ((d1 <= d2) || (d1 != d1) || (d2 != d2)) ? 12 : 0;
        
        // UNLT (unordered or less than)
        results[idx++] = ((f1 < f2) || (f1 != f1) || (f2 != f2)) ? 13 : 0;
        results[idx++] = ((d1 < d2) || (d1 != d1) || (d2 != d2)) ? 14 : 0;
        
        // LTGT (less than or greater than, ordered)
        results[idx++] = ((f1 < f2) || (f1 > f2)) ? 15 : 0;
        results[idx++] = ((d1 < d2) || (d1 > d2)) ? 16 : 0;
    }
    
    // Complex control flow with nested if-else and goto
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        if (results[i] > 0) {
            if (results[i] % 2 == 0) {
                sum += results[i];
                continue;
            } else {
                sum -= results[i];
                goto add_more;
            }
        }
        sum += 1;
        add_more:
        sum += 2;
    }
    
    return sum;
}

// Test builtin unordered comparisons
int test_builtins(void) {
    int results = 0;
    
    // Test all __builtin functions that generate condition codes
    for (int i = 0; i < 8; i++) {
        float f1 = farr[i];
        float f2 = farr[i+8];
        double d1 = darr[i];
        double d2 = darr[i+8];
        
        // __builtin_isunordered - generates UNORDERED condition
        results += __builtin_isunordered(f1, f2) ? 1 : 0;
        results += __builtin_isunordered(d1, d2) ? 2 : 0;
        
        // __builtin_isgreater - generates GT condition (ordered)
        results += __builtin_isgreater(f1, f2) ? 4 : 0;
        results += __builtin_isgreater(d1, d2) ? 8 : 0;
        
        // __builtin_isless - generates LT condition (ordered)
        results += __builtin_isless(f1, f2) ? 16 : 0;
        results += __builtin_isless(d1, d2) ? 32 : 0;
        
        // __builtin_isgreaterequal - generates GE condition
        results += __builtin_isgreaterequal(f1, f2) ? 64 : 0;
        results += __builtin_isgreaterequal(d1, d2) ? 128 : 0;
        
        // __builtin_islessequal - generates LE condition
        results += __builtin_islessequal(f1, f2) ? 256 : 0;
        results += __builtin_islessequal(d1, d2) ? 512 : 0;
        
        // __builtin_islessgreater - generates LTGT condition
        results += __builtin_islessgreater(f1, f2) ? 1024 : 0;
        results += __builtin_islessgreater(d1, d2) ? 2048 : 0;
    }
    
    // Switch statement to force condition code materialization
    switch (results % 8) {
        case 0:
            results += 1000;
            break;
        case 1:
            results += 2000;
            break;
        case 2:
            results += 3000;
            break;
        case 3:
            results += 4000;
            break;
        case 4:
            results += 5000;
            break;
        case 5:
            results += 6000;
            break;
        case 6:
            results += 7000;
            break;
        case 7:
            results += 8000;
            break;
    }
    
    return results;
}

// Test vector comparisons
int test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vb = {2.0f, 1.0f, INFINITY, NAN};
    v2df vc = {1.0, NAN};
    v2df vd = {NAN, 1.0};
    
    int mask = 0;
    
    // Vector comparisons that generate packed condition codes
    v4sf cmp1 = va < vb;  // UNLT for vector elements
    v4sf cmp2 = va > vb;  // UNGT for vector elements
    v4sf cmp3 = va <= vb; // UNLE for vector elements
    v4sf cmp4 = va >= vb; // UNGE for vector elements
    v4sf cmp5 = va == vb; // UNEQ for vector elements
    v4sf cmp6 = va != vb; // LTGT for vector elements
    
    v2df cmp7 = vc < vd;
    v2df cmp8 = vc > vd;
    v2df cmp9 = vc <= vd;
    v2df cmp10 = vc >= vd;
    v2df cmp11 = vc == vd;
    v2df cmp12 = vc != vd;
    
    // Reduce vector to scalar mask
    for (int i = 0; i < 4; i++) {
        mask += ((int*)&cmp1)[i] ? (1 << i) : 0;
        mask += ((int*)&cmp2)[i] ? (2 << i) : 0;
        mask += ((int*)&cmp3)[i] ? (4 << i) : 0;
        mask += ((int*)&cmp4)[i] ? (8 << i) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        mask += ((long long*)&cmp7)[i] ? (16 << i) : 0;
        mask += ((long long*)&cmp8)[i] ? (32 << i) : 0;
    }
    
    return mask;
}

// Test inline assembly with condition codes
int test_asm(void) {
    int results[8] = {0};
    
    for (int i = 0; i < 8; i++) {
        double a = darr[i];
        double b = darr[i+8];
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        // Inline assembly with various condition codes
        __asm__ volatile (
            // UNORDERED
            "ucomisd %2, %1\n\t"
            "setp %0\n\t"
            : "=r"(r1) : "x"(a), "x"(b) : "cc"
        );
        
        // ORDERED
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0\n\t"
            : "=r"(r2) : "x"(a), "x"(b) : "cc"
        );
        
        // UNEQ (unordered or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"
            : "=r"(r3) : "x"(a), "x"(b) : "cc"
        );
        
        // UNGE (not less than)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnb %0\n\t"
            : "=r"(r4) : "x"(a), "x"(b) : "cc"
        );
        
        // UNGT (not less than or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnbe %0\n\t"
            : "=r"(r5) : "x"(a), "x"(b) : "cc"
        );
        
        // UNLE (unordered or less than or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %0\n\t"
            : "=r"(r6) : "x"(a), "x"(b) : "cc"
        );
        
        // UNLT (unordered or less than)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0\n\t"
            : "=r"(r7) : "x"(a), "x"(b) : "cc"
        );
        
        // LTGT (less than or greater than)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %0\n\t"
            : "=r"(r8) : "x"(a), "x"(b) : "cc"
        );
        
        results[i] = r1 + r2*2 + r3*4 + r4*8 + r5*16 + r6*32 + r7*64 + r8*128;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

// Test fpclassify and classification functions
int test_classification(void) {
    int results = 0;
    
    for (int i = 0; i < 16; i++) {
        float f = farr[i];
        double d = darr[i];
        
        // fpclassify generates condition codes for classification
        int fclass = fpclassify(f);
        int dclass = fpclassify(d);
        
        // Use ternary operator to force CMOV generation
        results += (fclass == FP_NAN) ? 1 : 
                   (fclass == FP_INFINITE) ? 2 :
                   (fclass == FP_ZERO) ? 3 :
                   (fclass == FP_SUBNORMAL) ? 4 : 5;
        
        results += (dclass == FP_NAN) ? 10 : 
                   (dclass == FP_INFINITE) ? 20 :
                   (dclass == FP_ZERO) ? 30 :
                   (dclass == FP_SUBNORMAL) ? 40 : 50;
        
        // isnan and isinf checks
        results += isnan(f) ? 100 : 0;
        results += isinf(f) ? 200 : 0;
        results += isnan(d) ? 300 : 0;
        results += isinf(d) ? 400 : 0;
    }
    
    return results;
}

int main(void) {
    int checksum = 0;
    
    // Run all tests
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    checksum += test_classification();
    
    // Print result to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
