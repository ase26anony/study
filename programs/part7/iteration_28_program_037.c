#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float fvals[16] = {
    1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -0.0f,
    3.14f, -2.71f, 1e10f, -1e10f, 1e-10f, -1e-10f,
    __builtin_nanf(""), __builtin_inff(), -__builtin_inff()
};

double dvals[16] = {
    1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0,
    3.141592653589793, -2.718281828459045, 1e100, -1e100,
    1e-100, -1e-100, __builtin_nan(""), __builtin_inf(), -__builtin_inf()
};

/* Test scalar comparisons with all relational operators */
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    for (int i = 0; i < 8; i++) {
        float fa = fvals[i];
        float fb = fvals[i+1];
        double da = dvals[i];
        double db = dvals[i+1];
        
        /* Generate UNORDERED (unord) - via isnan checks */
        results[idx++] = (isnan(fa) || isnan(fb)) ? 1 : 0;
        results[idx++] = (isnan(da) || isnan(db)) ? 2 : 0;
        
        /* Generate ORDERED (ord) - inverse of unordered */
        results[idx++] = (!isnan(fa) && !isnan(fb)) ? 3 : 0;
        results[idx++] = (!isnan(da) && !isnan(db)) ? 4 : 0;
        
        /* Generate UNEQ (ueq) - unordered or equal */
        results[idx++] = (isnan(fa) || isnan(fb) || fa == fb) ? 5 : 0;
        results[idx++] = (isnan(da) || isnan(db) || da == db) ? 6 : 0;
        
        /* Generate UNGE (nlt) - unordered or greater-or-equal */
        results[idx++] = (isnan(fa) || isnan(fb) || fa >= fb) ? 7 : 0;
        results[idx++] = (isnan(da) || isnan(db) || da >= db) ? 8 : 0;
        
        /* Generate UNGT (nle) - unordered or greater */
        results[idx++] = (isnan(fa) || isnan(fb) || fa > fb) ? 9 : 0;
        results[idx++] = (isnan(da) || isnan(db) || da > db) ? 10 : 0;
        
        /* Generate UNLE (ule) - unordered or less-or-equal */
        results[idx++] = (isnan(fa) || isnan(fb) || fa <= fb) ? 11 : 0;
        results[idx++] = (isnan(da) || isnan(db) || da <= db) ? 12 : 0;
        
        /* Generate UNLT (ult) - unordered or less */
        results[idx++] = (isnan(fa) || isnan(fb) || fa < fb) ? 13 : 0;
        results[idx++] = (isnan(da) || isnan(db) || da < db) ? 14 : 0;
        
        /* Generate LTGT (une) - less or greater (ordered comparison) */
        results[idx++] = (!isnan(fa) && !isnan(fb) && fa != fb) ? 15 : 0;
        results[idx++] = (!isnan(da) && !isnan(db) && da != db) ? 16 : 0;
    }
    
    /* Complex control flow with nested conditionals */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        switch (results[i] % 8) {
            case 0:
                if (fvals[0] < fvals[1]) sum += results[i];
                break;
            case 1:
                if (dvals[0] > dvals[1]) sum += results[i] * 2;
                else if (isunordered(dvals[0], dvals[1])) goto unordered_label;
                break;
            case 2:
                if (fvals[2] <= fvals[3]) continue;
                sum += results[i];
                break;
            case 3:
                if (dvals[2] >= dvals[3]) {
                    sum += results[i];
                    break;
                }
                /* fall through */
            case 4:
                unordered_label:
                sum += results[i] / 2;
                break;
            case 5:
                if (fvals[4] == fvals[5]) sum += results[i] * 3;
                break;
            case 6:
                if (dvals[4] != dvals[5]) sum += results[i] * 4;
                break;
            case 7:
                /* Use fpclassify to generate various condition codes */
                switch (fpclassify(fvals[6])) {
                    case FP_NAN: sum += 100; break;
                    case FP_INFINITE: sum += 200; break;
                    case FP_ZERO: sum += 300; break;
                    case FP_SUBNORMAL: sum += 400; break;
                    case FP_NORMAL: sum += 500; break;
                }
                break;
        }
    }
    
    return sum;
}

/* Test builtin unordered comparison functions */
int test_builtins(void) {
    int sum = 0;
    
    for (int i = 0; i < 8; i++) {
        float fa = fvals[i];
        float fb = fvals[i+8];
        double da = dvals[i];
        double db = dvals[i+8];
        
        /* These builtins directly map to condition codes */
        sum += __builtin_isunordered(fa, fb) ? 1 : 0;
        sum += __builtin_isunordered(da, db) ? 2 : 0;
        
        sum += __builtin_isgreater(fa, fb) ? 4 : 0;
        sum += __builtin_isgreater(da, db) ? 8 : 0;
        
        sum += __builtin_isless(fa, fb) ? 16 : 0;
        sum += __builtin_isless(da, db) ? 32 : 0;
        
        sum += __builtin_isgreaterequal(fa, fb) ? 64 : 0;
        sum += __builtin_isgreaterequal(da, db) ? 128 : 0;
        
        sum += __builtin_islessequal(fa, fb) ? 256 : 0;
        sum += __builtin_islessequal(da, db) ? 512 : 0;
        
        /* Use ternary operators to force CMOV generation */
        int res1 = __builtin_isunordered(fa, fb) ? 1000 : 2000;
        int res2 = !__builtin_isunordered(da, db) ? 3000 : 4000;
        sum += res1 + res2;
        
        /* Nested ternary for complex condition code usage */
        int res3 = (fa < fb) ? 1 : (fa > fb) ? 2 : (fa == fb) ? 3 : 4;
        int res4 = (da < db) ? 5 : (da > db) ? 6 : (da == db) ? 7 : 8;
        sum += res3 * res4;
    }
    
    return sum;
}

