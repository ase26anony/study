/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons with NaN */
static inline double make_nan(void) {
    return __builtin_nan("");
}

/* Mix of inline assembly and floating-point comparisons to trigger
   all condition code name printing */
int main(void) {
    /* Arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = make_nan();
        }
        if (i % 11 == 0) {
            arr2[i] = make_nan();
        }
    }
    
    volatile int cc_accumulator = 0;
    int result = 0;
    
    /* Loop performing all floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all standard FP comparisons - each may generate different condition codes */
        int lt   = (a < b)  ? 1 : 0;
        int le   = (a <= b) ? 2 : 0;
        int gt   = (a > b)  ? 3 : 0;
        int ge   = (a >= b) ? 4 : 0;
        int eq   = (a == b) ? 5 : 0;
        int neq  = (a != b) ? 6 : 0;
        
        /* Force use of results to prevent optimization */
        cc_accumulator += lt + le + gt + ge + eq + neq;
        
        /* Ternary operators that may generate conditional moves */
        result = (a < b)  ? result | 0x01 : result & ~0x01;
        result = (a <= b) ? result | 0x02 : result & ~0x02;
        result = (a > b)  ? result | 0x04 : result & ~0x04;
        result = (a >= b) ? result | 0x08 : result & ~0x08;
        result = (a == b) ? result | 0x10 : result & ~0x10;
        result = (a != b) ? result | 0x20 : result & ~0x20;
    }
    
    /* Direct inline assembly with %C constraint to trigger condition code printing */
    /* UNORDERED condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNORDERED condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(0)  /* 0 = UNORDERED */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* ORDERED condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# ORDERED condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(7)  /* 7 = ORDERED */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* UNEQ condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNEQ condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(8)  /* 8 = UNEQ */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* UNGE condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNGE condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(13) /* 13 = UNGE */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* UNGT condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNGT condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(14) /* 14 = UNGT */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* UNLE condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNLE condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(15) /* 15 = UNLE */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* UNLT condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# UNLT condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(16) /* 16 = UNLT */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* LTGT condition */
    {
        int var = 42;
        int src = 99;
        asm volatile (
            "# LTGT condition code\n\t"
            "cmov%C0 %1, %0\n\t"
            : "+r"(var)
            : "r"(src), "i"(12) /* 12 = LTGT */
            : "cc"
        );
        cc_accumulator += var;
    }
    
    /* Additional unordered comparison scenarios */
    {
        double nan1 = make_nan();
        double nan2 = make_nan();
        double normal = 3.14159;
        
        /* These comparisons should generate unordered condition codes */
        volatile int unord1 = isnan(nan1);
        volatile int unord2 = (nan1 != nan1);  /* NaN != NaN is true */
        volatile int unord3 = (nan1 == nan2);  /* NaN == NaN is false */
        volatile int unord4 = (nan1 < normal); /* NaN < anything is false (unordered) */
        volatile int unord5 = (normal > nan2); /* anything > NaN is false (unordered) */
        
        cc_accumulator += unord1 + unord2 + unord3 + unord4 + unord5;
    }
    
    /* Complex floating-point expression that may generate condition codes */
    {
        double x = 1.0;
        double y = 2.0;
        double z = make_nan();
        
        /* Mixed comparisons that may generate various condition codes */
        for (int i = 0; i < 10; i++) {
            x += 0.1;
            y -= 0.05;
            
            /* Chain of comparisons */
            if ((x < y) && !(z == z)) {  /* z == z is false for NaN */
                cc_accumulator++;
            }
            
            if ((x >= y) || (z != z)) {  /* z != z is true for NaN */
                cc_accumulator--;
            }
        }
    }
    
    printf("Result: %d, Accumulator: %d\n", result, cc_accumulator);
    return 0;
}
