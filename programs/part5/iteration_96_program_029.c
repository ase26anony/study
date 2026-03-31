/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of various x86 condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static double arr1[256];
static double arr2[256];

/* Initialize arrays with mixed values */
void init_arrays(void) {
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
}

/* Direct inline assembly with %C constraint to force condition code printing */
void direct_cc_asm(void) {
    int var = 42;
    int src = 99;
    
    /* UNORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(0)  /* 0 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(7)  /* 7 = ORDERED */
                  : "cc");
    
    /* UNEQ condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(6)  /* 6 = UNEQ */
                  : "cc");
    
    /* UNGE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(5)  /* 5 = UNGE */
                  : "cc");
    
    /* UNGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(4)  /* 4 = UNGT */
                  : "cc");
    
    /* UNLE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(3)  /* 3 = UNLE */
                  : "cc");
    
    /* UNLT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(2)  /* 2 = UNLT */
                  : "cc");
    
    /* LTGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(1)  /* 1 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Complex floating-point comparisons to generate condition codes */
int complex_fp_comparisons(void) {
    volatile int cc_accumulator = 0;
    int result = 0;
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* All six standard FP comparisons */
        int lt = (a < b) ? 1 : 0;
        int le = (a <= b) ? 1 : 0;
        int gt = (a > b) ? 1 : 0;
        int ge = (a >= b) ? 1 : 0;
        int eq = (a == b) ? 1 : 0;
        int ne = (a != b) ? 1 : 0;
        
        /* Force conditional move generation through ternary operators */
        result = (a < b) ? (result + 1) : (result - 1);
        result = (a <= b) ? (result * 2) : (result / 2);
        result = (a > b) ? (result | 0xFF) : (result & 0xFF00);
        result = (a >= b) ? (result ^ 0xAAAA) : (result ^ 0x5555);
        result = (a == b) ? (result << 2) : (result >> 2);
        result = (a != b) ? (result + i) : (result - i);
        
        /* Accumulate comparison results (volatile prevents elimination) */
        cc_accumulator += lt + le + gt + ge + eq + ne;
        
        /* Mixed integer/float conditional moves */
        int int_result = 0;
        double fp_result = 0.0;
        
        /* These may generate cmovCC instructions */
        int_result = (a < b) ? 100 : 200;
        fp_result = (a <= b) ? 3.14 : 2.71;
        int_result = (a > b) ? int_result + 50 : int_result - 50;
        fp_result = (a >= b) ? fp_result * 2.0 : fp_result / 2.0;
        int_result = (a == b) ? 0xDEAD : 0xBEEF;
        fp_result = (a != b) ? __builtin_nan("") : 0.0;
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "r"(int_result), "r"(fp_result));
    }
    
    return cc_accumulator + result;
}

/* Additional unordered comparison scenarios */
void unordered_scenarios(void) {
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nan("0x123");
    double inf = __builtin_inf();
    double normal = 42.0;
    
    volatile int test_results[8] = {0};
    
    /* Generate various unordered conditions */
    test_results[0] = (nan1 < normal) ? 1 : 0;    /* UNORDERED/UNLT */
    test_results[1] = (nan1 <= normal) ? 1 : 0;   /* UNORDERED/UNLE */
    test_results[2] = (nan1 > normal) ? 1 : 0;    /* UNORDERED/UNGT */
    test_results[3] = (nan1 >= normal) ? 1 : 0;   /* UNORDERED/UNGE */
    test_results[4] = (nan1 == normal) ? 1 : 0;   /* UNORDERED/UNEQ */
    test_results[5] = (nan1 != normal) ? 1 : 0;   /* UNORDERED/LTGT */
    test_results[6] = (nan1 == nan2) ? 1 : 0;     /* UNORDERED/UNEQ */
    test_results[7] = (normal == normal) ? 1 : 0; /* ORDERED/EQ */
    
    /* Control flow depending on unordered comparisons */
    if (nan1 < normal) {
        test_results[0] += 100;
    }
    if (!(nan1 >= normal)) {
        test_results[3] += 200;
    }
    if (normal == normal) {
        test_results[7] += 300;
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(test_results[0]), "r"(test_results[7]));
}

/* Main function with forced condition code generation */
int main(void) {
    init_arrays();
    
    /* Direct assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Complex FP comparisons */
    int cc_result = complex_fp_comparisons();
    
    /* Additional unordered scenarios */
    unordered_scenarios();
    
    /* Print result to prevent optimization */
    printf("Condition code test result: %d\n", cc_result);
    
    /* Additional inline assembly with %C for different conditions */
    int x = 0, y = 1;
    
    /* Test with builtin constant predicate */
    if (__builtin_constant_p(0)) {
        asm volatile ("set%C0 %0" : "=r"(x) : "i"(0) : "cc");  /* UNORDERED */
    }
    if (__builtin_constant_p(7)) {
        asm volatile ("set%C0 %0" : "=r"(y) : "i"(7) : "cc");  /* ORDERED */
    }
    
    printf("Final values: x=%d, y=%d\n", x, y);
    
    return 0;
}
