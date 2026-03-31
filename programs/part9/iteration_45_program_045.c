#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Global data arrays */
static int32_t global_ints[512];
static float global_floats[512];
static double global_doubles[512];

/* Accumulator arrays */
static int32_t accum_ints[512];
static float accum_floats[512];
static double accum_doubles[512];

/* Volatile variables to prevent constant folding */
static volatile int volatile_mask_seed = 0;

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (float)(rand() % 1000) / 10.0f;
        global_doubles[i] = (double)(rand() % 1000) / 10.0;
        accum_ints[i] = 0;
        accum_floats[i] = 0.0f;
        accum_doubles[i] = 0.0;
    }
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
static v8si shuffle_10_operand_int(v8si a, v8si b, v8si mask1, v8si mask2, 
                                   v8si mask3, v8si mask4, int idx) {
    /* Complex control flow to stress expander */
    v8si result;
    
    if (volatile_mask_seed > 100) {
        /* First shuffle with 10 operands: 2 input vectors + 8-element mask */
        v8si temp = __builtin_shuffle(a, b, mask1);
        
        /* Nested control flow */
        switch (idx % 4) {
            case 0:
                /* Second shuffle with different mask */
                result = __builtin_shuffle(temp, a, mask2);
                break;
            case 1:
                /* Third shuffle */
                result = __builtin_shuffle(b, temp, mask3);
                break;
            default:
                /* Fourth shuffle */
                result = __builtin_shuffle(a, b, mask4);
                break;
        }
    } else {
        /* Alternative path with arithmetic */
        result = a + b;
    }
    
    /* Additional operation to prevent elimination */
    result = result * (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    
    return result;
}
#endif

#ifdef __AVX512F__
/* Function targeting 11+ operands with __builtin_shufflevector */
static v16si shufflevector_11_operand_int(v16si a, v16si b, v16si c, 
                                          v16si mask1, v16si mask2, int idx) {
    v16si result;
    
    /* Use volatile to force runtime evaluation */
    volatile int v_idx = idx;
    
    if (v_idx & 1) {
        /* __builtin_shufflevector with 3 input vectors + 16-element mask = 19 operands */
        /* The expander will need to handle many operands */
        v16si temp = __builtin_shufflevector(a, b, c,
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        
        /* Second shuffle with different pattern */
        result = __builtin_shufflevector(temp, a, b,
            8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
    } else {
        /* Alternative: __builtin_shuffle with 2 vectors + 16-element mask = 18 operands */
        result = __builtin_shuffle(a, b, mask1);
    }
    
    /* Mix with mask2 using arithmetic */
    result = result + (mask2 & (v16si){1, 2, 3, 4, 5, 6, 7, 8, 
                                       9, 10, 11, 12, 13, 14, 15, 16});
    
    return result;
}
#endif

/* Mixed floating-point shuffle function */
#ifdef __AVX2__
static v8sf shuffle_mixed_float(v8sf a, v8sf b, v8si int_mask, v8sf float_mask, int idx) {
    v8sf result;
    
    /* Convert integer mask to appropriate type for shuffle */
    v8si converted_mask = int_mask + (v8si){idx, idx+1, idx+2, idx+3, 
                                            idx+4, idx+5, idx+6, idx+7};
    
    /* Complex control flow */
    for (int i = 0; i < 3; i++) {
        if (i == (idx % 3)) {
            /* Shuffle with mixed-type operations */
            v8sf temp = __builtin_shuffle(a, b, converted_mask);
            result = temp * float_mask;
        } else if (i == ((idx + 1) % 3)) {
            /* Alternative shuffle pattern */
            v8sf temp = __builtin_shuffle(b, a, converted_mask);
            result = temp + float_mask;
        } else {
            /* Default: simple operation */
            result = a + b;
        }
        
        /* Volatile store to prevent elimination */
        volatile v8sf volatile_store = result;
        (void)volatile_store;
    }
    
    return result;
}
#endif

/* Double precision shuffle with many operands */
#ifdef __AVX512F__
static v8df shuffle_double_11_operand(v8df a, v8df b, v8df c, 
                                      v8si mask1, v8df mask2, int idx) {
    v8df result;
    
    /* Switch statement to create complex CFG */
    switch (idx % 5) {
        case 0:
            /* __builtin_shufflevector with 3 vectors + 8-element mask = 11 operands */
            result = __builtin_shufflevector(a, b, c,
                0, 8, 1, 9, 2, 10, 3, 11);
            break;
        case 1:
            /* Alternative pattern */
            result = __builtin_shufflevector(b, c, a,
                4, 12, 5, 13, 6, 14, 7, 15);
            break;
        case 2:
            /* __builtin_shuffle with 2 vectors + 8-element mask = 10 operands */
            result = __builtin_shuffle(a, b, mask1);
            break;
        case 3:
            /* Mix with mask2 */
            result = __builtin_shuffle(a, b, mask1) * mask2;
            break;
        default:
            /* Complex nested operation */
            v8df temp1 = __builtin_shuffle(a, b, mask1);
            v8df temp2 = __builtin_shuffle(b, c, mask1);
            result = temp1 + temp2;
            break;
    }
    
    return result;
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Set volatile seed from command line */
    if (argc > 2) {
        volatile_mask_seed = atoi(argv[2]);
    }
    
    int64_t int_checksum = 0;
    double float_checksum = 0.0;
    double double_checksum = 0.0;
    
    /* Perform multiple iterations with different data slices */
    for (int iter = 0; iter < 10; iter++) {
        int base_idx = (iter * 16) % 480;
        
#ifdef __AVX2__
        /* Test with 256-bit vectors */
        v8si int_vec1 = *(v8si*)&global_ints[base_idx];
        v8si int_vec2 = *(v8si*)&global_ints[base_idx + 8];
        v8si mask1 = {0, 2, 4, 6, 1, 3, 5, 7};
        v8si mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si mask3 = {1, 3, 5, 7, 0, 2, 4, 6};
        v8si mask4 = {2, 4, 6, 0, 3, 5, 7, 1};
        
        /* Call 10-operand shuffle function */
        v8si int_result = shuffle_10_operand_int(int_vec1, int_vec2, 
                                                 mask1, mask2, mask3, mask4, iter);
        
        /* Accumulate results */
        for (int i = 0; i < 8; i++) {
            accum_ints[base_idx + i] += int_result[i];
        }
        
        /* Test mixed float shuffles */
        v8sf float_vec1 = *(v8sf*)&global_floats[base_idx];
        v8sf float_vec2 = *(v8sf*)&global_floats[base_idx + 8];
        v8sf float_mask = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        
        v8sf float_result = shuffle_mixed_float(float_vec1, float_vec2, 
                                                mask1, float_mask, iter);
        
        for (int i = 0; i < 8; i++) {
            accum_floats[base_idx + i] += float_result[i];
        }
#endif

#ifdef __AVX512F__
        /* Test with 512-bit vectors for 11+ operands */
        if (iter % 2 == 0) {
            v16si int_vec1_512 = *(v16si*)&global_ints[base_idx];
            v16si int_vec2_512 = *(v16si*)&global_ints[base_idx + 16];
            v16si int_vec3_512 = *(v16si*)&global_ints[base_idx + 32];
            v16si mask1_512 = {0, 16, 1, 17, 2, 18, 3, 19, 
                               4, 20, 5, 21, 6, 22, 7, 23};
            v16si mask2_512 = {15, 14, 13, 12, 11, 10, 9, 8,
                               7, 6, 5, 4, 3, 2, 1, 0};
            
            v16si int_result_512 = shufflevector_11_operand_int(
                int_vec1_512, int_vec2_512, int_vec3_512, 
                mask1_512, mask2_512, iter);
            
            for (int i = 0; i < 16; i++) {
                accum_ints[base_idx + i] += int_result_512[i];
            }
        }
        
        /* Test double precision shuffles */
        v8df double_vec1 = *(v8df*)&global_doubles[base_idx];
        v8df double_vec2 = *(v8df*)&global_doubles[base_idx + 8];
        v8df double_vec3 = *(v8df*)&global_doubles[base_idx + 16];
        v8df double_mask = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        
        v8df double_result = shuffle_double_11_operand(
            double_vec1, double_vec2, double_vec3, mask1, double_mask, iter);
        
        for (int i = 0; i < 8; i++) {
            accum_doubles[base_idx + i] += double_result[i];
        }
#endif
    }
    
    /* Compute checksums */
    for (int i = 0; i < 512; i++) {
        int_checksum += accum_ints[i];
        float_checksum += accum_floats[i];
        double_checksum += accum_doubles[i];
    }
    
    printf("Integer checksum: %ld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    printf("Double checksum: %f\n", double_checksum);
    
    return 0;
}
