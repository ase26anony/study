/* 
 * Program to trigger condition code name printing in i386.cc
 * Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -fdump-rtl-final -fno-trapping-math -o cc_test cc_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by mixing NaN and normal values */
#define MIX_WITH_NAN(i) ((i) % 7 == 0 ? __builtin_nan("") : (double)(i) * 1.5)

/* Inline assembly macro using %C constraint to directly trigger condition code printing */
#define EMIT_COND_CODE(cond, var, src) \
    __asm__ volatile ("# BEGIN COND: " #cond "\n\t" \
                      "cmov%C0 %1, %0\n\t" \
                      "# END COND: " #cond \
                      : "+r"(var) \
                      : "r"(src), "i"(cond) \
                      : "cc")

/* Alternative with different instruction pattern */
#define EMIT_FPU_COND(cond, a, b) \
    do { \
        int __result; \
        __asm__ volatile ("# FPU COMPARE: " #cond "\n\t" \
                          "fucomip %%st(1), %%st(0)\n\t" \
                          "set%C0 %0" \
                          : "=r"(__result) \
                          : "t"(a), "u"(b) \
                          : "cc", "st", "st(1)"); \
    } while(0)

int main(void) {
    /* Arrays with mix of NaN and normal values */
    double arr1[256], arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with pattern that includes NaNs */
    for (int i = 0; i < 256; i++) {
        arr1[i] = MIX_WITH_NAN(i);
        arr2[i] = MIX_WITH_NAN(255 - i); /* Different pattern for more unordered cases */
    }
    
    /* Force generation of various condition codes through comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        int temp;
        
        /* Generate UNORDERED and ORDERED codes */
        if (a != a || b != b) { /* Check for NaN */
            /* This comparison will be unordered */
            cc_accumulator += (a < b) ? 1 : 0;  /* May generate UNLT or UNORDERED */
            cc_accumulator += (a > b) ? 2 : 0;  /* May generate UNGT */
        }
        
        /* Generate all standard condition codes */
        temp = (a < b)  ? 1 : 0;  /* May generate LT or UNLT */
        temp = (a <= b) ? 1 : 0;  /* May generate LE or UNLE */
        temp = (a > b)  ? 1 : 0;  /* May generate GT or UNGT */
        temp = (a >= b) ? 1 : 0;  /* May generate GE or UNGE */
        temp = (a == b) ? 1 : 0;  /* May generate EQ or UNEQ */
        temp = (a != b) ? 1 : 0;  /* May generate NE or LTGT */
        
        /* Use volatile to prevent optimization */
        (void)temp;
    }
    
    /* Direct inline assembly to trigger specific condition code printing */
    {
        int x = 42, y = 100;
        
        /* UNORDERED - for NaN comparisons */
        EMIT_COND_CODE(UNORDERED, x, y);
        
        /* ORDERED - for normal comparisons */
        EMIT_COND_CODE(ORDERED, y, x);
        
        /* UNEQ - unordered or equal */
        EMIT_COND_CODE(UNEQ, x, y);
        
        /* UNGE - unordered or greater than or equal */
        EMIT_COND_CODE(UNGE, x, y);
        
        /* UNGT - unordered or greater than */
        EMIT_COND_CODE(UNGT, x, y);
        
        /* UNLE - unordered or less than or equal */
        EMIT_COND_CODE(UNLE, x, y);
        
        /* UNLT - unordered or less than */
        EMIT_COND_CODE(UNLT, x, y);
        
        /* LTGT - less than or greater than (but not equal, not unordered) */
        EMIT_COND_CODE(LTGT, x, y);
    }
    
    /* More complex floating point comparisons that generate condition codes */
    {
        double nan_val = __builtin_nan("");
        double normal_val = 3.14159;
        double inf_val = __builtin_inf();
        
        /* These should generate various condition codes */
        volatile int cmp1 = (nan_val < normal_val);
        volatile int cmp2 = (normal_val >= nan_val);
        volatile int cmp3 = (nan_val == nan_val);
        volatile int cmp4 = (normal_val != normal_val);
        volatile int cmp5 = (inf_val > normal_val);
        volatile int cmp6 = (normal_val <= -inf_val);
        
        /* Mixed comparisons */
        volatile int cmp7 = (nan_val != normal_val) && (normal_val == 3.14159);
        volatile int cmp8 = (normal_val < inf_val) || (nan_val > 0.0);
    }
    
    /* Use ternary operator with FP conditions to potentially generate conditional moves */
    {
        double a = __builtin_nan("");
        double b = 2.0;
        double c = 3.0;
        
        for (int i = 0; i < 100; i++) {
            /* These ternary operations may compile to conditional moves with FP condition codes */
            double result1 = (a < b) ? c : b;
            double result2 = (b >= a) ? a : c;
            double result3 = (a == a) ? b : c;  /* Always false for NaN */
            double result4 = (b != b) ? a : c;  /* Always false for normal number */
            
            /* Prevent optimization */
            cc_accumulator += (int)result1 + (int)result2 + (int)result3 + (int)result4;
        }
    }
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    return 0;
}
