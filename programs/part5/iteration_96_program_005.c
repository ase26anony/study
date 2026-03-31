/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static inline int unordered_compare(double a, double b, int op) {
    volatile double va = a;
    volatile double vb = b;
    
    switch (op) {
        case 0: return (va < vb) ? 1 : 0;    /* LT */
        case 1: return (va <= vb) ? 1 : 0;   /* LE */
        case 2: return (va > vb) ? 1 : 0;    /* GT */
        case 3: return (va >= vb) ? 1 : 0;   /* GE */
        case 4: return (va == vb) ? 1 : 0;   /* EQ */
        case 5: return (va != vb) ? 1 : 0;   /* NEQ */
        default: return 0;
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16)  /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(17)  /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18)  /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19)  /* 19 = UNGE */
                  : "cc");
    
    /* UNGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20)  /* 20 = UNGT */
                  : "cc");
    
    /* UNLE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(21)  /* 21 = UNLE */
                  : "cc");
    
    /* UNLT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(22)  /* 22 = UNLT */
                  : "cc");
    
    /* LTGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23)  /* 23 = LTGT */
                  : "cc");
}

/* Mixed floating-point comparisons that generate various condition codes */
static int perform_mixed_comparisons(double a, double b) {
    int result = 0;
    
    /* These ternary operations may generate conditional moves */
    result += (a < b) ? 1 : 0;    /* May generate UNLT/UNORDERED */
    result += (a <= b) ? 2 : 0;   /* May generate UNLE/UNORDERED */
    result += (a > b) ? 4 : 0;    /* May generate UNGT/UNORDERED */
    result += (a >= b) ? 8 : 0;   /* May generate UNGE/UNORDERED */
    result += (a == b) ? 16 : 0;  /* May generate UNEQ/UNORDERED */
    result += (a != b) ? 32 : 0;  /* May generate LTGT/UNORDERED */
    
    return result;
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE];
    double arr2[SIZE];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("");
            arr2[i] = __builtin_nan("");
        }
    }
    
    volatile int cc_accumulator = 0;
    
    /* Loop performing various floating-point comparisons */
    for (int i = 0; i < SIZE; i++) {
        /* Force unordered comparisons with NaN values */
        cc_accumulator += unordered_compare(arr1[i], arr2[i], i % 6);
        
        /* Mixed comparisons that may generate different condition codes */
        cc_accumulator += perform_mixed_comparisons(arr1[i], arr2[i]);
        
        /* Direct unordered comparison that may generate UNORDERED/ORDERED codes */
        volatile int is_unordered = (arr1[i] != arr1[i]) || (arr2[i] != arr2[i]);
        cc_accumulator += is_unordered ? 1 : 0;
        
        /* Ordered comparison */
        volatile int is_ordered = (arr1[i] == arr1[i]) && (arr2[i] == arr2[i]);
        cc_accumulator += is_ordered ? 1 : 0;
    }
    
    /* Trigger direct condition code printing via inline assembly */
    emit_condition_codes();
    
    /* Additional floating-point control flow to generate condition codes */
    double test_val = __builtin_nan("");
    double normal_val = 3.14159;
    
    /* These if-else chains force generation of conditional jumps with CC */
    if (test_val < normal_val) {
        cc_accumulator += 1000;
    } else if (test_val <= normal_val) {
        cc_accumulator += 2000;
    } else if (test_val > normal_val) {
        cc_accumulator += 3000;
    } else if (test_val >= normal_val) {
        cc_accumulator += 4000;
    } else if (test_val == normal_val) {
        cc_accumulator += 5000;
    } else if (test_val != normal_val) {
        cc_accumulator += 6000;  /* This should trigger with NaN */
    }
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* More complex expression that may generate LTGT */
    double x = 1.0;
    double y = 2.0;
    double z = __builtin_nan("");
    
    /* This comparison may generate LTGT condition code */
    volatile int ltgt_test = (x < y) && (x > y);
    cc_accumulator += ltgt_test ? 1 : 0;
    
    /* UNEQ test: equal or unordered */
    volatile int uneq_test = !(x < y) && !(x > y);
    cc_accumulator += uneq_test ? 1 : 0;
    
    printf("Final result: %d\n", cc_accumulator);
    
    return 0;
}