/* Test vector/SIMD comparisons */
int test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vb = {2.0f, 1.0f, INFINITY, NAN};
    v2df vda = {1.0, NAN};
    v2df vdb = {NAN, 1.0};
    
    int sum = 0;
    
    /* Vector comparisons generate packed condition codes */
    v4sf vcmp_lt = va < vb;  /* Should generate UNLT/ULT */
    v4sf vcmp_gt = va > vb;  /* Should generate UNGT/UGT */
    v4sf vcmp_eq = va == vb; /* Should generate UNEQ/UEQ */
    v4sf vcmp_ne = va != vb; /* Should generate LTGT/UNE */
    
    v2df vd_cmp_lt = vda < vdb;
    v2df vd_cmp_ge = vda >= vdb; /* Should generate UNGE/UGE */
    v2df vd_cmp_le = vda <= vdb; /* Should generate UNLE/ULE */
    
    /* Reduce vector results to scalar */
    for (int i = 0; i < 4; i++) {
        sum += vcmp_lt[i] ? (1 << i) : 0;
        sum += vcmp_gt[i] ? (2 << i) : 0;
        sum += vcmp_eq[i] ? (4 << i) : 0;
        sum += vcmp_ne[i] ? (8 << i) : 0;
    }
    
    for (int i = 0; i < 2; i++) {
        sum += vd_cmp_lt[i] ? (16 << i) : 0;
        sum += vd_cmp_ge[i] ? (32 << i) : 0;
        sum += vd_cmp_le[i] ? (64 << i) : 0;
    }
    
    /* Loop with vector operations */
    for (int i = 0; i < 4; i++) {
        v4sf tmp = {fvals[i], fvals[i+1], fvals[i+2], fvals[i+3]};
        v4sf cmp_result = tmp < va;
        
        /* Conditional move based on vector comparison */
        int mask = 0;
        for (int j = 0; j < 4; j++) {
            mask |= (cmp_result[j] != 0.0f) ? (1 << j) : 0;
        }
        
        switch (mask & 0xF) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 3: sum += 3; break;
            case 7: sum += 4; break;
            case 15: sum += 5; break;
            default: sum += 6; break;
        }
    }
    
    return sum;
}

/* Test inline assembly with condition code constraints */
int test_asm(void) {
    int sum = 0;
    unsigned char byte_result;
    
    for (int i = 0; i < 8; i++) {
        double a = dvals[i];
        double b = dvals[i+1];
        float fa = fvals[i];
        float fb = fvals[i+1];
        
        /* Test various condition codes in inline assembly */
        
        /* UNORDERED */
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setp %0"
            : "=r" (byte_result)
            : "t" (a), "u" (b)
            : "cc", "st"
        );
        sum += byte_result;
        
        /* ORDERED */
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnp %0"
            : "=r" (byte_result)
            : "t" (a), "u" (b)
            : "cc", "st"
        );
        sum += byte_result * 2;
        
        /* UNEQ (unordered or equal) */
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "sete %0\n\t"
            "setp %%al\n\t"
            "orb %%al, %0"
            : "=r" (byte_result)
            : "t" (fa), "u" (fb)
            : "cc", "al", "st"
        );
        sum += byte_result * 3;
        
        /* UNGE (not less than) */
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setnb %0"
            : "=r" (byte_result)
            : "x" (a), "x" (b)
            : "cc"
        );
        sum += byte_result * 4;
        
        /* UNGT (not less or equal) */
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setnbe %0"
            : "=r" (byte_result)
            : "x" (a), "x" (b)
            : "cc"
        );
        sum += byte_result * 5;
        
        /* UNLE (unordered or less or equal) */
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setbe %0"
            : "=r" (byte_result)
            : "x" (fa), "x" (fb)
            : "cc"
        );
        sum += byte_result * 6;
        
        /* UNLT (unordered or less than) */
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setb %0"
            : "=r" (byte_result)
            : "x" (fa), "x" (fb)
            : "cc"
        );
        sum += byte_result * 7;
        
        /* LTGT (unordered or not equal) */
        __asm__ volatile (
            "fucomip %%st(1), %%st\n\t"
            "setne %0"
            : "=r" (byte_result)
            : "t" (a), "u" (b)
            : "cc", "st"
        );
        sum += byte_result * 8;
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize special values */
    fvals[14] = __builtin_nanf("");
    fvals[15] = __builtin_inff();
    dvals[14] = __builtin_nan("");
    dvals[15] = __builtin_inf();
    
    /* Run all tests */
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    /* Prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
