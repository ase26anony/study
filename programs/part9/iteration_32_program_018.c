#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* Custom inline function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + h * i - j / (k ? k : 1);
}

/* Custom inline function with 10 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j) {
    return ((int64_t)a + b) * c - d + (e << 2) | (f & g) ^ h + i * j;
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
    volatile char c1 = 127;
    volatile short s1 = 32767;
    volatile long l1 = 65537L;
    volatile float f1 = 3.14159f;
    volatile double d1 = 2.71828;
    
    volatile uint64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        int64_t result1, result2, result3, result4;
        
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
            "add %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r" (result1)
            : "r" (v0 + iter), "r" (v1), "r" (v2), "r" (v3), 
              "r" (v4), "r" (v5), "r" (v6), "r" (v7), 
              "r" (v8), "r" (v9)
            : "cc"
        );
        
        /* 2. 11-operand C expression using all volatile variables */
        result2 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                     v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + 
                  c1 - s1 + l1 + (int)f1 + (int)d1;
        
        /* 3. 11-operand function call */
        result3 = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* 4. 10-operand function call with mixed types */
        result4 = multi_op_10(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        
        /* 5. Vector operations with 10 and 11 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_c = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec11_c = vec11_a * vec11_b;
        
        /* Mixed floating-point vectors */
        v10sf vec10f_a = {f1, f1*2, f1*3, f1*4, f1*5, f1*6, f1*7, f1*8, f1*9, f1*10};
        v10sf vec10f_b = {d1, d1*2, d1*3, d1*4, d1*5, d1*6, d1*7, d1*8, d1*9, d1*10};
        v10sf vec10f_c = vec10f_a + vec10f_b;
        
        /* Extract results from vectors */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_c[i] + vec11_c[i % 11];
        }
        
        /* Update checksum with all results */
        checksum += result1 + result2 + result3 + result4 + vec_sum + iter;
        
        /* Modify volatile variables slightly */
        v0 += 1;
        v1 -= 1;
        v2 ^= iter;
        v3 |= 1;
    }
    
    /* Complex conditional to prevent dead code elimination */
    if (checksum > 1000000) {
        printf("Checksum: %lu\n", (unsigned long)checksum);
        
        /* Additional 11-operand expression in print path */
        volatile int final_result = 
            v0 + v1 * v2 - v3 | v4 & v5 ^ v6 + v7 - v8 * v9 / (v10 ? v10 : 1);
        printf("Final: %d\n", final_result);
    } else {
        printf("Small checksum: %lu\n", (unsigned long)checksum);
        
        /* Alternative 10-operand expression */
        volatile int alt_result = 
            (((((((((v0 << 2) + v1) * v2) - v3) | v4) & v5) ^ v6) + v7) * v8) / v9;
        printf("Alt: %d\n", alt_result);
    }
    
    return 0;
}
