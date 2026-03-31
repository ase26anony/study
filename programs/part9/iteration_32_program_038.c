#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef double v5df __attribute__((vector_size(40)));

/* 11-argument inline function */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, short b, char c, long d, int e, 
            float f, double g, int h, short i, char j, long k)
{
    /* Complex expression using all arguments */
    return (((((((((((int64_t)a + b) * c) - d) << (e & 3)) & h) | 
              (int)(f * 100)) ^ (int)(g * 1000)) + i) * j) / (k ? k : 1));
}

/* 10-argument inline function */
static inline float __attribute__((always_inline))
multi_op_10(float a, double b, int c, short d, char e,
            float f, double g, int h, short i, char j)
{
    /* Mixed-type expression */
    return (a + (float)b + c + d + e + f + (float)g + h + i + j) / 10.0f;
}

int main(void)
{
    /* 11 volatile variables with prime numbers */
    volatile int8_t  v0  = 2;
    volatile int16_t v1  = 3;
    volatile int32_t v2  = 5;
    volatile int64_t v3  = 7;
    volatile int32_t v4  = 11;
    volatile float   v5  = 13.0f;
    volatile double  v6  = 17.0;
    volatile int32_t v7  = 19;
    volatile int16_t v8  = 23;
    volatile int8_t  v9  = 29;
    volatile int64_t v10 = 31;
    
    /* Additional variables for results */
    volatile int64_t asm_result = 0;
    volatile int64_t expr_result = 0;
    volatile float vector_result = 0.0f;
    volatile int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++)
    {
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
            : "=r"(asm_result)
            : "r"((int)v0), "r"((int)v1), "r"((int)v2),
              "r"((int)v3), "r"((int)v4), "r"((int)v7),
              "r"((int)v8), "r"((int)v9), "r"(iter)
            : "cc"
        );
        
        /* 2. 11-operand C expression */
        expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v7) | 
                         v8) ^ v9) + iter) * (v0 + 1)) / (v10 ? v10 : 1));
        
        /* 3. Vector operations */
        v10si vec_a = {v0, v1, v2, v3, v4, v7, v8, v9, iter, iter+1};
        v10si vec_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        v10si vec_c;
        
        /* Element-wise addition - may expand to multiple operations */
        for (int i = 0; i < 10; i++) {
            vec_c[i] = vec_a[i] + vec_b[i];
        }
        
        /* Sum vector elements */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec_c[i];
        }
        vector_result = (float)vec_sum;
        
        /* 4. Call 11-argument inline function */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, 
                                         v5, v6, v7, v8, v9, v10);
        
        /* 5. Mixed floating-point vector operation */
        v10sf fvec_a = {v5, v5*2, v5*3, (float)v6, (float)v6*2,
                       (float)iter, (float)(iter+1), (float)(iter+2),
                       (float)v0, (float)v1};
        v10sf fvec_b = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                       6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        v10sf fvec_c;
        
        /* Another element-wise operation */
        for (int i = 0; i < 10; i++) {
            fvec_c[i] = fvec_a[i] + fvec_b[i];
        }
        
        /* 6. 11-element vector type (if supported) */
        int v11_arr[11] = {v0, v1, v2, v3, v4, v7, v8, v9, 
                          iter, iter+1, iter+2};
        
        /* Complex chain of operations using all variables */
        checksum += asm_result + expr_result + (int64_t)vector_result + 
                   func_result + (int64_t)fvec_c[0] + v11_arr[iter % 11];
        
        /* Modify volatile variables slightly */
        v0 = (v0 + 1) % 100;
        v5 = v5 + 0.1f;
        v6 = v6 + 0.01;
    }
    
    /* 7. Additional 11-operand inline assembly attempt */
    int64_t final_result;
    __asm__ volatile (
        "/* Another 10/11 operand pattern */\n\t"
        "mov %0, #0\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(final_result)
        : "r"(v0), "r"((int)v1), "r"((int)v2),
          "r"((int)v3), "r"((int)v4), "r"((int)v7),
          "r"((int)v8), "r"((int)v9), "r"((int)v10),
          "r"(checksum & 0xFFFFFFFF)
        : "cc"
    );
    
    /* 8. Mixed-type 11-operand expression */
    double mixed_result = (double)v0 + (double)v1 + (double)v2 + 
                         (double)v3 + (double)v4 + v5 + v6 + 
                         (double)v7 + (double)v8 + (double)v9 + 
                         (double)v10;
    
    /* Prevent dead code elimination */
    if (checksum > 0 || final_result > 0 || mixed_result > 0) {
        printf("Results: checksum=%lld, final=%lld, mixed=%.2f\n", 
               (long long)checksum, (long long)final_result, mixed_result);
    } else {
        printf("Alternative path\n");
    }
    
    return (checksum > 1000000) ? 0 : 1;
}
