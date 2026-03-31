/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by mixing NaN values */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.1;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly to trigger %C format specifier */
static void inline_asm_cc(void) {
    int var = 0;
    int src = 42;
    
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
        : "r"(src), "i"(1)  /* 1 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(5)  /* 5 = UNGE */
        : "cc"
    );
    
    /* UNGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = UNGT */
        : "cc"
    );
    
    /* UNLE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(2)  /* 2 = UNLE */
        : "cc"
    );
    
    /* UNLT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(3)  /* 3 = UNLT */
        : "cc"
    );
    
    /* LTGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Perform all floating-point comparisons to generate condition codes */
static int perform_comparisons(double a, double b) {
    volatile int result = 0;
    
    /* Generate all condition codes through comparisons */
    result += (a < b) ? 1 : 0;    /* UNLT or LT */
    result += (a <= b) ? 2 : 0;   /* UNLE or LE */
    result += (a > b) ? 4 : 0;    /* UNGT or GT */
    result += (a >= b) ? 8 : 0;   /* UNGE or GE */
    result += (a == b) ? 16 : 0;  /* UNEQ or EQ */
    result += (a != b) ? 32 : 0;  /* LTGT or NE */
    
    /* Ordered/unordered checks */
    result += (a == a && b == b) ? 64 : 0;  /* ORDERED */
    result += (a != a || b != b) ? 128 : 0; /* UNORDERED */
    
    return result;
}

/* Use ternary operators with floating-point conditions to force conditional moves */
static int ternary_with_fp_cond(double a, double b) {
    int x = 0, y = 0, z = 0;
    
    /* These may generate conditional move instructions */
    x = (a < b) ? 100 : 200;
    y = (a > b) ? 300 : 400;
    z = (a != b) ? 500 : 600;
    
    /* Force use of results */
    asm volatile ("" : : "r"(x), "r"(y), "r"(z));
    return x + y + z;
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE], arr2[SIZE];
    volatile int cc_accumulator = 0;
    
    /* Initialize with NaN values at specific indices */
    init_arrays(arr1, arr2, SIZE);
    
    /* Trigger inline assembly with %C format specifier */
    inline_asm_cc();
    
    /* Loop through arrays performing all comparisons */
    for (int i = 0; i < SIZE; i++) {
        /* Force unordered comparisons by mixing NaN and normal values */
        cc_accumulator += perform_comparisons(arr1[i], arr2[i]);
        
        /* Use ternary operators that may generate conditional moves */
        cc_accumulator += ternary_with_fp_cond(arr1[i], arr2[i]);
        
        /* Direct unordered check */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator += 1000;
        }
        
        /* Direct ordered check */
        if (isordered(arr1[i], arr2[i])) {
            cc_accumulator += 2000;
        }
    }
    
    /* Additional complex floating-point expressions */
    double volatile nan_val = __builtin_nan("");
    double volatile inf_val = __builtin_inf();
    double volatile normal_val = 3.14159;
    
    /* Mixed comparisons that should generate various condition codes */
    cc_accumulator += (nan_val < normal_val) ? 1 : 0;
    cc_accumulator += (normal_val > nan_val) ? 2 : 0;
    cc_accumulator += (nan_val == nan_val) ? 4 : 0;
    cc_accumulator += (inf_val >= normal_val) ? 8 : 0;
    cc_accumulator += (normal_val <= inf_val) ? 16 : 0;
    cc_accumulator += (nan_val != normal_val) ? 32 : 0;
    
    /* Prevent dead code elimination */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Additional inline assembly with explicit condition codes */
    int result = 0;
    double cmp_a = __builtin_nan("");
    double cmp_b = 1.0;
    
    /* This should generate UNORDERED condition code */
    if (cmp_a < cmp_b) {
        result = 1;
    }
    
    /* Force assembly output with condition codes */
    asm volatile (
        "# ========== Condition Code Test ==========\n\t"
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "cmov%C3 %3, %0\n\t"
        : "+r"(result)
        : "m"(cmp_a), "m"(cmp_b), "r"(999), "i"(0)  /* 0 = UNORDERED */
        : "cc", "st", "st(1)"
    );
    
    printf("Final result: %d\n", result);
    return 0;
}
