/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by mixing NaN values */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition */
    asm volatile (
        "cmov%C1 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition */
    asm volatile (
        "cmov%C2 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(1)  /* 1 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition */
    asm volatile (
        "cmov%C3 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(2)  /* 2 = UNGE */
        : "cc"
    );
    
    /* UNGT condition */
    asm volatile (
        "cmov%C4 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(3)  /* 3 = UNGT */
        : "cc"
    );
    
    /* UNLE condition */
    asm volatile (
        "cmov%C5 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = UNLE */
        : "cc"
    );
    
    /* UNLT condition */
    asm volatile (
        "cmov%C6 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(5)  /* 5 = UNLT */
        : "cc"
    );
    
    /* LTGT condition */
    asm volatile (
        "cmov%C7 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Generate all floating-point comparisons to produce various condition codes */
static int generate_fp_comparisons(double a, double b) {
    volatile int result = 0;
    
    /* All standard comparisons that can produce different condition codes */
    result += (a < b) ? 1 : 0;    /* LT */
    result += (a <= b) ? 2 : 0;   /* LE */
    result += (a > b) ? 4 : 0;    /* GT */
    result += (a >= b) ? 8 : 0;   /* GE */
    result += (a == b) ? 16 : 0;  /* EQ */
    result += (a != b) ? 32 : 0;  /* NE */
    
    /* Ordered/unordered checks */
    result += (!isunordered(a, b)) ? 64 : 0;   /* ORDERED */
    result += (isunordered(a, b)) ? 128 : 0;   /* UNORDERED */
    
    return result;
}

/* Use ternary operators with FP conditions to potentially generate conditional moves */
static int ternary_with_fp_cond(double a, double b) {
    int x = 0;
    
    /* These may generate conditional move instructions */
    x = (a < b) ? (x | 0x01) : (x & ~0x01);
    x = (a <= b) ? (x | 0x02) : (x & ~0x02);
    x = (a > b) ? (x | 0x04) : (x & ~0x04);
    x = (a >= b) ? (x | 0x08) : (x & ~0x08);
    x = (a == b) ? (x | 0x10) : (x & ~0x10);
    x = (a != b) ? (x | 0x20) : (x & ~0x20);
    
    return x;
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE], arr2[SIZE];
    volatile int cc_accumulator = 0;
    
    /* Initialize with mixed normal and NaN values */
    init_arrays(arr1, arr2, SIZE);
    
    /* Direct inline assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Loop through arrays performing all types of comparisons */
    for (int i = 0; i < SIZE; i++) {
        /* Generate various condition codes from FP comparisons */
        cc_accumulator += generate_fp_comparisons(arr1[i], arr2[i]);
        
        /* Use ternary operators that may generate conditional moves */
        cc_accumulator += ternary_with_fp_cond(arr1[i], arr2[i]);
        
        /* Force unordered comparisons explicitly */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator |= 0x100;
        }
        
        if (!isunordered(arr1[i], arr2[i])) {
            cc_accumulator |= 0x200;
        }
    }
    
    /* Additional inline assembly blocks with different condition codes */
    {
        int x = 0, y = 1;
        
        /* More %C usage with different conditions */
        asm volatile (
            "test %1, %1\n\t"
            "set%C0 %b2\n\t"
            : "=r"(x)
            : "r"(y), "i"(0)  /* UNORDERED */
            : "cc"
        );
        
        asm volatile (
            "test %1, %1\n\t"
            "set%C1 %b2\n\t"
            : "=r"(x)
            : "r"(y), "i"(6)  /* LTGT */
            : "cc"
        );
    }
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
