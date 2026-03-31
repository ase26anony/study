#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float fvals[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 3.14f,
    1.5f, 2.5f, NAN, 100.0f, -200.0f, 0.001f, -0.001f, 99.9f
};

double dvals[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0, 3.1415926535,
    1.5, 2.5, NAN, 100.0, -200.0, 0.0001, -0.0001, 99.999
};

/* Test scalar comparisons with all relational operators */
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float a = fvals[i];
        float b = fvals[i+8];
        double c = dvals[i];
        double d = dvals[i+8];
        
        /* Force ternary operators for CMOV generation */
        results[idx++] = (a < b) ? 1 : 0;      /* UNLT? */
        results[idx++] = (a > b) ? 2 : 0;      /* UNGT? */
        results[idx++] = (a <= b) ? 3 : 0;     /* UNLE? */
        results[idx++] = (a >= b) ? 4 : 0;     /* UNGE? */
        results[idx++] = (a == b) ? 5 : 0;     /* UNEQ? */
        results[idx++] = (a != b) ? 6 : 0;     /* LTGT? */
        
        results[idx++] = (c < d) ? 7 : 0;
        results[idx++] = (c > d) ? 8 : 0;
        results[idx++] = (c <= d) ? 9 : 0;
        results[idx++] = (c >= d) ? 10 : 0;
        results[idx++] = (c == d) ? 11 : 0;
        results[idx++] = (c != d) ? 12 : 0;
        
        /* Complex control flow with nested conditionals */
        if (isnan(a)) {
            if (!isnan(b)) {
                results[idx++] = 13;
            } else {
                results[idx++] = 14;
            }
        } else if (isinf(a)) {
            results[idx++] = (a > 0) ? 15 : 16;
        } else {
            switch (fpclassify(a)) {
                case FP_NORMAL:
                    results[idx++] = (a < 0) ? 17 : 18;
                    break;
                case FP_SUBNORMAL:
                    results[idx++] = 19;
                    break;
                case FP_ZERO:
                    results[idx++] = (signbit(a)) ? 20 : 21;
                    break;
                default:
                    results[idx++] = 22;
                    break;
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test builtin unordered comparisons */
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float a = fvals[i];
        float b = fvals[15-i];
        double c = dvals[i];
        double d = dvals[15-i];
        
        /* Direct builtin calls that map to condition codes */
        results[idx++] = __builtin_isunordered(a, b) ? 1 : 0;    /* UNORDERED */
        results[idx++] = __builtin_isgreater(a, b) ? 2 : 0;      /* UNLE? (inverted) */
        results[idx++] = __builtin_isless(a, b) ? 3 : 0;         /* UNGE? (inverted) */
        results[idx++] = __builtin_isgreaterequal(a, b) ? 4 : 0; /* UNLT? (inverted) */
        results[idx++] = __builtin_islessequal(a, b) ? 5 : 0;    /* UNGT? (inverted) */
        
        results[idx++] = __builtin_isunordered(c, d) ? 6 : 0;    /* UNORDERED */
        results[idx++] = __builtin_isgreater(c, d) ? 7 : 0;
        results[idx++] = __builtin_isless(c, d) ? 8 : 0;
        
        /* Combined expressions */
        results[idx++] = (__builtin_isunordered(a, b) || a == b) ? 9 : 0;  /* UNEQ? */
        results[idx++] = (!__builtin_isunordered(a, b) && a >= b) ? 10 : 0; /* ORDERED + UNGE */
        
        /* Complex ternary with builtins */
        int val = __builtin_isunordered(a, b) ? 11 : 
                 (__builtin_isgreater(a, b) ? 12 : 
                 (__builtin_isless(a, b) ? 13 : 14));
        results[idx++] = val;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test vector comparisons */
int test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vb = {2.0f, 1.0f, INFINITY, NAN};
    v2df vc = {1.0, NAN};
    v2df vd = {NAN, 1.0};
    
    int results[16];
    int idx = 0;
    
    /* Vector comparisons generate packed RTL */
    v4sf vcmp_lt = va < vb;    /* Should generate UNLT in some form */
    v4sf vcmp_gt = va > vb;    /* UNGT */
    v4sf vcmp_eq = va == vb;   /* UNEQ */
    v4sf vcmp_neq = va != vb;  /* LTGT */
    
    v2df vcmp_d_lt = vc < vd;
    v2df vcmp_d_gt = vc > vd;
    
    /* Reduce vector to scalar mask */
    int mask_lt = 0, mask_gt = 0, mask_eq = 0, mask_neq = 0;
    for (int i = 0; i < 4; i++) {
        mask_lt |= (((int*)&vcmp_lt)[i] != 0) << i;
        mask_gt |= (((int*)&vcmp_gt)[i] != 0) << i;
        mask_eq |= (((int*)&vcmp_eq)[i] != 0) << i;
        mask_neq |= (((int*)&vcmp_neq)[i] != 0) << i;
    }
    
    results[idx++] = mask_lt;
    results[idx++] = mask_gt;
    results[idx++] = mask_eq;
    results[idx++] = mask_neq;
    
    /* Use vector comparison in conditional */
    float sum_f = 0.0f;
    for (int i = 0; i < 4; i++) {
        sum_f += (vcmp_lt[i] != 0.0f) ? va[i] : vb[i];
        sum_f += (vcmp_gt[i] != 0.0f) ? vb[i] : va[i];
    }
    results[idx++] = (int)sum_f;
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test inline assembly with condition codes */
int test_asm(void) {
    unsigned char results[32];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        double a = dvals[i];
        double b = dvals[i+8];
        float c = fvals[i];
        float d = fvals[i+8];
        
        /* Inline assembly that uses condition code names */
        unsigned char byte1, byte2, byte3, byte4;
        
        /* Compare doubles */
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "seta %0\n\t"
            : "=r" (byte1)
            : "x" (a), "x" (b)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "setb %0\n\t"
            : "=r" (byte2)
            : "x" (a), "x" (b)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "setp %0\n\t"      /* UNORDERED */
            : "=r" (byte3)
            : "x" (a), "x" (b)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "setne %0\n\t"     /* LTGT */
            : "=r" (byte4)
            : "x" (a), "x" (b)
            : "cc"
        );
        
        results[idx++] = byte1;
        results[idx++] = byte2;
        results[idx++] = byte3;
        results[idx++] = byte4;
        
        /* Compare floats with ucomiss */
        unsigned char byte5, byte6;
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setbe %0\n\t"     /* UNLE? */
            : "=r" (byte5)
            : "x" (c), "x" (d)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setae %0\n\t"     /* UNGE? */
            : "=r" (byte6)
            : "x" (c), "x" (d)
            : "cc"
        );
        
        results[idx++] = byte5;
        results[idx++] = byte6;
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main function with complex control flow */
int main(void) {
    int total = 0;
    
    /* Loop with varying comparisons */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                total += test_scalar_cmps();
                break;
            case 1:
                total += test_builtins();
                /* Nested loop with goto */
                for (int j = 0; j < 2; j++) {
                    if (j == 1) {
                        total += test_vector();
                        goto after_vector;
                    }
                }
                after_vector:
                break;
            case 2:
                total += test_asm();
                /* Complex if-else chain */
                if (total > 1000) {
                    total -= 500;
                } else if (total > 500) {
                    total += 200;
                } else {
                    for (int k = 0; k < 2; k++) {
                        total += (k % 2 == 0) ? test_scalar_cmps() : test_builtins();
                        if (k == 1) continue;
                        total += 1;
                    }
                }
                break;
            default:
                /* Unreachable but creates more control flow */
                while (total < 0) {
                    total += 100;
                }
                break;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_check = 0.0;
    for (int i = 0; i < 16; i++) {
        if (isnan(fvals[i])) {
            final_check += 1.0;
        } else if (isinf(fvals[i])) {
            final_check += (fvals[i] > 0) ? 2.0 : 3.0;
        } else if (fvals[i] == 0.0f) {
            final_check += (signbit(fvals[i])) ? 4.0 : 5.0;
        } else {
            final_check += (fvals[i] < dvals[i]) ? 6.0 : 7.0;
        }
    }
    
    printf("Result: %d (float check: %f)\n", total, final_check);
    return total != 0 ? 0 : 1;
}
