/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force unordered comparisons by mixing NaN and normal values */
#define MIX_WITH_NAN(i) ((i % 7 == 0) ? __builtin_nan("") : (i * 1.5))

/* Inline assembly macro that directly uses %C format specifier */
#define EMIT_COND_CODE(cond, var, src) \
    __asm__ volatile ("cmov%C0 %1, %0" \
                      : "+r"(var) \
                      : "r"(src), "i"(cond) \
                      : "cc")

/* Another assembly pattern that prints condition codes */
#define PRINT_COND_CODE(cond) \
    __asm__ volatile ("# Condition: %C0" :: "i"(cond))

int main(void) {
    volatile int cc_accumulator = 0;
    int result = 0;
    
    /* Arrays with mixed NaN and normal values */
    double arr1[256], arr2[256];
    
    /* Initialize arrays with pattern that includes NaNs */
    for (int i = 0; i < 256; i++) {
        arr1[i] = MIX_WITH_NAN(i);
        arr2[i] = MIX_WITH_NAN(255 - i);  /* Different pattern for arr2 */
    }
    
    /* Force generation of various condition codes through comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        volatile int cmp_result;
        
        /* Generate UNORDERED and ORDERED codes */
        cmp_result = !(a == a) || !(b == b);  /* Check for NaN */
        cc_accumulator += cmp_result;
        
        /* Generate all standard FP comparison condition codes */
        if (a < b)   cc_accumulator += 1;  /* LT */
        if (a <= b)  cc_accumulator += 2;  /* LE */
        if (a > b)   cc_accumulator += 3;  /* GT */
        if (a >= b)  cc_accumulator += 4;  /* GE */
        if (a == b)  cc_accumulator += 5;  /* EQ */
        if (a != b)  cc_accumulator += 6;  /* NEQ */
        
        /* Use ternary operators to force conditional move generation */
        result = (a < b)  ? result + 1 : result - 1;   /* LT/GE */
        result = (a <= b) ? result + 2 : result - 2;   /* LE/GT */
        result = (a > b)  ? result + 3 : result - 3;   /* GT/LE */
        result = (a >= b) ? result + 4 : result - 4;   /* GE/LT */
        result = (a == b) ? result + 5 : result - 5;   /* EQ/NEQ */
        result = (a != b) ? result + 6 : result - 6;   /* NEQ/EQ */
        
        /* Force unordered comparisons explicitly */
        if (isunordered(a, b)) {
            cc_accumulator += 100;  /* UNORDERED */
        }
        if (!isunordered(a, b)) {
            cc_accumulator += 200;  /* ORDERED */
        }
    }
    
    /* Direct inline assembly to trigger condition code printing */
    /* Each of these should generate different condition code strings */
    
    /* UNORDERED */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_unord);
        EMIT_COND_CODE(__builtin_ia32_unord, result, 0x1234);
    }
    
    /* ORDERED */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_ord);
        EMIT_COND_CODE(__builtin_ia32_ord, result, 0x5678);
    }
    
    /* UNEQ */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_ueq);
        EMIT_COND_CODE(__builtin_ia32_ueq, result, 0x9ABC);
    }
    
    /* UNGE */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_nlt);
        EMIT_COND_CODE(__builtin_ia32_nlt, result, 0xDEF0);
    }
    
    /* UNGT */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_nle);
        EMIT_COND_CODE(__builtin_ia32_nle, result, 0x1111);
    }
    
    /* UNLE */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_ule);
        EMIT_COND_CODE(__builtin_ia32_ule, result, 0x2222);
    }
    
    /* UNLT */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_ult);
        EMIT_COND_CODE(__builtin_ia32_ult, result, 0x3333);
    }
    
    /* LTGT */
    if (__builtin_constant_p(0)) {
        PRINT_COND_CODE(__builtin_ia32_une);
        EMIT_COND_CODE(__builtin_ia32_une, result, 0x4444);
    }
    
    /* Additional complex floating expressions to ensure RTL generation */
    {
        volatile double x = __builtin_nan("");
        volatile double y = 3.14159;
        volatile double z = 2.71828;
        
        /* Complex expression that should generate multiple condition codes */
        int complex_result = (x < y) ? ((y > z) ? 1 : 2) : ((x != z) ? 3 : 4);
        cc_accumulator += complex_result;
        
        /* Mixed integer/float comparison */
        int int_val = 42;
        if (x < int_val) cc_accumulator += 7;
        if (y > int_val) cc_accumulator += 8;
    }
    
    /* Prevent optimization */
    printf("Result: %d, Accumulator: %d\n", result, cc_accumulator);
    
    return result != 0 ? 0 : cc_accumulator != 0 ? 1 : 2;
}
