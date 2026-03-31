#include <stdio.h>
#include <stdint.h>

/* Define vector types with exactly 10 and 11 elements */
typedef int v10si __attribute__((vector_size(10 * sizeof(int))));
typedef int v11si __attribute__((vector_size(11 * sizeof(int))));

/* 11-argument inline function that must be expanded */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, 
            int g, int h, int i, int j, int k) {
    /* Complex expression that uses all 11 arguments */
    return ((((((((((int64_t)a * b) + c) - d) << (e & 3)) & f) | 
              g) ^ h) + i) * j) / (k ? k : 1);
}

/* 10-argument inline function */
static inline int __attribute__((always_inline))
multi_op_10(char a, short b, int c, long d, float e, 
            double f, int g, short h, char i, long j) {
    /* Mixed-type expression */
    return (int)(a + b + c + d + (int)e + (int)f + g + h + i + j);
}

int main() {
    /* 11 volatile variables with prime numbers */
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
    
    /* Additional mixed-type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        int result1, result2;
        int64_t result3;
        
        /* 1. 10-operand inline assembly */
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
            : "=r"(result1)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        
        /* 2. 11-operand C expression using all volatile variables */
        result2 = (((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                    v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        
        /* 3. 11-operand function call */
        result3 = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* 4. 10-operand mixed-type function */
        int result4 = multi_op_10(c1, s1, v0, l1, f1, d1, v1, v2, (char)v3, v4);
        
        /* 5. Vector operations with 10 and 11 elements */
        v10si vec10 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        v10si vec10_result = vec10 + vec10_add;
        
        v11si vec11 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec11_result = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum += vec11_result[i];
        }
        
        /* 6. Another 11-operand expression with mixed operations */
        int result5 = (v0 & v1) | (v2 ^ v3) + (v4 * v5) - (v6 / (v7 ? v7 : 1)) +
                     (v8 << 2) + (v9 >> 1) + (v10 & 0xFF);
        
        /* Update checksum with all results */
        checksum += result1 + result2 + result3 + result4 + result5 + vec_sum;
        
        /* Modify variables slightly each iteration */
        v0 = v0 + 1;
        v1 = v1 - 1;
        v2 = v2 ^ iter;
        v3 = v3 + (iter & 1);
        v4 = v4 - (iter & 1);
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression using all variables one last time */
    int final_result = 
        ((((((((((v0 * v1) + v2 - v3) << (v4 & 3)) & v5) | v6) ^ v7) + v8) * v9) / 
         (v10 ? v10 : 1)) + c1 + s1 + l1 + (int)f1 + (int)d1;
    
    printf("Final result: %d\n", final_result);
    
    return (checksum > 0) ? 0 : 1;
}
