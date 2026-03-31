#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float fvals[] = {0.0f, 1.0f, -1.0f, NAN, INFINITY, -INFINITY, 2.0f, -2.0f};
double dvals[] = {0.0, 1.0, -1.0, NAN, INFINITY, -INFINITY, 2.0, -2.0};
int results[256];
int result_idx = 0;

/* Test scalar comparisons with all relational operators */
void test_scalar_cmps(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    /* Test various float comparisons that should generate different condition codes */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            f1 = fvals[i];
            f2 = fvals[j];
            d1 = dvals[i];
            d2 = dvals[j];
            
            /* Use ternary operator to force CMOV/SET generation */
            results[result_idx++] = (f1 < f2) ? 1 : 0;      /* UNLT or LT */
            results[result_idx++] = (f1 > f2) ? 2 : 0;      /* UNGT or GT */
            results[result_idx++] = (f1 <= f2) ? 3 : 0;     /* UNLE or LE */
            results[result_idx++] = (f1 >= f2) ? 4 : 0;     /* UNGE or GE */
            results[result_idx++] = (f1 == f2) ? 5 : 0;     /* UNEQ or EQ */
            results[result_idx++] = (f1 != f2) ? 6 : 0;     /* LTGT or NE */
            
            /* Double comparisons */
            results[result_idx++] = (d1 < d2) ? 7 : 0;
            results[result_idx++] = (d1 > d2) ? 8 : 0;
            results[result_idx++] = (d1 <= d2) ? 9 : 0;
            results[result_idx++] = (d1 >= d2) ? 10 : 0;
            results[result_idx++] = (d1 == d2) ? 11 : 0;
            results[result_idx++] = (d1 != d2) ? 12 : 0;
            
            /* Complex control flow to keep condition codes live */
            switch (i % 4) {
                case 0:
                    if (f1 < f2) goto label1;
                    if (d1 > d2) break;
                    continue;
                label1:
                    results[result_idx++] = 100 + j;
                    break;
                case 1:
                    while (f1 <= f2) {
                        results[result_idx++] = 200 + j;
                        f1 += 0.5f;
                    }
                    break;
                case 2:
                    for (int k = 0; k < 3 && d1 >= d2; k++) {
                        results[result_idx++] = 300 + k;
                        d1 -= 0.5;
                    }
                    break;
                default:
                    do {
                        results[result_idx++] = 400;
                    } while (f1 == f2 && i < 2);
                    break;
            }
        }
    }
}

/* Test built-in unordered comparison functions */
void test_builtins(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    
    for (int i = 0; i < 8; i++) {
        f1 = fvals[i];
        f2 = fvals[(i + 1) % 8];
        d1 = dvals[i];
        d2 = dvals[(i + 3) % 8];
        
        /* Built-ins that directly map to condition codes */
        results[result_idx++] = __builtin_isunordered(f1, f2) ? 13 : 0;    /* UNORDERED */
        results[result_idx++] = __builtin_isgreater(f1, f2) ? 14 : 0;      /* UNLE? Actually generates GT */
        results[result_idx++] = __builtin_isless(f1, f2) ? 15 : 0;         /* UNGE? Actually generates LT */
        results[result_idx++] = __builtin_isgreaterequal(f1, f2) ? 16 : 0; /* UNLT */
        results[result_idx++] = __builtin_islessequal(f1, f2) ? 17 : 0;    /* UNGT */
        
        /* Double versions */
        results[result_idx++] = __builtin_isunordered(d1, d2) ? 18 : 0;
        results[result_idx++] = __builtin_isgreater(d1, d2) ? 19 : 0;
        results[result_idx++] = __builtin_isless(d1, d2) ? 20 : 0;
        results[result_idx++] = __builtin_isgreaterequal(d1, d2) ? 21 : 0;
        results[result_idx++] = __builtin_islessequal(d1, d2) ? 22 : 0;
        
        /* Test ORDERED condition through fpclassify */
        int c1 = fpclassify(f1);
        int c2 = fpclassify(f2);
        results[result_idx++] = (!isnan(f1) && !isnan(f2)) ? 23 : 0;  /* ORDERED */
        
        /* Mix with isnan/isinf */
        results[result_idx++] = (isnan(f1) || isnan(f2)) ? 24 : 0;    /* UNORDERED alternative */
        results[result_idx++] = (isinf(d1) && isinf(d2)) ? 25 : 0;
    }
}

