#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int global_seed;
static int data_int[512];
static float data_float[512];
static int64_t accumulator[512] = {0};

/* Vector type definitions using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));    /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));   /* 16x float (512-bit) */
typedef double v4df __attribute__((vector_size(32)));   /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));   /* 8x double (512-bit) */

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        data_int[i] = (i * 1103515245 + 12345) & 0x7FFF;
        data_float[i] = (float)((i * 1103515245 + 12345) & 0x7FFF) / 32768.0f;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
void shuffle_10_operand_int(v16si *result, const int *src, volatile int *mask_indices) {
    /* Load 4 vectors (16 elements each) */
    v16si v0 = *(const v16si *)(src + 0);
    v16si v1 = *(const v16si *)(src + 16);
    v16si v2 = *(const v16si *)(src + 32);
    v16si v3 = *(const v16si *)(src + 48);
    
    /* Create control mask with volatile indices to prevent constant folding */
    volatile int idx[16];
    for (int i = 0; i < 16; i++) {
        idx[i] = mask_indices[i] & 0x3F;  /* Limit to 0-63 (4 vectors * 16 elements) */
    }
    
    /* Complex shuffle pattern that may require 10+ operands during expansion */
    /* The shuffle indices are computed from volatile variables */
    v16si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
                  idx[8], idx[9], idx[10], idx[11], idx[12], idx[13], idx[14], idx[15]};
    
    /* This shuffle with 3 input vectors and a mask may expand to many operands */
    v16si shuffled = __builtin_shuffle(v0, v1, v2, v3, mask);
    
    /* Additional arithmetic to ensure the result is used */
    v16si scaled = shuffled + (v16si){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    
    *result = scaled;
}

/* Another variant with floating-point vectors */
void shuffle_11_operand_float(v16sf *result, const float *src, volatile int *mask_indices) {
    /* Load multiple vectors */
    v16sf f0 = *(const v16sf *)(src + 0);
    v16sf f1 = *(const v16sf *)(src + 16);
    v16sf f2 = *(const v16sf *)(src + 32);
    v16sf f3 = *(const v16sf *)(src + 48);
    
    /* Volatile mask computation */
    volatile int idx[16];
    for (int i = 0; i < 16; i++) {
        idx[i] = (mask_indices[i] * 3 + i) & 0x3F;
    }
    
    v16si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
                  idx[8], idx[9], idx[10], idx[11], idx[12], idx[13], idx[14], idx[15]};
    
    /* Complex shuffle that may require 11 operands */
    v16sf temp1 = __builtin_shuffle(f0, f1, f2, mask);
    v16sf temp2 = __builtin_shuffle(f2, f3, f0, mask);
    
    /* Mix results */
    v16sf shuffled = temp1 + temp2;
    
    /* Scale and store */
    v16sf scaled = shuffled * (v16sf){1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f,
                                      1.9f, 2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f};
    
    *result = scaled;
}

/* Function with nested control flow around vector operations */
void conditional_shuffle(v8si *result, const int *src, volatile int control) {
    v8si v0 = *(const v8si *)(src + 0);
    v8si v1 = *(const v8si *)(src + 8);
    v8si v2 = *(const v8si *)(src + 16);
    v8si v3 = *(const v8si *)(src + 24);
    
    /* Complex control flow to stress the expander */
    if (control & 1) {
        volatile int idx[8] = {0, 7, 1, 6, 2, 5, 3, 4};
        v8si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7]};
        
        /* Switch statement with different shuffle patterns */
        switch (control & 3) {
            case 0:
                *result = __builtin_shuffle(v0, v1, v2, mask);
                break;
            case 1:
                *result = __builtin_shuffle(v1, v2, v3, mask);
                break;
            case 2:
                *result = __builtin_shuffle(v2, v3, v0, mask);
                break;
            default:
                *result = __builtin_shuffle(v3, v0, v1, mask);
                break;
        }
    } else {
        /* Alternative shuffle pattern */
        volatile int idx[8] = {7, 0, 6, 1, 5, 2, 4, 3};
        v8si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7]};
        *result = __builtin_shuffle(v0, v1, v2, v3, mask);
    }
}

