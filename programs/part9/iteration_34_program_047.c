/* test_optabs.c - Program to trigger case 10: in optabs.cc (10-operand expansion) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle function that conceptually needs many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d, 
                                       int idx0, int idx1, int idx2, int idx3,
                                       int idx4, int idx5) {
    /* This complex shuffle pattern should require expansion with many operands */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, idx2, idx3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){idx4, idx5, idx0, idx1});
    
    /* Mix them with bitwise operations using immediate masks */
    v4si mask1 = (v4si){0x00FF00FF, 0xFF00FF00, 0x00FF00FF, 0xFF00FF00};
    v4si mask2 = (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F};
    
    v4si result = (temp1 & mask1) | (temp2 & mask2);
    return result;
}

/* Vector conversion with many parameters */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d,
                                      float scale1, float scale2, 
                                      float bias1, float bias2,
                                      int shift1, int shift2) {
    /* Multiple conversions and operations that may expand to many operands */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Complex expression with many constants */
    v4sf result = fa * (v4sf){scale1, scale2, scale1, scale2} +
                  fb * (v4sf){bias1, bias2, bias1, bias2} +
                  fc * (v4sf){shift1, shift2, shift1, shift2} +
                  fd * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    return result;
}

/* Custom vector permute that uses many immediate values */
static v4si custom_vector_permute(v4si v0, v4si v1, v4si v2, v4si v3) {
    /* This should trigger a complex expansion with many operands */
    /* Using __builtin_shuffle with multiple vectors and control */
    v4si temp = __builtin_shuffle(v0, v1, (v4si){3, 2, 1, 0});
    temp = __builtin_shuffle(temp, v2, (v4si){4, 5, 6, 7});
    temp = __builtin_shuffle(temp, v3, (v4si){8, 9, 10, 11});
    
    /* Additional bitwise operations with immediate masks */
    v4si result = temp ^ (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    result = result & (v4si){0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000};
    result = result | (v4si){0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    /* Initialize vectors with volatile to prevent constant propagation */
    volatile int init_val = argc > 1 ? atoi(argv[1]) : 42;
    
    v4si vec_a = {init_val + 1, init_val + 2, init_val + 3, init_val + 4};
    v4si vec_b = {init_val + 5, init_val + 6, init_val + 7, init_val + 8};
    v4si vec_c = {init_val + 9, init_val + 10, init_val + 11, init_val + 12};
    v4si vec_d = {init_val + 13, init_val + 14, init_val + 15, init_val + 16};
    
    v4sf vec_fa = {init_val * 0.1f, init_val * 0.2f, 
                   init_val * 0.3f, init_val * 0.4f};
    v4sf vec_fb = {init_val * 0.5f, init_val * 0.6f, 
                   init_val * 0.7f, init_val * 0.8f};
    
    /* Result accumulator */
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    int iterations = argc > 2 ? atoi(argv[2]) : 100;
    for (int i = 0; i < iterations; i++) {
        /* Complex shuffle with many parameters - targets case 10: */
        v4si shuffle_result = complex_shuffle_10_operand(
            vec_a, vec_b, vec_c, vec_d,
            i % 4, (i + 1) % 4, (i + 2) % 4, (i + 3) % 4,
            (i + 4) % 8, (i + 5) % 8
        );
        
        /* Vector conversion with many parameters */
        v4sf conv_result = vector_conversion_complex(
            vec_a, vec_b, vec_c, vec_d,
            1.0f + i * 0.1f, 2.0f + i * 0.2f,
            0.5f + i * 0.05f, 1.5f + i * 0.15f,
            i % 8, (i + 4) % 8
        );
        
        /* Custom permute */
        v4si permute_result = custom_vector_permute(vec_a, vec_b, vec_c, vec_d);
        
        /* Accumulate results to prevent optimization */
        int_result += shuffle_result + permute_result;
        float_result += conv_result;
        
        /* Modify inputs slightly */
        vec_a[0] += 1;
        vec_b[1] += 1;
        vec_c[2] += 1;
        vec_d[3] += 1;
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile v4si final_int_result = int_result;
    volatile v4sf final_float_result = float_result;
    
    /* Print something to create side effect */
    printf("Int result: [%d, %d, %d, %d]\n", 
           final_int_result[0], final_int_result[1], 
           final_int_result[2], final_int_result[3]);
    printf("Float result: [%f, %f, %f, %f]\n",
           final_float_result[0], final_float_result[1],
           final_float_result[2], final_float_result[3]);
    
    return 0;
}
