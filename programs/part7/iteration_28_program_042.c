#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.5f};
double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.5};

/* Test scalar comparisons with all relational operators */
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    /* Mix float and double comparisons */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Use ternary operator to force CMOV/SET generation */
            results[idx++] = (fvals[i] < fvals[j]) ? 1 : 0;      /* LT */
            results[idx++] = (fvals[i] > fvals[j]) ? 2 : 0;      /* GT */
            results[idx++] = (fvals[i] <= fvals[j]) ? 3 : 0;     /* LE */
            results[idx++] = (fvals[i] >= fvals[j]) ? 4 : 0;     /* GE */
            results[idx++] = (fvals[i] == fvals[j]) ? 5 : 0;     /* EQ */
            results[idx++] = (fvals[i] != fvals[j]) ? 6 : 0;     /* NEQ */
            
            results[idx++] = (dvals[i] < dvals[j]) ? 7 : 0;
            results[idx++] = (dvals[i] > dvals[j]) ? 8 : 0;
            results[idx++] = (dvals[i] <= dvals[j]) ? 9 : 0;
            results[idx++] = (dvals[i] >= dvals[j]) ? 10 : 0;
            results[idx++] = (dvals[i] == dvals[j]) ? 11 : 0;
            results[idx++] = (dvals[i] != dvals[j]) ? 12 : 0;
        }
    }
    
    /* Complex control flow with nested if-else */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        if (results[i] > 0) {
            if (results[i] % 2 == 0) {
                sum += results[i];
            } else {
                sum -= results[i];
            }
        } else {
            switch (results[i] & 3) {
                case 0: sum += 100; break;
                case 1: sum += 200; break;
                case 2: sum += 300; break;
                default: sum += 400; break;
            }
        }
    }
    
    return sum;
}

/* Test built-in unordered comparison functions */
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    /* Test all __builtin_is* functions */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* These directly map to specific condition codes */
            results[idx++] = __builtin_isgreater(fvals[i], fvals[j]) ? 1 : 0;
            results[idx++] = __builtin_isless(fvals[i], fvals[j]) ? 2 : 0;
            results[idx++] = __builtin_isgreaterequal(fvals[i], fvals[j]) ? 3 : 0;
            results[idx++] = __builtin_islessequal(fvals[i], fvals[j]) ? 4 : 0;
            results[idx++] = __builtin_isunordered(fvals[i], fvals[j]) ? 5 : 0;
            
            results[idx++] = __builtin_isgreater(dvals[i], dvals[j]) ? 6 : 0;
            results[idx++] = __builtin_isless(dvals[i], dvals[j]) ? 7 : 0;
            results[idx++] = __builtin_isgreaterequal(dvals[i], dvals[j]) ? 8 : 0;
            results[idx++] = __builtin_islessequal(dvals[i], dvals[j]) ? 9 : 0;
            results[idx++] = __builtin_isunordered(dvals[i], dvals[j]) ? 10 : 0;
        }
    }
    
    /* Use classification functions */
    for (int i = 0; i < 8; i++) {
        results[idx++] = isnan(fvals[i]) ? 20 : 0;
        results[idx++] = isinf(fvals[i]) ? 30 : 0;
        results[idx++] = isnan(dvals[i]) ? 40 : 0;
        results[idx++] = isinf(dvals[i]) ? 50 : 0;
        results[idx++] = (fpclassify(dvals[i]) == FP_NAN) ? 60 : 0;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test vector/SIMD comparisons */
int test_vector(void) {
    v4sf vf1 = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vf2 = {2.0f, 1.0f, INFINITY, NAN};
    v2df vd1 = {1.0, NAN};
    v2df vd2 = {NAN, 1.0};
    
    int results[16];
    int idx = 0;
    
    /* Vector comparisons generate packed RTL */
    v4sf cmp1 = vf1 < vf2;
    v4sf cmp2 = vf1 > vf2;
    v4sf cmp3 = vf1 == vf2;
    v4sf cmp4 = vf1 != vf2;
    
    v2df cmp5 = vd1 < vd2;
    v2df cmp6 = vd1 > vd2;
    v2df cmp7 = vd1 == vd2;
    v2df cmp8 = vd1 != vd2;
    
    /* Reduce vector to scalar for condition code generation */
    float* f1 = (float*)&cmp1;
    float* f2 = (float*)&cmp2;
    double* d1 = (double*)&cmp5;
    double* d2 = (double*)&cmp6;
    
    for (int i = 0; i < 4; i++) {
        results[idx++] = (f1[i] != 0.0f) ? 1 : 0;
        results[idx++] = (f2[i] != 0.0f) ? 2 : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = (d1[i] != 0.0) ? 3 : 0;
        results[idx++] = (d2[i] != 0.0) ? 4 : 0;
    }
    
    /* Complex loop with break/continue */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        if (results[i] == 0) {
            continue;
        }
        if (sum > 1000) {
            break;
        }
        sum += results[i];
        
        /* Use goto to create interesting control flow */
        if (results[i] > 2) {
            goto add_extra;
        }
        sum += 10;
        continue;
        
    add_extra:
        sum += 100;
    }
    
    return sum;
}

/* Test inline assembly with condition code constraints */
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double a = dvals[i];
            double b = dvals[j];
            
            /* Inline assembly that uses condition codes */
            unsigned char r1, r2, r3, r4, r5, r6, r7, r8;
            
            /* Test various condition codes */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "seta %0"
                : "=r"(r1) : "x"(a), "x"(b) : "cc");
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %0"
                : "=r"(r2) : "x"(a), "x"(b) : "cc");
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %0"
                : "=r"(r3) : "x"(a), "x"(b) : "cc");
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %0"
                : "=r"(r4) : "x"(a), "x"(b) : "cc");
            
            /* Test unordered/ordered conditions */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %0"
                : "=r"(r5) : "x"(a), "x"(b) : "cc");
            
            /* Use 'g' constraint to let compiler choose register */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setae %0"
                : "=g"(r6) : "x"(a), "x"(b) : "cc");
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %0"
                : "=g"(r7) : "x"(a), "x"(b) : "cc");
            
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %0"
                : "=g"(r8) : "x"(a), "x"(b) : "cc");
            
            results[idx++] = r1;
            results[idx++] = r2;
            results[idx++] = r3;
            results[idx++] = r4;
            results[idx++] = r5;
            results[idx++] = r6;
            results[idx++] = r7;
            results[idx++] = r8;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main function with all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Run all test functions */
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