/* Test vector/SIMD comparisons */
void test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, 4.0f};
    v4sf vb = {1.0f, 3.0f, 3.0f, INFINITY};
    v2df vc = {1.0, NAN};
    v2df vd = {2.0, INFINITY};
    
    /* Vector comparisons generate packed comparisons */
    v4sf vcmp1 = va < vb;    /* Should generate UNLT/LT per element */
    v4sf vcmp2 = va > vb;    /* UNGT/GT */
    v4sf vcmp3 = va <= vb;   /* UNLE/LE */
    v4sf vcmp4 = va >= vb;   /* UNGE/GE */
    v4sf vcmp5 = va == vb;   /* UNEQ/EQ */
    v4sf vcmp6 = va != vb;   /* LTGT/NE */
    
    v2df vcmp7 = vc < vd;
    v2df vcmp8 = vc > vd;
    
    /* Reduce vector to scalar mask - forces condition code materialization */
    int mask1 = 0, mask2 = 0;
    for (int i = 0; i < 4; i++) {
        if (vcmp1[i]) mask1 |= (1 << i);
        if (vcmp3[i]) mask2 |= (1 << i);
    }
    results[result_idx++] = mask1;
    results[result_idx++] = mask2;
    
    /* Complex control flow with vector results */
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                if (vcmp1[i]) results[result_idx++] = 500 + i;
                break;
            case 1:
                if (vcmp2[i]) results[result_idx++] = 600 + i;
                else if (vcmp3[i]) results[result_idx++] = 700 + i;
                break;
            case 2:
                results[result_idx++] = vcmp4[i] ? 800 : 0;
                break;
            case 3:
                results[result_idx++] = vcmp5[i] ? 900 : 0;
                break;
        }
    }
}

/* Test inline assembly with condition code constraints */
void test_asm(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile float c = INFINITY;
    volatile float d = 2.0f;
    
    uint8_t byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    
    /* Inline assembly that uses condition code names */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r"(byte1) : "x"(a), "x"(b) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(byte2) : "x"(c), "x"(d) : "cc");
    
    /* Test various condition codes */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %0"    /* UNORDERED */
        : "=r"(byte3) : "x"(a), "x"(b) : "cc");
    
    __asm__ volatile (
        "comiss %1, %2\n\t"
        "setnp %0"   /* ORDERED */
        : "=r"(byte4) : "x"(c), "x"(d) : "cc");
    
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"    /* UNEQ/EQ */
        : "=r"(byte5) : "x"(a), "x"(a) : "cc");
    
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"   /* LTGT/NE */
        : "=r"(byte6) : "x"(c), "x"(d) : "cc");
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setae %0"   /* UNLT/GE */
        : "=r"(byte7) : "x"(a), "x"(b) : "cc");
    
    __asm__ volatile (
        "comiss %1, %2\n\t"
        "setbe %0"   /* UNGT/LE */
        : "=r"(byte8) : "x"(c), "x"(d) : "cc");
    
    results[result_idx++] = byte1;
    results[result_idx++] = byte2;
    results[result_idx++] = byte3;
    results[result_idx++] = byte4;
    results[result_idx++] = byte5;
    results[result_idx++] = byte6;
    results[result_idx++] = byte7;
    results[result_idx++] = byte8;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize results array */
    for (int i = 0; i < 256; i++) {
        results[i] = 0;
    }
    
    /* Run all tests */
    test_scalar_cmps();
    test_builtins();
    test_vector();
    test_asm();
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < result_idx && i < 256; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", result_idx);
    
    return 0;
}
