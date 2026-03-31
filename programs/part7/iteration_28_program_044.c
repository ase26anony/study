#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 32

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global arrays with special floating-point values */
float fvals[ARRAY_SIZE];
double dvals[ARRAY_SIZE];
int results[ARRAY_SIZE * 4];
volatile int checksum = 0;

/* Initialize arrays with normal and special values */
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fvals[i] = (i % 8 == 0) ? NAN : 
                   (i % 8 == 1) ? INFINITY : 
                   (i % 8 == 2) ? -INFINITY : 
                   (i % 8 == 3) ? 0.0f : 
                   (i % 8 == 4) ? -0.0f : 
                   (float)(i * 1.5);
        
        dvals[i] = (i % 7 == 0) ? NAN : 
                   (i % 7 == 1) ? INFINITY : 
                   (i % 7 == 2) ? -INFINITY : 
                   (i % 7 == 3) ? 0.0 : 
                   (i % 7 == 4) ? -0.0 : 
                   (double)(i * 2.3);
    }
}

/* Test scalar comparisons with all relational operators */
void test_scalar_cmps(void) {
    int idx = 0;
    
    /* Complex control flow with nested if-else and switch */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        float f1 = fvals[i];
        float f2 = fvals[i + 1];
        double d1 = dvals[i];
        double d2 = dvals[i + 1];
        
        /* UNORDERED cases (NaN comparisons) */
        results[idx++] = (f1 != f1) ? i : -i;  /* isnan check */
        results[idx++] = (d1 != d1) ? i : -i;  /* isnan check */
        
        /* ORDERED cases */
        results[idx++] = (f1 == f1) ? i : -i;  /* ordered check */
        results[idx++] = (d1 == d1) ? i : -i;  /* ordered check */
        
        /* UNEQ (unordered or equal) */
        results[idx++] = (f1 != f2) ? 0 : 1;
        results[idx++] = (d1 != d2) ? 0 : 1;
        
        /* UNGE (unordered or greater or equal) */
        results[idx++] = (f1 < f2) ? 0 : 1;
        results[idx++] = (d1 < d2) ? 0 : 1;
        
        /* UNGT (unordered or greater) */
        results[idx++] = (f1 <= f2) ? 0 : 1;
        results[idx++] = (d1 <= d2) ? 0 : 1;
        
        /* UNLE (unordered or less or equal) */
        results[idx++] = (f1 > f2) ? 0 : 1;
        results[idx++] = (d1 > d2) ? 0 : 1;
        
        /* UNLT (unordered or less) */
        results[idx++] = (f1 >= f2) ? 0 : 1;
        results[idx++] = (d1 >= d2) ? 0 : 1;
        
        /* LTGT (less, greater, or unordered - but not equal) */
        results[idx++] = (f1 == f2) ? 0 : 1;
        results[idx++] = (d1 == d2) ? 0 : 1;
        
        /* Switch statement to force different code paths */
        switch (i % 8) {
            case 0:
                /* More unordered checks */
                results[idx++] = isnan(f1) ? 100 : -100;
                break;
            case 1:
                results[idx++] = isinf(d1) ? 200 : -200;
                break;
            case 2:
                /* Classification functions */
                results[idx++] = (fpclassify(f1) == FP_NAN) ? 1 : 0;
                break;
            case 3:
                results[idx++] = (fpclassify(d1) == FP_INFINITE) ? 1 : 0;
                break;
            case 4:
                /* Mixed float/double comparisons */
                results[idx++] = ((double)f1 > d2) ? 300 : -300;
                break;
            case 5:
                results[idx++] = (f1 < (float)d2) ? 400 : -400;
                break;
            case 6:
                /* Goto to create interesting control flow */
                if (f1 != f1) goto unordered_label;
                results[idx++] = 500;
                break;
            case 7:
unordered_label:
                results[idx++] = 600;
                break;
        }
    }
}