/* Mixed SIMD patterns with type conversions */
void mixed_simd_operations(int64_t *acc, const int *src_int, const float *src_float, volatile int iter) {
#ifdef __AVX512F__
    /* Use 512-bit vectors if available */
    v16si vi = *(const v16si *)(src_int + iter * 16);
    v16sf vf = *(const v16sf *)(src_float + iter * 16);
    
    volatile int idx[16];
    for (int i = 0; i < 16; i++) {
        idx[i] = (iter + i) & 0x1F;
    }
    
    v16si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
                  idx[8], idx[9], idx[10], idx[11], idx[12], idx[13], idx[14], idx[15]};
    
    /* Complex operation chain */
    v16si shuffled_i = __builtin_shuffle(vi, vi, vi, vi, mask);
    v16sf shuffled_f = __builtin_shuffle(vf, vf, vf, vf, mask);
    
    /* Convert and accumulate */
    for (int i = 0; i < 16; i++) {
        acc[iter * 16 + i] += shuffled_i[i] + (int64_t)shuffled_f[i];
    }
#elif defined(__AVX2__)
    /* Use 256-bit vectors */
    v8si vi = *(const v8si *)(src_int + iter * 8);
    v8sf vf = *(const v8sf *)(src_float + iter * 8);
    
    volatile int idx[8];
    for (int i = 0; i < 8; i++) {
        idx[i] = (iter * 2 + i) & 0xF;
    }
    
    v8si mask = {idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7]};
    
    v8si shuffled_i = __builtin_shuffle(vi, vi, vi, mask);
    v8sf shuffled_f = __builtin_shuffle(vf, vf, vf, mask);
    
    for (int i = 0; i < 8; i++) {
        acc[iter * 8 + i] += shuffled_i[i] + (int64_t)shuffled_f[i];
    }
#elif defined(__SSE2__)
    /* Use 128-bit vectors */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si vi = *(const v4si *)(src_int + iter * 4);
    v4sf vf = *(const v4sf *)(src_float + iter * 4);
    
    volatile int idx[4];
    for (int i = 0; i < 4; i++) {
        idx[i] = (iter * 3 + i) & 0x7;
    }
    
    v4si mask = {idx[0], idx[1], idx[2], idx[3]};
    
    v4si shuffled_i = __builtin_shuffle(vi, vi, mask);
    v4sf shuffled_f = __builtin_shuffle(vf, vf, mask);
    
    for (int i = 0; i < 4; i++) {
        acc[iter * 4 + i] += shuffled_i[i] + (int64_t)shuffled_f[i];
    }
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    init_data(seed);
    
    volatile int control_mask[16];
    for (int i = 0; i < 16; i++) {
        control_mask[i] = (seed + i * 17) & 0xFF;
    }
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        /* Call different shuffle functions with complex patterns */
        v16si int_result;
        v16sf float_result;
        v8si cond_result;
        
        shuffle_10_operand_int(&int_result, data_int + iter * 16, control_mask);
        shuffle_11_operand_float(&float_result, data_float + iter * 16, control_mask);
        conditional_shuffle(&cond_result, data_int + iter * 32, control_mask[iter & 0xF]);
        
        /* Store results to prevent elimination */
        volatile v16si store_int = int_result;
        volatile v16sf store_float = float_result;
        volatile v8si store_cond = cond_result;
        
        /* Accumulate integer results */
        for (int i = 0; i < 16; i++) {
            accumulator[iter * 16 + i] += store_int[i];
        }
        
        /* Mixed SIMD operations */
        mixed_simd_operations(accumulator, data_int, data_float, iter);
        
        /* Update control mask for next iteration */
        for (int i = 0; i < 16; i++) {
            control_mask[i] = (control_mask[i] * 13 + 7) & 0xFF;
        }
    }
    
    /* Compute final checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
        checksum = (checksum << 5) | (checksum >> 59);  /* Simple mixing */
    }
    
    printf("Result checksum: %lld\n", (long long)checksum);
    return 0;
}
