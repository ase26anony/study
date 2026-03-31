#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static volatile int32_t volatile_buffer[ARRAY_SIZE];
static volatile float volatile_float_buffer[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Accumulator arrays */
static v8si int_accum_256[4];
static v16si int_accum_512[4];
static v8sf float_accum_256[4];
static v16sf float_accum_512[4];

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple deterministic pseudo-random sequence */
        global_ints[i] = (i * 1103515245 + seed) % 1000;
        global_floats[i] = (float)((i * 1103515245 + seed) % 1000) / 100.0f;
    }
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
static v8si shuffle_10_operand_int(v8si a, v8si b, v8si c, v8si d, 
                                   v8si mask1, v8si mask2, v8si mask3, 
                                   volatile int idx) {
    /* Complex control flow to prevent optimization */
    v8si result;
    if (idx & 1) {
        /* This shuffle uses 10 operands total during expansion:
           2 input vectors + 8-element mask vector = 10 operands */
        v8si temp = __builtin_shuffle(a, b, mask1);
        
        /* Another operation to keep values alive */
        result = temp + c;
        
        /* Volatile store to prevent dead code elimination */
        volatile_buffer[0] = ((int32_t*)&result)[0];
    } else {
        /* Alternative path with different shuffle pattern */
        v8si temp = __builtin_shuffle(c, d, mask2);
        result = temp - a;
        volatile_buffer[1] = ((int32_t*)&result)[1];
    }
    
    /* Nested switch for more control flow complexity */
    switch (idx % 4) {
        case 0:
            return result;
        case 1:
            return result + mask3;
        case 2:
            return result - mask3;
        default:
            return result * 2;
    }
}
#endif

#ifdef __AVX512F__
/* Function targeting 11-operand expansion */
static v16si shuffle_11_operand_int(v16si a, v16si b, v16si c, v16si d,
                                    v16si mask1, v16si mask2, v16si mask3,
                                    volatile int idx1, volatile int idx2) {
    v16si result;
    
    /* Loop with conditional shuffle execution */
    for (int i = 0; i < 3; i++) {
        if ((idx1 + i) % 2 == 0) {
            /* __builtin_shuffle with 16-element vectors:
               2 input vectors + 16-element mask = 18 operands total
               This should trigger the 11+ operand path */
            v16si temp = __builtin_shuffle(a, b, mask1);
            
            /* Mix with other vectors */
            result = temp + c - d;
            
            /* Volatile store */
            volatile_buffer[i * 4] = ((int32_t*)&result)[i];
        } else {
            /* Alternative shuffle pattern */
            v16si temp = __builtin_shuffle(c, d, mask2);
            result = temp * a + b;
            volatile_buffer[i * 4 + 1] = ((int32_t*)&result)[i + 1];
        }
    }
    
    /* Final shuffle that could use many operands */
    if (idx2 > 0) {
        v16si final_shuffle = __builtin_shuffle(result, a, mask3);
        return final_shuffle;
    }
    
    return result;
}
#endif

/* Mixed floating-point shuffle with control flow */
#ifdef __AVX2__
static v8sf shuffle_float_complex(v8sf a, v8sf b, v8sf c, 
                                  v8si int_mask, volatile int pattern) {
    v8sf result;
    
    /* Switch statement to create complex control flow */
    switch (pattern % 5) {
        case 0: {
            /* Shuffle with conversion-like pattern */
            v8sf temp = __builtin_shuffle(a, b, int_mask);
            result = temp * c;
            break;
        }
        case 1: {
            /* Different shuffle arrangement */
            v8si temp_mask = int_mask + (v8si){1,2,3,4,5,6,7,8};
            v8sf temp = __builtin_shuffle(b, c, temp_mask);
            result = temp + a;
            break;
        }
        case 2: {
            /* Chain multiple shuffles */
            v8sf temp1 = __builtin_shuffle(a, c, int_mask);
            v8sf temp2 = __builtin_shuffle(b, temp1, int_mask);
            result = temp2 * 2.0f;
            break;
        }
        case 3: {
            /* Shuffle with arithmetic */
            v8sf temp = __builtin_shuffle(a, b, int_mask);
            result = temp / c;
            break;
        }
        default: {
            /* Identity shuffle with modification */
            v8si identity_mask = {0,1,2,3,4,5,6,7};
            result = __builtin_shuffle(a, a, identity_mask);
            break;
        }
    }
    
    /* Store to volatile buffer */
    volatile_float_buffer[0] = ((float*)&result)[0];
    volatile_float_buffer[1] = ((float*)&result)[1];
    
    return result;
}
#endif

