#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* Custom builtin-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_operand_op(int a, int b, int c, int d, int e,
                 int f, int g, int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 0x3) | (f ^ g) + (h * i) - (j / (k ? k : 1));
}

/* Function with 10 arguments mixing types */
static inline float __attribute__((always_inline))
mixed_type_op(char a, short b, int c, long d, float e,
              double f, char g, short h, int i, long j) {
    return (float)a + (float)b + (float)c + (float)d + e + 
           (float)f + (float)g + (float)h + (float)i + (float)j;
}

int main() {
    /* Declare 11 volatile variables with distinct prime values */
    volatile char v0 = 2;
    volatile short v1 = 3;
    volatile int v2 = 5;
    volatile long v3 = 7;
    volatile float v4 = 11.0f;
    volatile double v5 = 13.0;
    volatile char v6 = 17;
    volatile short v7 = 19;
    volatile int v8 = 23;
    volatile long v9 = 29;
    volatile int v10 = 31;
    
    /* Additional variables for results */
    volatile int64_t asm_result = 0;
    volatile int64_t expr_result = 0;
    volatile float mixed_result = 0.0f;
    volatile int vector_result = 0;
    
    /* Checksum accumulator */
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        __asm__ volatile (
            "/* Dummy 10-operand operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(asm_result)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
              "r"((int)v4), "r"((int)v5), "r"(v6),
              "r"(v7), "r"(v8), "r"(v9)
            : "cc"
        );
        
        /* 2. 11-operand complex expression */
        expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 > 0 ? 1 : 0)) & v5) | 
                         v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + 
                         (v0 % v1) - (v2 & v3) | (v4 > v5);
        
        /* 3. 11-operand function call */
        expr_result += multi_operand_op(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* 4. Mixed-type 10-operand function */
        mixed_result = mixed_type_op(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        
        /* 5. Vector operations with 10 and 11 elements */
        v10si vec10_a = {v0, v1, v2, v3, (int)v4, (int)v5, v6, v7, v8, v9};
        v10si vec10_b = {v10, v0, v1, v2, v3, (int)v4, (int)v5, v6, v7, v8};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, (int)v4, (int)v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v10, v9, v8, v7, v6, (int)v5, (int)v4, v3, v2, v1, v0};
        v11si vec11_result = vec11_a - vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            vector_result += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            vector_result -= vec11_result[i];
        }
        
        /* Update checksum */
        checksum += asm_result + expr_result + (int64_t)mixed_result + vector_result;
        
        /* Modify variables slightly to prevent complete optimization */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 0.5f;
        v5 += 0.25; v6 += 5; v7 += 6; v8 += 7; v9 += 8; v10 += 9;
    }
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum (positive): %lld\n", (long long)checksum);
    } else if (checksum < 0) {
        printf("Checksum (negative): %lld\n", (long long)checksum);
    } else {
        printf("Checksum (zero): %lld\n", (long long)checksum);
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = asm_result + expr_result + mixed_result + vector_result;
    
    return checksum != 0 ? 0 : 1;
}
