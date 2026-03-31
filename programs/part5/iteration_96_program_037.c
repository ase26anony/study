/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of various condition codes through floating-point comparisons */
void generate_condition_codes(void) {
    volatile double a = __builtin_nan("");
    volatile double b = 3.14159;
    volatile double c = 2.71828;
    volatile double d = __builtin_nan("");
    
    int result = 0;
    
    /* Generate UNORDERED condition (NaN comparison) */
    if (a != a) {  /* Always true for NaN */
        result |= 1;
    }
    
    /* Generate ORDERED condition */
    if (b == b) {  /* Always true for non-NaN */
        result |= 2;
    }
    
    /* Generate UNEQ (unordered or equal) */
    if (!(a > b)) {  /* NaN > b is false, so !false is true */
        result |= 4;
    }
    
    /* Generate UNGE (not less than) */
    if (!(a < b)) {  /* NaN < b is false */
        result |= 8;
    }
    
    /* Generate UNGT (not less than or equal) */
    if (!(a <= b)) {  /* NaN <= b is false */
        result |= 16;
    }
    
    /* Generate UNLE (unordered or less than or equal) */
    if (!(b > a)) {  /* b > NaN is false */
        result |= 32;
    }
    
    /* Generate UNLT (unordered or less than) */
    if (!(b >= a)) {  /* b >= NaN is false */
        result |= 64;
    }
    
    /* Generate LTGT (less than or greater than) */
    if (b != c) {  /* Normal inequality */
        result |= 128;
    }
    
    /* Prevent optimization */
    asm volatile("" : "+r" (result));
}

/* Direct inline assembly to trigger %C format specifier */
void direct_asm_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16)  /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(17)  /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18)  /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19)  /* 19 = UNGE */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20)  /* 20 = UNGT */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(21)  /* 21 = UNLE */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(22)  /* 22 = UNLT */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23)  /* 23 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile("" : "+r" (var));
}

/* Array-based approach with mixed NaN and normal values */
int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int condition_acc = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
        /* Insert NaN at specific indices */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
    
    /* Perform various comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int temp = 0;
        
        /* All six standard comparisons */
        temp = (a < b) ? 1 : 0;
        condition_acc += temp;
        
        temp = (a <= b) ? 2 : 0;
        condition_acc += temp;
        
        temp = (a > b) ? 4 : 0;
        condition_acc += temp;
        
        temp = (a >= b) ? 8 : 0;
        condition_acc += temp;
        
        temp = (a == b) ? 16 : 0;
        condition_acc += temp;
        
        temp = (a != b) ? 32 : 0;
        condition_acc += temp;
        
        /* Ternary operators that might generate conditional moves */
        int cmp_result;
        cmp_result = (a < b) ? 100 : 200;
        condition_acc += cmp_result;
        
        cmp_result = (a != b) ? 300 : 400;
        condition_acc += cmp_result;
        
        /* Mixed integer/float comparison */
        int int_val = i;
        cmp_result = (a < int_val) ? 500 : 600;
        condition_acc += cmp_result;
    }
    
    /* Call functions that generate specific condition codes */
    generate_condition_codes();
    direct_asm_condition_codes();
    
    /* Additional unordered comparisons */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    volatile double normal = 42.0;
    
    /* These should generate UNORDERED-related codes */
    if (nan1 == nan2) {
        condition_acc += 1000;
    }
    
    if (nan1 != normal) {
        condition_acc += 2000;
    }
    
    if (!(nan1 < normal)) {
        condition_acc += 3000;
    }
    
    if (!(normal > nan1)) {
        condition_acc += 4000;
    }
    
    /* Print result to prevent optimization */
    printf("Condition accumulator: %d\n", condition_acc);
    
    return 0;
}
