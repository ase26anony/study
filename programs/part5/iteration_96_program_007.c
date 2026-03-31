/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of all x86 condition codes from floating-point comparisons */
int main(void) {
    /* Arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with pattern: normal values and NaN at specific indices */
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i % 7 == 0) ? __builtin_nan("") : (i * 1.5);
        arr2[i] = (i % 11 == 0) ? __builtin_nan("") : (i * 2.3 + 1.0);
    }
    
    /* Force generation of all condition codes through floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        int result;
        
        /* Generate UNORDERED/ORDERED codes with direct NaN comparisons */
        volatile int is_unordered = (a != a) || (b != b);
        cc_accumulator += is_unordered;
        
        /* Generate full range of condition codes using all comparison operators */
        /* Each comparison may produce different condition codes for NaN values */
        
        /* Less than - may generate UNLT or UNORDERED */
        result = (a < b) ? 1 : 0;
        cc_accumulator += result;
        
        /* Less than or equal - may generate UNLE or UNORDERED */
        result = (a <= b) ? 2 : 0;
        cc_accumulator += result;
        
        /* Greater than - may generate UNGT or UNORDERED */
        result = (a > b) ? 3 : 0;
        cc_accumulator += result;
        
        /* Greater than or equal - may generate UNGE or UNORDERED */
        result = (a >= b) ? 4 : 0;
        cc_accumulator += result;
        
        /* Equal - may generate UNEQ or UNORDERED */
        result = (a == b) ? 5 : 0;
        cc_accumulator += result;
        
        /* Not equal - may generate LTGT or UNORDERED */
        result = (a != b) ? 6 : 0;
        cc_accumulator += result;
    }
    
    /* Direct inline assembly to trigger condition code name printing */
    /* Using %C format specifier to force condition code name output */
    
    /* UNORDERED condition code */
    {
        int var1 = 42, src1 = 99;
        asm volatile ("# UNORDERED test begin\n\t"
                      "cmov%C0 %1, %0\n\t"
                      "# UNORDERED test end"
                      : "+r"(var1)
                      : "r"(src1), "i"(0)  /* 0 = UNORDERED */
                      : "cc");
        cc_accumulator += var1;
    }
    
    /* ORDERED condition code */
    {
        int var2 = 100, src2 = 200;
        asm volatile ("# ORDERED test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var2)
                      : "r"(src2), "i"(7)  /* 7 = ORDERED */
                      : "cc");
        cc_accumulator += var2;
    }
    
    /* UNEQ condition code */
    {
        int var3 = 300, src3 = 400;
        asm volatile ("# UNEQ test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var3)
                      : "r"(src3), "i"(1)  /* 1 = UNEQ */
                      : "cc");
        cc_accumulator += var3;
    }
    
    /* UNGE condition code */
    {
        int var4 = 500, src4 = 600;
        asm volatile ("# UNGE test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var4)
                      : "r"(src4), "i"(5)  /* 5 = UNGE */
                      : "cc");
        cc_accumulator += var4;
    }
    
    /* UNGT condition code */
    {
        int var5 = 700, src5 = 800;
        asm volatile ("# UNGT test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var5)
                      : "r"(src5), "i"(6)  /* 6 = UNGT */
                      : "cc");
        cc_accumulator += var5;
    }
    
    /* UNLE condition code */
    {
        int var6 = 900, src6 = 1000;
        asm volatile ("# UNLE test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var6)
                      : "r"(src6), "i"(2)  /* 2 = UNLE */
                      : "cc");
        cc_accumulator += var6;
    }
    
    /* UNLT condition code */
    {
        int var7 = 1100, src7 = 1200;
        asm volatile ("# UNLT test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var7)
                      : "r"(src7), "i"(3)  /* 3 = UNLT */
                      : "cc");
        cc_accumulator += var7;
    }
    
    /* LTGT condition code */
    {
        int var8 = 1300, src8 = 1400;
        asm volatile ("# LTGT test\n\t"
                      "cmov%C0 %1, %0"
                      : "+r"(var8)
                      : "r"(src8), "i"(4)  /* 4 = LTGT */
                      : "cc");
        cc_accumulator += var8;
    }
    
    /* Additional complex floating-point expressions to ensure RTL generation */
    {
        volatile double x = __builtin_nan("");
        volatile double y = 3.14159;
        volatile double z = 2.71828;
        
        /* Complex expression that should generate multiple condition codes */
        int complex_result = ((x < y) && (y > z) && (x != z)) ? 1 : 
                            ((y == z) || (x >= y) || (z <= x)) ? 2 : 0;
        cc_accumulator += complex_result;
    }
    
    /* Prevent optimization and use result */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
