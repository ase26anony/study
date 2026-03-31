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
            float f1 = fvals[i];
            float f2 = fvals[j];
            double d1 = dvals[i];
            double d2 = dvals[j];
            
            /* Use ternary operator to force CMOV/SET generation */
            results[idx++] = (f1 < f2) ? 1 : 0;      /* UNLT */
            results[idx++] = (f1 > f2) ? 2 : 0;      /* UNGT */
            results[idx++] = (f1 <= f2) ? 3 : 0;     /* UNLE */
            results[idx++] = (f1 >= f2) ? 4 : 0;     /* UNGE */
            results[idx++] = (f1 == f2) ? 5 : 0;     /* UNEQ */
            results[idx++] = (f1 != f2) ? 6 : 0;     /* LTGT */
            
            /* Double comparisons */
            results[idx++] = (d1 < d2) ? 7 : 0;
            results[idx++] = (d1 > d2) ? 8 : 0;
            results[idx++] = (d1 <= d2) ? 9 : 0;
            results[idx++] = (d1 >= d2) ? 10 : 0;
            results[idx++] = (d1 == d2) ? 11 : 0;
            results[idx++] = (d1 != d2) ? 12 : 0;
        }
    }
    
    /* Complex control flow with nested if-else */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        if (results[i] > 0) {
            if (fvals[i % 8] < dvals[i % 8]) {
                sum += results[i];
            } else if (fvals[i % 8] > dvals[i % 8]) {
                sum -= results[i];
            } else {
                sum ^= results[i];
            }
        }
    }
    
    return sum;
}

/* Test built-in unordered comparisons */
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    /* Test all __builtin_is* functions */
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[(i + 1) % 8];
        double d1 = dvals[i];
        double d2 = dvals[(i + 2) % 8];
        
        /* These built-ins directly map to condition codes */
        results[idx++] = __builtin_isunordered(f1, f2) ? 1 : 0;    /* UNORDERED */
        results[idx++] = __builtin_isgreater(f1, f2) ? 2 : 0;      /* UNGT */
        results[idx++] = __builtin_isless(f1, f2) ? 3 : 0;         /* UNLT */
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 4 : 0; /* UNGE */
        results[idx++] = __builtin_islessequal(f1, f2) ? 5 : 0;    /* UNLE */
        
        /* Double versions */
        results[idx++] = __builtin_isunordered(d1, d2) ? 6 : 0;
        results[idx++] = __builtin_isgreater(d1, d2) ? 7 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 8 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 9 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 10 : 0;
        
        /* Test ORDERED condition */
        results[idx++] = (!__builtin_isunordered(f1, f2)) ? 11 : 0; /* ORDERED */
        results[idx++] = (!__builtin_isunordered(d1, d2)) ? 12 : 0;
    }
    
    /* Use switch statement for complex control flow */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        switch (results[i] % 8) {
            case 0:
                if (__builtin_isunordered(fvals[i % 8], dvals[i % 8]))
                    sum += 1;
                break;
            case 1:
                if (__builtin_isgreater(fvals[i % 8], dvals[i % 8]))
                    sum += 2;
                break;
            case 2:
                if (__builtin_isless(fvals[i % 8], dvals[i % 8]))
                    sum += 3;
                break;
            case 3:
                sum += results[i];
                break;
            default:
                sum ^= results[i];
                break;
        }
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
    v4sf vcmp_lt = vf1 < vf2;    /* UNLT */
    v4sf vcmp_gt = vf1 > vf2;    /* UNGT */
    v4sf vcmp_le = vf1 <= vf2;   /* UNLE */
    v4sf vcmp_ge = vf1 >= vf2;   /* UNGE */
    v4sf vcmp_eq = vf1 == vf2;   /* UNEQ */
    v4sf vcmp_neq = vf1 != vf2;  /* LTGT */
    
    /* Reduce vector to scalar */
    for (int i = 0; i < 4; i++) {
        results[idx++] = vcmp_lt[i] ? 1 : 0;
        results[idx++] = vcmp_gt[i] ? 2 : 0;
        results[idx++] = vcmp_le[i] ? 3 : 0;
        results[idx++] = vcmp_ge[i] ? 4 : 0;
        results[idx++] = vcmp_eq[i] ? 5 : 0;
        results[idx++] = vcmp_neq[i] ? 6 : 0;
    }
    
    /* Double vector comparisons */
    v2df vd_cmp_lt = vd1 < vd2;
    v2df vd_cmp_gt = vd1 > vd2;
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = vd_cmp_lt[i] ? 7 : 0;
        results[idx++] = vd_cmp_gt[i] ? 8 : 0;
    }
    
    /* Complex loop with goto for control flow diversification */
    int sum = 0;
    int i = 0;
    
loop_start:
    if (i >= idx) goto loop_end;
    
    if (results[i] > 0) {
        sum += results[i];
        i++;
        goto loop_start;
    } else {
        i++;
        if (i % 3 == 0) {
            continue;  /* Simulated continue */
        }
        goto loop_start;
    }
    
loop_end:
    return sum;
}

/* Test inline assembly with condition code constraints */
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[(i + 3) % 8];
        double d1 = dvals[i];
        double d2 = dvals[(i + 5) % 8];
        
        /* Inline assembly that uses condition codes directly */
        unsigned char b1, b2, b3, b4, b5, b6, b7, b8;
        
        /* Test various condition codes */
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "seta %0"
            : "=r" (b1) : "x" (f1), "x" (f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0"
            : "=r" (b2) : "x" (d1), "x" (d2) : "cc");
        
        /* Test unordered/ordered conditions */
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setp %0"
            : "=r" (b3) : "x" (f1), "x" (f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0"
            : "=r" (b4) : "x" (d1), "x" (d2) : "cc");
        
        /* Test equality conditions */
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "sete %0"
            : "=r" (b5) : "x" (f1), "x" (f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %0"
            : "=r" (b6) : "x" (d1), "x" (d2) : "cc");
        
        /* Test greater/less conditions */
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setg %0"
            : "=r" (b7) : "x" (f1), "x" (f2) : "cc");
        
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setl %0"
            : "=r" (b8) : "x" (d1), "x" (d2) : "cc");
        
        results[idx++] = b1;
        results[idx++] = b2;
        results[idx++] = b3;
        results[idx++] = b4;
        results[idx++] = b5;
        results[idx++] = b6;
        results[idx++] = b7;
        results[idx++] = b8;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    
    return sum;
}

/* Test classification functions */
int test_classification(void) {
    int results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float f = fvals[i];
        double d = dvals[i];
        
        /* These may generate condition code checks */
        results[idx++] = isnan(f) ? 1 : 0;
        results[idx++] = isinf(f) ? 2 : 0;
        results[idx++] = isnormal(f) ? 3 : 0;
        results[idx++] = fpclassify(f) == FP_NAN ? 4 : 0;
        results[idx++] = fpclassify(f) == FP_INFINITE ? 5 : 0;
        
        results[idx++] = isnan(d) ? 6 : 0;
        results[idx++] = isinf(d) ? 7 : 0;
        results[idx++] = isnormal(d) ? 8 : 0;
        results[idx++] = fpclassify(d) == FP_NAN ? 9 : 0;
        results[idx++] = fpclassify(d) == FP_INFINITE ? 10 : 0;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate results */
    checksum ^= test_scalar_cmps();
    checksum ^= test_builtins();
    checksum ^= test_vector();
    checksum ^= test_asm();
    checksum ^= test_classification();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
