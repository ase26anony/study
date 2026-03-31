/* gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* For RTL dumps: gcc -O2 -fdump-rtl-final -fdump-rtl-expand -m32 test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of various condition codes through floating-point comparisons */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* These comparisons will generate different condition codes */
    volatile int result;
    
    /* UNORDERED: Compare NaN with anything */
    result = (nan_val < normal1);
    result = (normal1 < nan_val);
    
    /* ORDERED: Compare two normal numbers */
    result = (normal1 < normal2);
    result = (normal2 > normal1);
    
    /* UNEQ: Unordered or equal - NaN == NaN is false, but compiler may generate UNEQ */
    result = (nan_val == nan_val);  /* false, but may use UNEQ */
    
    /* UNGE: Unordered or greater than or equal */
    result = (nan_val >= normal1);
    result = (normal1 >= nan_val);
    
    /* UNGT: Unordered or greater than */
    result = (nan_val > normal1);
    result = (normal1 > nan_val);
    
    /* UNLE: Unordered or less than or equal */
    result = (nan_val <= normal1);
    result = (normal1 <= nan_val);
    
    /* UNLT: Unordered or less than */
    result = (nan_val < normal1);
    result = (normal1 < nan_val);
    
    /* LTGT: Less than or greater than (ordered and not equal) */
    result = (normal1 != normal2);  /* Ordered and not equal */
    result = (normal1 < normal2) || (normal1 > normal2);
    
    /* Use volatile to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
}

/* Direct inline assembly to trigger %C format specifier */
void inline_assembly_condition_codes(void) {
    int var = 42;
    int src = 99;
    int result;
    
    /* UNORDERED condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(8)  /* 8 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(13) /* 13 = UNGE */
        : "cc"
    );
    
    /* UNGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(14) /* 14 = UNGT */
        : "cc"
    );
    
    /* UNLE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(15) /* 15 = UNLE */
        : "cc"
    );
    
    /* UNLT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(16) /* 16 = UNLT */
        : "cc"
    );
    
    /* LTGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(12) /* 12 = LTGT */
        : "cc"
    );
    
    result = var;
    asm volatile("" : : "r"(result) : "memory");
}

/* Main test with arrays and loops as requested */
int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int condition_acc = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 0.75;
        
        /* Insert NaN at specific indices */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_inf();
        }
        if (i % 17 == 0) {
            arr2[i] = -__builtin_inf();
        }
    }
    
    /* Perform all six standard floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int temp = 0;
        
        /* Use ternary operators to force potential conditional move generation */
        temp = (a < b) ? (temp | 0x01) : (temp & ~0x01);
        temp = (a <= b) ? (temp | 0x02) : (temp & ~0x02);
        temp = (a > b) ? (temp | 0x04) : (temp & ~0x04);
        temp = (a >= b) ? (temp | 0x08) : (temp & ~0x08);
        temp = (a == b) ? (temp | 0x10) : (temp & ~0x10);
        temp = (a != b) ? (temp | 0x20) : (temp & ~0x20);
        
        condition_acc += temp;
        
        /* Additional complex expressions to generate more condition codes */
        if (!(a < b) && !(a > b) && (a == b)) {
            condition_acc += 0x40;  /* EQ */
        }
        
        if ((a < b) || (a > b)) {
            condition_acc += 0x80;  /* LTGT */
        }
        
        if (!(a >= b)) {
            condition_acc += 0x100; /* LT (inverse of GE) */
        }
    }
    
    /* Call functions to generate more condition codes */
    generate_condition_codes();
    inline_assembly_condition_codes();
    
    /* Additional inline assembly with %C for different conditions */
    {
        int x = 0, y = 1, z;
        
        /* Direct use of %C in different assembly patterns */
        asm volatile (
            "set%C0 %0\n\t"
            : "=r"(z)
            : "i"(0)  /* UNORDERED */
            : "cc"
        );
        
        asm volatile (
            "j%C0 1f\n\t"
            "mov %1, %0\n\t"
            "1:\n\t"
            : "+r"(x)
            : "r"(y), "i"(7)  /* ORDERED */
            : "cc"
        );
    }
    
    /* Print result to prevent optimization */
    printf("Condition accumulator: %d\n", condition_acc);
    
    return 0;
}