/* Function using __builtin_shufflevector for large vectors */
#ifdef __AVX512F__
static v8df shufflevector_double_large(v8df a, v8df b, volatile int start_idx) {
    /* __builtin_shufflevector with 512-bit vectors (8 doubles each)
       This creates many operands: 2 input vectors + 8 output indices = 10 operands */
    
    /* Use runtime-dependent indices to prevent constant folding */
    int idx0 = (start_idx + 0) % 8;
    int idx1 = (start_idx + 1) % 8;
    int idx2 = (start_idx + 2) % 8;
    int idx3 = (start_idx + 3) % 8;
    int idx4 = (start_idx + 4) % 8;
    int idx5 = (start_idx + 5) % 8;
    int idx6 = (start_idx + 6) % 8;
    int idx7 = (start_idx + 7) % 8;
    
    /* This should generate 10 operands for the expander */
    v8df result = __builtin_shufflevector(a, b, 
                                          idx0, idx1, idx2, idx3,
                                          idx4, idx5, idx6, idx7);
    
    return result;
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Initialize accumulators */
    memset(int_accum_256, 0, sizeof(int_accum_256));
    memset(int_accum_512, 0, sizeof(int_accum_512));
    memset(float_accum_256, 0, sizeof(float_accum_256));
    memset(float_accum_512, 0, sizeof(float_accum_512));
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        volatile int pattern = iter;
        
#ifdef __AVX2__
        /* Test with 256-bit vectors */
        v8si int_vec1 = *(v8si*)&global_ints[iter * 8];
        v8si int_vec2 = *(v8si*)&global_ints[iter * 8 + 8];
        v8si int_vec3 = *(v8si*)&global_ints[iter * 8 + 16];
        v8si int_vec4 = *(v8si*)&global_ints[iter * 8 + 24];
        
        /* Create masks with runtime-dependent values */
        v8si mask1 = {0,2,4,6,1,3,5,7};
        v8si mask2 = {7,6,5,4,3,2,1,0};
        v8si mask3 = {3,2,1,0,7,6,5,4};
        
        /* Apply shuffle with many operands */
        v8si int_result = shuffle_10_operand_int(int_vec1, int_vec2, 
                                                 int_vec3, int_vec4,
                                                 mask1, mask2, mask3,
                                                 pattern);
        
        /* Accumulate results */
        int_accum_256[iter % 4] = int_accum_256[iter % 4] + int_result;
        
        /* Test floating-point shuffles */
        v8sf float_vec1 = *(v8sf*)&global_floats[iter * 8];
        v8sf float_vec2 = *(v8sf*)&global_floats[iter * 8 + 8];
        v8sf float_vec3 = *(v8sf*)&global_floats[iter * 8 + 16];
        
        v8sf float_result = shuffle_float_complex(float_vec1, float_vec2,
                                                  float_vec3, mask1, pattern);
        
        float_accum_256[iter % 4] = float_accum_256[iter % 4] + float_result;
#endif

#ifdef __AVX512F__
        /* Test with 512-bit vectors */
        v16si int_vec1_512 = *(v16si*)&global_ints[iter * 16];
        v16si int_vec2_512 = *(v16si*)&global_ints[iter * 16 + 16];
        v16si int_vec3_512 = *(v16si*)&global_ints[iter * 16 + 32];
        v16si int_vec4_512 = *(v16si*)&global_ints[iter * 16 + 48];
        
        v16si mask1_512 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        v16si mask2_512 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
        v16si mask3_512 = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
        
        volatile int idx1 = pattern * 2;
        volatile int idx2 = pattern * 3;
        
        v16si int_result_512 = shuffle_11_operand_int(int_vec1_512, int_vec2_512,
                                                      int_vec3_512, int_vec4_512,
                                                      mask1_512, mask2_512, mask3_512,
                                                      idx1, idx2);
        
        int_accum_512[iter % 4] = int_accum_512[iter % 4] + int_result_512;
        
        /* Test shufflevector with doubles */
        v8df double_vec1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        v8df double_vec2 = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
        
        v8df double_result = shufflevector_double_large(double_vec1, double_vec2, pattern);
        
        /* Convert and accumulate */
        for (int i = 0; i < 8; i++) {
            global_floats[iter * 8 + i] += (float)double_result[i];
        }
#endif
    }
    
    /* Compute final checksum */
    int64_t int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            int_checksum += ((int32_t*)&int_accum_256[i])[j];
            float_checksum += ((float*)&float_accum_256[i])[j];
        }
    }
    
#ifdef __AVX512F__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            int_checksum += ((int32_t*)&int_accum_512[i])[j];
        }
    }
#endif
    
    /* Also include global arrays in checksum */
    for (int i = 0; i < 100; i++) {
        int_checksum += global_ints[i];
        float_checksum += global_floats[i];
    }
    
    printf("Final checksum - Integer: %lld, Float: %f\n", 
           (long long)int_checksum, float_checksum);
    
    return 0;
}
