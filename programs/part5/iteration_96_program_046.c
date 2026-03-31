/* gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* For RTL dumps: gcc -O2 -fdump-rtl-final -fdump-rtl-expand -m32 test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static inline double get_nan(void) {
    return __builtin_nan("");
}

/* Inline assembly with %C constraint to directly trigger condition code printing */
static void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* Direct inline assembly with %C constraint for various condition codes */
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(0) /* 0 = UNORDERED */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(7) /* 7 = ORDERED */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(8) /* 8 = UNEQ */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(9) /* 9 = UNGE */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(10) /* 10 = UNGT */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(11) /* 11 = UNLE */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(12) /* 12 = UNLT */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(13) /* 13 = LTGT */
                  : "cc");
}

/* Function to create complex floating-point comparisons */
static int perform_fp_comparisons(double a, double b) {
    volatile int result = 0;
    
    /* All six standard comparisons - will generate various condition codes */
    result += (a < b) ? 1 : 0;    /* May generate UNLT or LT */
    result += (a <= b) ? 1 : 0;   /* May generate UNLE or LE */
    result += (a > b) ? 1 : 0;    /* May generate UNGT or GT */
    result += (a >= b) ? 1 : 0;   /* May generate UNGE or GE */
    result += (a == b) ? 1 : 0;   /* May generate UNEQ or EQ */
    result += (a != b) ? 1 : 0;   /* May generate LTGT or NE */
    
    /* Ordered/unordered checks */
    result += (a == a && b == b) ? 1 : 0;  /* ORDERED check */
    result += (a != a || b != b) ? 1 : 0;  /* UNORDERED check */
    
    return result;
}

int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = get_nan();
        }
        if (i % 11 == 0) {
            arr2[i] = get_nan();
        }
        if (i % 13 == 0) {
            arr1[i] = get_nan();
            arr2[i] = get_nan();
        }
    }
    
    /* Loop with various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        /* Perform all comparisons - volatile prevents optimization */
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* This will generate condition code checks */
        cc_accumulator += perform_fp_comparisons(a, b);
        
        /* Additional complex expressions to force condition code generation */
        double temp = (a < b) ? a : b;
        temp = (a > b) ? a : b;
        temp = (a == b) ? a : b;
        temp = (a != b) ? a : b;
        
        /* Mixed integer/float conditional moves */
        int int_result = (a < b) ? i : -i;
        int_result = (a > b) ? i : -i;
        int_result = (a == b) ? i : -i;
        int_result = (a != b) ? i : -i;
        
        /* Prevent optimization */
        (void)temp;
        (void)int_result;
    }
    
    /* Directly trigger condition code printing via inline assembly */
    emit_condition_codes();
    
    /* Additional unordered comparison scenarios */
    double nan1 = get_nan();
    double nan2 = get_nan();
    double normal = 3.14159;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    volatile int unordered_check = (nan1 == nan2) ? 1 : 0;
    volatile int ordered_check = (normal == normal) ? 1 : 0;
    
    /* Complex expression with multiple condition codes */
    for (int i = 0; i < 10; i++) {
        double x = (i % 2) ? get_nan() : (double)i;
        double y = (i % 3) ? get_nan() : (double)(i * 2);
        
        /* Force generation of various condition codes */
        if (x < y) cc_accumulator++;
        if (x <= y) cc_accumulator++;
        if (x > y) cc_accumulator++;
        if (x >= y) cc_accumulator++;
        if (x == y) cc_accumulator++;
        if (x != y) cc_accumulator++;
        
        /* Check for ordered/unordered */
        if (x == x && y == y) cc_accumulator++;  /* ORDERED */
        if (x != x || y != y) cc_accumulator++;  /* UNORDERED */
    }
    
    /* Print result to prevent dead code elimination */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Unordered check: %d, Ordered check: %d\n", unordered_check, ordered_check);
    
    return 0;
}
