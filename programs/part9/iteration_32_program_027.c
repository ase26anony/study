#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10f __attribute__((vector_size(40)));
typedef double v5d __attribute__((vector_size(40))); /* 5 doubles = 40 bytes */

/* 11-argument inline function */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, 
            int f, int g, int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + h * i - j / (k + 1);
}

/* 10-argument inline function with mixed types */
static inline float __attribute__((always_inline))
mixed_op_10(float a, double b, int c, short d, char e,
            float f, double g, int h, short i, char j) {
    return a + (float)b + c + d + e + f + (float)g + h + i + j;
}

int main() {
    /* Declare 11 volatile variables with prime numbers */
    volatile int v0 = 2;
    volatile int v1 = 3;
    volatile int v2 = 5;
    volatile int v3 = 7;
    volatile int v4 = 11;
    volatile int v5 = 13;
    volatile int v6 = 17;
    volatile int v7 = 19;
    volatile int v8 = 23;
    volatile int v9 = 29;
    volatile int v10 = 31;
    
    /* Additional variables for mixed types */
    volatile float f1 = 2.5f;
    volatile double d1 = 3.14159;
    volatile short s1 = 7;
    volatile char c1 = 11;
    
    volatile float f2 = 1.618f;
    volatile double d2 = 2.71828;
    volatile short s2 = 13;
    volatile char c2 = 17;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int asm_result;
        __asm__ volatile (
            "/* Custom 10-operand operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(asm_result)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        checksum += asm_result;
        
        /* 2. 11-operand inline assembly */
        int asm_result2;
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
            "mov %0, %1\n\t"
            "imul %0, %2\n\t"
            "add %0, %3\n\t"
            "sub %0, %4\n\t"
            "add %0, %5\n\t"
            "xor %0, %6\n\t"
            "or %0, %7\n\t"
            "and %0, %8\n\t"
            "add %0, %9\n\t"
            "sub %0, %10"
            : "=r"(asm_result2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        checksum += asm_result2;
        
        /* 3. Complex 11-operand C expression */
        int expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                             v6) ^ v7) + v8) * v9) / (v10 + 1)) + 
                         (((v0 | v1) & v2) ^ (v3 << 1));
        checksum += expr_result;
        
        /* 4. 11-argument function call */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += func_result;
        
        /* 5. Mixed-type 10-operand function */
        float float_result = mixed_op_10(f1, d1, v0, s1, c1, f2, d2, v1, s2, c2);
        checksum += (int64_t)float_result;
        
        /* 6. Vector operations */
        v10si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec_c = vec_a + vec_b;
        
        /* Extract all elements to force scalarization */
        for (int i = 0; i < 10; i++) {
            checksum += vec_c[i];
        }
        
        /* 7. Another complex expression with 11 variables */
        int expr2 = (v0 & v1) | (v2 ^ v3) + ((v4 * v5) >> (v6 & 3)) - 
                   (v7 / (v8 + 1)) * (v9 % (v10 + 1)) + (v0 ^ v10);
        checksum += expr2;
        
        /* 8. 11-operand expression with mixed operations */
        int expr3 = v0 + (v1 - v2) * (v3 + v4) / (v5 - v6) | (v7 & v8) ^ (v9 << v10);
        checksum += expr3;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Use all variables in final computation to prevent optimization */
    volatile int final = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    printf("Final: %d\n", final);
    
    return 0;
}
