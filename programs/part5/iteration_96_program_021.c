/* gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* For RTL dumps: gcc -O2 -fdump-rtl-final -fdump-rtl-expand -m32 test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static void unordered_comparisons(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal = 3.14159;
    volatile double zero = 0.0;
    
    /* These comparisons will produce unordered results */
    volatile int res1 = (nan_val < normal);
    volatile int res2 = (nan_val > normal);
    volatile int res3 = (nan_val <= normal);
    volatile int res4 = (nan_val >= normal);
    volatile int res5 = (nan_val == normal);
    volatile int res6 = (nan_val != normal);
    
    /* Ordered comparisons */
    volatile int res7 = (normal < inf_val);
    volatile int res8 = (normal > zero);
    volatile int res9 = (normal <= inf_val);
    volatile int res10 = (normal >= zero);
    volatile int res11 = (normal == 3.14159);
    volatile int res12 = (normal != zero);
    
    /* Mixed comparisons that may generate various condition codes */
    volatile double a = nan_val;
    volatile double b = normal;
    volatile double c = inf_val;
    
    /* Force generation of UNORDERED/ORDERED condition codes */
    if (!(a == a)) { /* NaN check - should be UNORDERED */
        __asm__ volatile("# UNORDERED triggered");
    }
    
    if (b == b) { /* Normal number check - should be ORDERED */
        __asm__ volatile("# ORDERED triggered");
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 0;
    int src = 42;
    
    /* Use various condition codes with %C constraint */
    /* UNORDERED */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(16) /* 16 = UNORDERED */
                     : "cc");
    
    /* ORDERED */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(17) /* 17 = ORDERED */
                     : "cc");
    
    /* UNEQ */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(18) /* 18 = UNEQ */
                     : "cc");
    
    /* UNGE */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(19) /* 19 = UNGE */
                     : "cc");
    
    /* UNGT */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(20) /* 20 = UNGT */
                     : "cc");
    
    /* UNLE */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(21) /* 21 = UNLE */
                     : "cc");
    
    /* UNLT */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(22) /* 22 = UNLT */
                     : "cc");
    
    /* LTGT */
    __asm__ volatile("cmov%C0 %1, %0" 
                     : "+r"(var) 
                     : "r"(src), "i"(23) /* 23 = LTGT */
                     : "cc");
    
    /* Prevent optimization */
    volatile int dummy = var;
    (void)dummy;
}

/* Complex loop with mixed NaN and normal values */
static int complex_loop_comparisons(void) {
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of NaN and normal values */
    for (int i = 0; i < 256; i++) {
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
            arr2[i] = i * 1.5;
        } else if (i % 11 == 0) {
            arr1[i] = i * 2.5;
            arr2[i] = __builtin_nan("");
        } else {
            arr1[i] = i * 1.5;
            arr2[i] = i * 2.5;
        }
    }
    
    /* Perform all six floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Each comparison may generate different condition codes */
        int lt_result = (a < b) ? 1 : 0;
        int le_result = (a <= b) ? 1 : 0;
        int gt_result = (a > b) ? 1 : 0;
        int ge_result = (a >= b) ? 1 : 0;
        int eq_result = (a == b) ? 1 : 0;
        int ne_result = (a != b) ? 1 : 0;
        
        /* Use ternary operators to force potential conditional move generation */
        cc_accumulator += lt_result;
        cc_accumulator += le_result;
        cc_accumulator += gt_result;
        cc_accumulator += ge_result;
        cc_accumulator += eq_result;
        cc_accumulator += ne_result;
        
        /* Force unordered checks */
        if (!(a == a) || !(b == b)) {
            cc_accumulator += 1000; /* Mark unordered cases */
        }
    }
    
    return cc_accumulator;
}

/* Use GCC builtins for floating-point comparisons */
static void use_fp_comparison_builtins(void) {
    double a = __builtin_nan("");
    double b = 1.0;
    double c = 2.0;
    
    /* These builtins may generate condition codes */
    volatile int res1 = __builtin_isgreater(a, b);    /* a > b */
    volatile int res2 = __builtin_isgreaterequal(a, b); /* a >= b */
    volatile int res3 = __builtin_isless(a, b);       /* a < b */
    volatile int res4 = __builtin_islessequal(a, b);  /* a <= b */
    volatile int res5 = __builtin_islessgreater(a, b); /* a < b || a > b */
    volatile int res6 = __builtin_isunordered(a, b);  /* a or b is NaN */
    
    /* Ordered comparisons between normal numbers */
    volatile int res7 = __builtin_isgreater(c, b);
    volatile int res8 = __builtin_isgreaterequal(c, b);
    volatile int res9 = __builtin_isless(b, c);
    volatile int res10 = __builtin_islessequal(b, c);
    volatile int res11 = __builtin_islessgreater(b, c);
    volatile int res12 = __builtin_isunordered(b, c);
    
    (void)res1; (void)res2; (void)res3; (void)res4;
    (void)res5; (void)res6; (void)res7; (void)res8;
    (void)res9; (void)res10; (void)res11; (void)res12;
}

/* Main function that combines all approaches */
int main(void) {
    printf("Testing condition code generation...\n");
    
    /* 1. Unordered comparisons */
    unordered_comparisons();
    
    /* 2. Direct inline assembly with %C constraint */
    direct_cc_asm();
    
    /* 3. Complex loop with mixed NaN/normal values */
    int result = complex_loop_comparisons();
    printf("Accumulator result: %d\n", result);
    
    /* 4. Use GCC floating-point comparison builtins */
    use_fp_comparison_builtins();
    
    /* Additional test: conditional moves based on FP comparisons */
    {
        double x = __builtin_nan("");
        double y = 3.14;
        int target = 0;
        int source = 99;
        
        /* This may generate cmov with condition codes */
        if (x != y) {
            target = source;
        }
        
        /* Force different condition codes */
        if (x < y) {
            target += 1;
        }
        
        if (x <= y) {
            target += 2;
        }
        
        if (x > y) {
            target += 3;
        }
        
        if (x >= y) {
            target += 4;
        }
        
        printf("Final target: %d\n", target);
    }
    
    return 0;
}