/* Test built-in unordered comparison functions */
void test_builtins(void) {
    int idx = ARRAY_SIZE * 2;
    
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        float f1 = fvals[i];
        float f2 = fvals[i + 1];
        double d1 = dvals[i];
        double d2 = dvals[i + 1];
        
        /* __builtin_isunordered - directly maps to UNORDERED */
        results[idx++] = __builtin_isunordered(f1, f2) ? 1 : 0;
        results[idx++] = __builtin_isunordered(d1, d2) ? 1 : 0;
        
        /* __builtin_isgreater - UNGT */
        results[idx++] = __builtin_isgreater(f1, f2) ? 2 : 0;
        results[idx++] = __builtin_isgreater(d1, d2) ? 2 : 0;
        
        /* __builtin_isgreaterequal - UNGE */
        results[idx++] = __builtin_isgreaterequal(f1, f2) ? 3 : 0;
        results[idx++] = __builtin_isgreaterequal(d1, d2) ? 3 : 0;
        
        /* __builtin_isless - UNLT */
        results[idx++] = __builtin_isless(f1, f2) ? 4 : 0;
        results[idx++] = __builtin_isless(d1, d2) ? 4 : 0;
        
        /* __builtin_islessequal - UNLE */
        results[idx++] = __builtin_islessequal(f1, f2) ? 5 : 0;
        results[idx++] = __builtin_islessequal(d1, d2) ? 5 : 0;
        
        /* __builtin_islessgreater - LTGT */
        results[idx++] = __builtin_islessgreater(f1, f2) ? 6 : 0;
        results[idx++] = __builtin_islessgreater(d1, d2) ? 6 : 0;
        
        /* Complex ternary expressions forcing CMOV generation */
        int* ptr1 = (i % 3 == 0) ? &results[0] : &results[1];
        int* ptr2 = __builtin_isunordered(f1, f2) ? &results[2] : &results[3];
        results[idx++] = (int)(ptr1 - ptr2);
    }
}

/* Test vector/SIMD comparisons */
void test_vector(void) {
    v4sf vec1, vec2;
    v2df dvec1, dvec2;
    
    /* Initialize vectors */
    for (int i = 0; i < 4; i++) {
        vec1[i] = fvals[i];
        vec2[i] = fvals[i + 4];
    }
    for (int i = 0; i < 2; i++) {
        dvec1[i] = dvals[i];
        dvec2[i] = dvals[i + 2];
    }
    
    /* Vector comparisons - these generate packed comparisons */
    v4sf cmp_result_f = (vec1 < vec2);
    v2df cmp_result_d = (dvec1 > dvec2);
    
    /* Reduce to scalar mask */
    int mask_f = 0, mask_d = 0;
    for (int i = 0; i < 4; i++) {
        mask_f |= ((int)cmp_result_f[i] != 0) << i;
    }
    for (int i = 0; i < 2; i++) {
        mask_d |= ((int)cmp_result_d[i] != 0) << i;
    }
    
    results[ARRAY_SIZE * 3] = mask_f;
    results[ARRAY_SIZE * 3 + 1] = mask_d;
    
    /* More vector operations */
    v4sf cmp_unord = (vec1 != vec1) | (vec2 != vec2);
    int unord_mask = 0;
    for (int i = 0; i < 4; i++) {
        unord_mask |= ((int)cmp_unord[i] != 0) << i;
    }
    results[ARRAY_SIZE * 3 + 2] = unord_mask;
}

/* Test inline assembly with condition code constraints */
void test_asm(void) {
    int idx = ARRAY_SIZE * 3 + 4;
    
    for (int i = 0; i < 8; i++) {
        float f1 = fvals[i];
        float f2 = fvals[i + 8];
        double d1 = dvals[i];
        double d2 = dvals[i + 8];
        
        unsigned char byte1, byte2, byte3, byte4;
        
        /* Test various condition codes via inline assembly */
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "seta %0"
            : "=r" (byte1)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setb %0"
            : "=r" (byte2)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        /* UNORDERED test */
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setp %0"
            : "=r" (byte3)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        
        /* ORDERED test */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnp %0"
            : "=r" (byte4)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        
        results[idx++] = byte1;
        results[idx++] = byte2;
        results[idx++] = byte3;
        results[idx++] = byte4;
        
        /* More assembly with different condition codes */
        int result;
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setg %0"
            : "=r" (result)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        results[idx++] = result;
        
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setl %0"
            : "=r" (result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = result;
        
        /* Test NE (not equal) and EQ (equal) */
        __asm__ volatile (
            "ucomiss %1, %2\n\t"
            "setne %0"
            : "=r" (result)
            : "x" (f1), "x" (f2)
            : "cc"
        );
        results[idx++] = result;
        
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "sete %0"
            : "=r" (result)
            : "x" (d1), "x" (d2)
            : "cc"
        );
        results[idx++] = result;
    }
}

int main(void) {
    init_arrays();
    
    /* Call all test functions */
    test_scalar_cmps();
    test_builtins();
    test_vector();
    test_asm();
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
        checksum += results[i];
    }
    
    /* Use checksum in output */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
