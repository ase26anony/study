/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by mixing NaN and normal values */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific positions to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void use_cc_in_asm(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(0)  /* 0 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(7)  /* 7 = ORDERED */
                  : "cc");
    
    /* UNEQ condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(1)  /* 1 = UNEQ */
                  : "cc");
    
    /* UNGE condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(5)  /* 5 = UNGE */
                  : "cc");
    
    /* UNGT condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(2)  /* 2 = UNGT */
                  : "cc");
    
    /* UNLE condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(6)  /* 6 = UNLE */
                  : "cc");
    
    /* UNLT condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(4)  /* 4 = UNLT */
                  : "cc");
    
    /* LTGT condition code */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(3)  /* 3 = LTGT */
                  : "cc");
}

/* Force generation of various floating-point condition codes */
static int generate_fp_condition_codes(double *arr1, double *arr2, int size) {
    volatile int accumulator = 0;
    
    for (int i = 0; i < size; i++) {
        double a = arr1[i];
        double b = arr2[i];
        int temp = 0;
        
        /* Generate all standard FP comparisons */
        /* Each comparison may produce different condition codes */
        
        /* Less than - may generate UNORDERED or UNLT */
        if (a < b) {
            temp |= 1;
        }
        
        /* Less than or equal - may generate UNORDERED or UNLE */
        if (a <= b) {
            temp |= 2;
        }
        
        /* Greater than - may generate UNORDERED or UNGT */
        if (a > b) {
            temp |= 4;
        }
        
        /* Greater than or equal - may generate UNORDERED or UNGE */
        if (a >= b) {
            temp |= 8;
        }
        
        /* Equal - may generate UNORDERED or UNEQ */
        if (a == b) {
            temp |= 16;
        }
        
        /* Not equal - may generate UNORDERED or LTGT */
        if (a != b) {
            temp |= 32;
        }
        
        /* Use ternary operator to potentially generate conditional moves */
        accumulator += (a < b) ? 1 : 0;
        accumulator += (a <= b) ? 2 : 0;
        accumulator += (a > b) ? 4 : 0;
        accumulator += (a >= b) ? 8 : 0;
        accumulator += (a == b) ? 16 : 0;
        accumulator += (a != b) ? 32 : 0;
        
        /* Force ordered/unordered checks */
        if (!isunordered(a, b)) {  /* ORDERED */
            accumulator += 64;
        }
        if (isunordered(a, b)) {   /* UNORDERED */
            accumulator += 128;
        }
    }
    
    return accumulator;
}

/* Use builtin to generate condition code checks */
static void use_builtin_condition_codes(double a, double b) {
    /* These builtins may generate condition code checks */
    volatile int res;
    
    res = __builtin_isgreater(a, b);      /* > */
    res = __builtin_isgreaterequal(a, b); /* >= */
    res = __builtin_isless(a, b);         /* < */
    res = __builtin_islessequal(a, b);    /* <= */
    res = __builtin_islessgreater(a, b);  /* != (but ordered) */
    res = __builtin_isunordered(a, b);    /* UNORDERED */
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE];
    double arr2[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2, SIZE);
    
    /* Direct inline assembly to trigger condition code printing */
    use_cc_in_asm();
    
    /* Generate floating-point condition codes through comparisons */
    int result = generate_fp_condition_codes(arr1, arr2, SIZE);
    
    /* Use builtins for additional condition code generation */
    use_builtin_condition_codes(arr1[0], arr2[0]);
    use_builtin_condition_codes(__builtin_nan(""), arr2[1]);
    use_builtin_condition_codes(arr1[2], __builtin_nan(""));
    use_builtin_condition_codes(__builtin_nan(""), __builtin_nan(""));
    
    /* Additional complex floating-point expressions */
    volatile double x = 1.0 / 0.0;  /* Infinity */
    volatile double y = -1.0 / 0.0; /* Negative infinity */
    volatile double z = 0.0 / 0.0;  /* NaN */
    
    /* These should generate various condition codes */
    int cmp1 = (x > y);   /* Should be true, both ordered */
    int cmp2 = (x == z);  /* false, z is NaN */
    int cmp3 = (z != z);  /* true, NaN != NaN */
    int cmp4 = (z < x);   /* false, unordered */
    
    /* Mix integer and float for potential conditional move generation */
    int int_result = 0;
    for (int i = 0; i < SIZE; i++) {
        /* Ternary with FP condition on integer target */
        int_result += (arr1[i] < arr2[i]) ? i : -i;
        int_result += (arr1[i] <= arr2[i]) ? (i * 2) : -(i * 2);
        int_result += (arr1[i] > arr2[i]) ? (i * 3) : -(i * 3);
        int_result += (arr1[i] >= arr2[i]) ? (i * 4) : -(i * 4);
    }
    
    /* Prevent optimization */
    printf("Result: %d, Int result: %d\n", result, int_result);
    printf("Comparisons: %d %d %d %d\n", cmp1, cmp2, cmp3, cmp4);
    
    return 0;
}
