/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static inline int compare_unordered(double a, double b) {
    volatile int res;
    /* All standard comparisons that can generate various condition codes */
    res = (a < b);   /* May generate UNLT or LTGT */
    res = (a <= b);  /* May generate UNLE or LTGT */
    res = (a > b);   /* May generate UNGT or LTGT */
    res = (a >= b);  /* May generate UNGE or LTGT */
    res = (a == b);  /* May generate UNEQ or ORDERED */
    res = (a != b);  /* May generate LTGT or UNORDERED */
    return res;
}

/* Direct inline assembly with %C constraint */
static void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition */
    __asm__ volatile (
        "# UNORDERED condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(16)  /* 16 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition */
    __asm__ volatile (
        "# ORDERED condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(17)  /* 17 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition */
    __asm__ volatile (
        "# UNEQ condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(18)  /* 18 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition */
    __asm__ volatile (
        "# UNGE condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(19)  /* 19 = UNGE */
        : "cc"
    );
    
    /* UNGT condition */
    __asm__ volatile (
        "# UNGT condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(20)  /* 20 = UNGT */
        : "cc"
    );
    
    /* UNLE condition */
    __asm__ volatile (
        "# UNLE condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(21)  /* 21 = UNLE */
        : "cc"
    );
    
    /* UNLT condition */
    __asm__ volatile (
        "# UNLT condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(22)  /* 22 = UNLT */
        : "cc"
    );
    
    /* LTGT condition */
    __asm__ volatile (
        "# LTGT condition code\n\t"
        "cmov%C0 %1, %0"
        : "+r"(var)
        : "r"(src), "i"(23)  /* 23 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    volatile int dummy = var;
    (void)dummy;
}

/* Generate mixed NaN and normal values */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        /* Normal values */
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.5;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        
        /* Create some equal values for EQ comparisons */
        if (i % 13 == 0) {
            arr1[i] = arr2[i] = 100.0 + i;
        }
    }
}

int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize with mixed NaN and normal values */
    init_arrays(arr1, arr2);
    
    /* Loop performing all types of floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all standard comparisons */
        cc_accumulator += (a < b) ? 1 : 0;   /* UNLT or LTGT */
        cc_accumulator += (a <= b) ? 2 : 0;  /* UNLE or LTGT */
        cc_accumulator += (a > b) ? 4 : 0;   /* UNGT or LTGT */
        cc_accumulator += (a >= b) ? 8 : 0;  /* UNGE or LTGT */
        cc_accumulator += (a == b) ? 16 : 0; /* UNEQ or ORDERED */
        cc_accumulator += (a != b) ? 32 : 0; /* LTGT or UNORDERED */
        
        /* Use ternary operator to force conditional move generation */
        int res1 = (a < b) ? i : -i;
        int res2 = (a > b) ? i*2 : -i*2;
        int res3 = (a == b) ? i*3 : -i*3;
        
        cc_accumulator += res1 + res2 + res3;
        
        /* Complex expression that may generate various condition codes */
        if (!__builtin_isunordered(a, b)) {
            /* ORDERED comparison */
            cc_accumulator += (a <= b) ? 64 : 128;
        } else {
            /* UNORDERED comparison */
            cc_accumulator += 256;
        }
    }
    
    /* Force emission of condition codes via inline assembly */
    emit_condition_codes();
    
    /* Additional unordered comparisons using builtins */
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nan("");
    double normal = 3.14159;
    
    /* These should generate UNORDERED/ORDERED codes */
    volatile int unord_test = __builtin_isunordered(nan1, normal);
    volatile int ord_test = !__builtin_isunordered(normal, normal);
    
    /* Mixed comparisons that may generate various condition codes */
    cc_accumulator += compare_unordered(nan1, normal);
    cc_accumulator += compare_unordered(normal, nan2);
    cc_accumulator += compare_unordered(normal, normal);
    cc_accumulator += compare_unordered(nan1, nan2);
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
