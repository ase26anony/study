/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of specific condition codes */
#define FORCE_COND_CODE(cond, a, b) \
    do { \
        int result; \
        __asm__ volatile ( \
            "comisd %2, %1\n\t" \
            "set%C0 %0" \
            : "=r"(result) \
            : "x"(a), "x"(b), "i"(cond) \
            : "cc" \
        ); \
        cond_acc += result; \
    } while(0)

/* Direct inline assembly with %C constraint */
#define EMIT_COND_MOVE(cond, src, dest) \
    do { \
        int tmp = (dest); \
        __asm__ volatile ( \
            "test $1, %1\n\t" \
            "cmov%C0 %2, %0" \
            : "+r"(tmp) \
            : "r"((int)(src)), "r"((dest) + 1), "i"(cond) \
            : "cc" \
        ); \
        cond_acc += tmp; \
    } while(0)

int main(void) {
    volatile int cond_acc = 0;
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("0xdead");
            arr2[i] = __builtin_nan("0xbeef");
        }
    }
    
    /* Perform all floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int cmp_result;
        
        /* Generate UNORDERED condition (unord) */
        if (isunordered(a, b)) {
            cmp_result = 1;
            cond_acc += cmp_result;
        }
        
        /* Generate ORDERED condition (ord) */
        if (isordered(a, b)) {
            cmp_result = 2;
            cond_acc += cmp_result;
        }
        
        /* Generate UNEQ condition (ueq) */
        if (!(a < b) && !(a > b)) {
            cmp_result = 3;
            cond_acc += cmp_result;
        }
        
        /* Generate UNGE condition (nlt) */
        if (!(a < b)) {
            cmp_result = 4;
            cond_acc += cmp_result;
        }
        
        /* Generate UNGT condition (nle) */
        if (!(a <= b)) {
            cmp_result = 5;
            cond_acc += cmp_result;
        }
        
        /* Generate UNLE condition (ule) */
        if (!(a > b)) {
            cmp_result = 6;
            cond_acc += cmp_result;
        }
        
        /* Generate UNLT condition (ult) */
        if (a < b) {
            cmp_result = 7;
            cond_acc += cmp_result;
        }
        
        /* Generate LTGT condition (une) */
        if (a != b) {
            cmp_result = 8;
            cond_acc += cmp_result;
        }
        
        /* Use ternary operators to force conditional move generation */
        int var1 = (a < b) ? 100 : 200;
        int var2 = (a <= b) ? 300 : 400;
        int var3 = (a > b) ? 500 : 600;
        int var4 = (a >= b) ? 700 : 800;
        int var5 = (a == b) ? 900 : 1000;
        int var6 = (a != b) ? 1100 : 1200;
        
        cond_acc += var1 + var2 + var3 + var4 + var5 + var6;
    }
    
    /* Direct inline assembly to force condition code printing */
    double x = __builtin_nan("");
    double y = 3.14159;
    
    /* Force UNORDERED condition code printing */
    FORCE_COND_CODE(UNORDERED, x, y);
    
    /* Force ORDERED condition code printing */
    FORCE_COND_CODE(ORDERED, y, x);
    
    /* Force UNEQ condition code printing */
    FORCE_COND_CODE(UNEQ, x, x);
    
    /* Force UNGE condition code printing */
    FORCE_COND_CODE(UNGE, y, x);
    
    /* Force UNGT condition code printing */
    FORCE_COND_CODE(UNGT, y + 1.0, y);
    
    /* Force UNLE condition code printing */
    FORCE_COND_CODE(UNLE, y, y + 1.0);
    
    /* Force UNLT condition code printing */
    FORCE_COND_CODE(UNLT, y - 1.0, y);
    
    /* Force LTGT condition code printing */
    FORCE_COND_CODE(LTGT, y, y + 2.0);
    
    /* Conditional move with various condition codes */
    EMIT_COND_MOVE(UNORDERED, 1, 100);
    EMIT_COND_MOVE(ORDERED, 1, 200);
    EMIT_COND_MOVE(UNEQ, 1, 300);
    EMIT_COND_MOVE(UNGE, 1, 400);
    EMIT_COND_MOVE(UNGT, 1, 500);
    EMIT_COND_MOVE(UNLE, 1, 600);
    EMIT_COND_MOVE(UNLT, 1, 700);
    EMIT_COND_MOVE(LTGT, 1, 800);
    
    /* Use builtin to check if we can generate constant condition codes */
    if (__builtin_constant_p(cond_acc)) {
        /* This branch forces consideration of constant condition codes */
        __asm__ volatile ("nop" : : "i"(UNORDERED), "i"(ORDERED));
    }
    
    /* Complex floating-point expression that survives optimization */
    volatile double complex_expr = (arr1[0] < arr2[0]) ? 
                                   (arr1[1] <= arr2[1] ? 1.0 : 2.0) :
                                   (arr1[2] > arr2[2] ? 3.0 : 4.0);
    
    cond_acc += (int)complex_expr;
    
    printf("Condition accumulator: %d\n", cond_acc);
    return cond_acc != 0;
}
