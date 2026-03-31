#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator_int[512];
float accumulator_float[512];

/* Vector type definitions using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));    /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));   /* 16x float (512-bit) */
typedef double v4df __attribute__((vector_size(32)));   /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));   /* 8x double (512-bit) */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        global_int_array[i] = (i * 1103515245 + seed) % 1000;
        global_float_array[i] = (float)((i * 1103515245 + seed) % 1000) / 10.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function using __builtin_shuffle with 10+ operands (256-bit integer) */
void shuffle_v8si_complex(v8si *result, const int *data, volatile int mask_idx) {
    v8si a = *(const v8si *)(data);
    v8si b = *(const v8si *)(data + 8);
    
    /* Create volatile control mask to prevent constant folding */
    volatile int mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = (mask_idx + i * 3) % 16;
    }
    
    /* Load mask from volatile array */
    int mask[8];
    for (int i = 0; i < 8; i++) mask[i] = mask_arr[i];
    
    /* This shuffle uses 10 operands: a, b, and 8 mask indices */
    v8si shuffled = __builtin_shuffle(a, b, 
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7]);
    
    /* Additional operation to ensure the result is used */
    *result = shuffled + a;
}

/* Function using __builtin_shuffle with 11+ operands (512-bit float) */
void shuffle_v16sf_complex(v16sf *result, const float *data, volatile int mask_idx) {
    v16sf a = *(const v16sf *)(data);
    v16sf b = *(const v16sf *)(data + 16);
    
    /* Volatile mask generation */
    volatile int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (mask_idx + i * 5) % 32;
    }
    
    /* Extract mask values */
    int mask[16];
    for (int i = 0; i < 16; i++) mask[i] = mask_arr[i];
    
    /* This shuffle uses 18 operands total - should hit large operand paths */
    v16sf shuffled = __builtin_shuffle(a, b,
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11],
        mask[12], mask[13], mask[14], mask[15]);
    
    *result = shuffled * a;
}

/* Mixed SIMD patterns with narrowing/expanding */
void mixed_simd_pattern(int *acc, const int *data, volatile int pattern) {
    /* Use different vector sizes based on pattern */
    switch (pattern % 4) {
        case 0: {
            /* 256-bit integer operations */
            v8si v1 = *(const v8si *)(data);
            v8si v2 = *(const v8si *)(data + 16);
            
            volatile int m[8];
            for (int i = 0; i < 8; i++) m[i] = (pattern + i) % 16;
            
            int mask[8];
            for (int i = 0; i < 8; i++) mask[i] = m[i];
            
            v8si shuffled = __builtin_shuffle(v1, v2,
                mask[0], mask[1], mask[2], mask[3],
                mask[4], mask[5], mask[6], mask[7]);
            
            v8si result = shuffled + v1 * v2;
            *(v8si *)acc = result;
            break;
        }
        case 1: {
            /* 512-bit float operations if supported */
#ifdef __AVX512F__
            v16sf v1 = *(const v16sf *)(data);
            v16sf v2 = *(const v16sf *)(data + 32);
            
            volatile int m[16];
            for (int i = 0; i < 16; i++) m[i] = (pattern + i * 2) % 32;
            
            int mask[16];
            for (int i = 0; i < 16; i++) mask[i] = m[i];
            
            v16sf shuffled = __builtin_shuffle(v1, v2,
                mask[0], mask[1], mask[2], mask[3],
                mask[4], mask[5], mask[6], mask[7],
                mask[8], mask[9], mask[10], mask[11],
                mask[12], mask[13], mask[14], mask[15]);
            
            v16sf result = shuffled + v1;
            *(v16sf *)acc = result;
#endif
            break;
        }
        case 2: {
            /* Double precision shuffle */
            v4df v1 = *(const v4df *)(data);
            v4df v2 = *(const v4df *)(data + 8);
            
            volatile int m[4];
            for (int i = 0; i < 4; i++) m[i] = (pattern + i * 7) % 8;
            
            int mask[4];
            for (int i = 0; i < 4; i++) mask[i] = m[i];
            
            v4df shuffled = __builtin_shuffle(v1, v2,
                mask[0], mask[1], mask[2], mask[3]);
            
            v4df result = shuffled * v1;
            *(v4df *)acc = (v4df)result;
            break;
        }
        case 3: {
            /* Mixed float/int operations */
            v8sf f1 = *(const v8sf *)(data);
            v8sf f2 = *(const v8sf *)(data + 8);
            
            volatile int m[8];
            for (int i = 0; i < 8; i++) m[i] = (pattern + i * 11) % 16;
            
            int mask[8];
            for (int i = 0; i < 8; i++) mask[i] = m[i];
            
            v8sf shuffled = __builtin_shuffle(f1, f2,
                mask[0], mask[1], mask[2], mask[3],
                mask[4], mask[5], mask[6], mask[7]);
            
            v8sf result = shuffled + f1 - f2;
            *(v8sf *)acc = result;
            break;
        }
    }
}

/* Function with nested control flow and vector operations */
void complex_control_flow(int *output, const int *input, volatile int control) {
    v8si temp_result;
    
    if (control & 1) {
        for (int i = 0; i < 4; i++) {
            if ((control >> i) & 1) {
                shuffle_v8si_complex(&temp_result, input + i * 8, control + i);
                
                /* Additional shuffle in loop */
                v8si a = *(const v8si *)(input + i * 8);
                volatile int mask_idx = control + i * 3;
                
                int mask[8];
                for (int j = 0; j < 8; j++) {
                    mask[j] = (mask_idx + j * 5) % 16;
                }
                
                v8si shuffled2 = __builtin_shuffle(a, temp_result,
                    mask[0], mask[1], mask[2], mask[3],
                    mask[4], mask[5], mask[6], mask[7]);
                
                temp_result = temp_result + shuffled2;
            }
        }
    } else {
        switch (control % 3) {
            case 0:
                shuffle_v8si_complex(&temp_result, input, control);
                break;
            case 1:
                /* Direct shuffle with many operands */
                v8si a = *(const v8si *)(input);
                v8si b = *(const v8si *)(input + 8);
                
                volatile int m[8];
                for (int i = 0; i < 8; i++) m[i] = (control + i * 13) % 16;
                
                int mask[8];
                for (int i = 0; i < 8; i++) mask[i] = m[i];
                
                temp_result = __builtin_shuffle(a, b,
                    mask[0], mask[1], mask[2], mask[3],
                    mask[4], mask[5], mask[6], mask[7]);
                break;
            case 2:
                /* Chain of shuffles */
                v8si v1 = *(const v8si *)(input);
                v8si v2 = *(const v8si *)(input + 8);
                v8si v3 = *(const v8si *)(input + 16);
                
                volatile int m1[8], m2[8];
                for (int i = 0; i < 8; i++) {
                    m1[i] = (control + i) % 16;
                    m2[i] = (control + i * 7) % 16;
                }
                
                int mask1[8], mask2[8];
                for (int i = 0; i < 8; i++) {
                    mask1[i] = m1[i];
                    mask2[i] = m2[i];
                }
                
                v8si s1 = __builtin_shuffle(v1, v2,
                    mask1[0], mask1[1], mask1[2], mask1[3],
                    mask1[4], mask1[5], mask1[6], mask1[7]);
                
                v8si s2 = __builtin_shuffle(s1, v3,
                    mask2[0], mask2[1], mask2[2], mask2[3],
                    mask2[4], mask2[5], mask2[6], mask2[7]);
                
                temp_result = s1 + s2;
                break;
        }
    }
    
    *(v8si *)output = temp_result;
}

/* Architecture-specific variants */
#ifdef __AVX512F__
void avx512_specific(float *acc, const float *data, volatile int idx) {
    v16sf a = *(const v16sf *)(data);
    v16sf b = *(const v16sf *)(data + 16);
    
    /* Large mask that requires many operands */
    volatile int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (idx + i * 19) % 32;
    }
    
    int mask[16];
    for (int i = 0; i < 16; i++) mask[i] = mask_arr[i];
    
    /* This should trigger 18-operand expansion */
    v16sf result = __builtin_shuffle(a, b,
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11],
        mask[12], mask[13], mask[14], mask[15]);
    
    *(v16sf *)acc = result;
}
#endif

#ifdef __AVX2__
void avx2_specific(int *acc, const int *data, volatile int idx) {
    v8si a = *(const v8si *)(data);
    v8si b = *(const v8si *)(data + 8);
    v8si c = *(const v8si *)(data + 16);
    
    volatile int m1[8], m2[8];
    for (int i = 0; i < 8; i++) {
        m1[i] = (idx + i * 11) % 24;
        m2[i] = (idx + i * 17) % 24;
    }
    
    int mask1[8], mask2[8];
    for (int i = 0; i < 8; i++) {
        mask1[i] = m1[i];
        mask2[i] = m2[i];
    }
    
    /* Chain two shuffles with many operands each */
    v8si s1 = __builtin_shuffle(a, b,
        mask1[0], mask1[1], mask1[2], mask1[3],
        mask1[4], mask1[5], mask1[6], mask1[7]);
    
    v8si s2 = __builtin_shuffle(s1, c,
        mask2[0], mask2[1], mask2[2], mask2[3],
        mask2[4], mask2[5], mask2[6], mask2[7]);
    
    *(v8si *)acc = s1 + s2;
}
#endif

#ifdef __SSE2__
void sse2_specific(float *acc, const float *data, volatile int idx) {
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4sf a = *(const v4sf *)(data);
    v4sf b = *(const v4sf *)(data + 4);
    
    volatile int m[4];
    for (int i = 0; i < 4; i++) {
        m[i] = (idx + i * 3) % 8;
    }
    
    int mask[4];
    for (int i = 0; i < 4; i++) mask[i] = m[i];
    
    v4sf result = __builtin_shuffle(a, b,
        mask[0], mask[1], mask[2], mask[3]);
    
    *(v4sf *)acc = result;
}
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    init_arrays(seed);
    
    volatile int control = seed;
    
    /* Main loop with various vector operations */
    for (int iter = 0; iter < 10; iter++) {
        volatile int pattern = (control + iter * 97) % 100;
        
        /* Integer shuffles */
        for (int i = 0; i < 8; i++) {
            int offset = (iter * 32 + i * 8) % 448;
            v8si int_result;
            shuffle_v8si_complex(&int_result, &global_int_array[offset], pattern + i);
            
            /* Accumulate results */
            for (int j = 0; j < 8; j++) {
                accumulator_int[offset + j] += ((int *)&int_result)[j];
            }
        }
        
        /* Mixed SIMD patterns */
        for (int i = 0; i < 4; i++) {
            int offset = (iter * 64 + i * 16) % 448;
            mixed_simd_pattern(&accumulator_int[offset], &global_int_array[offset], pattern + i);
        }
        
        /* Complex control flow */
        for (int i = 0; i < 2; i++) {
            int offset = (iter * 128 + i * 32) % 384;
            complex_control_flow(&accumulator_int[offset], &global_int_array[offset], pattern + i * 17);
        }
        
        /* Architecture-specific paths */
#ifdef __AVX512F__
        for (int i = 0; i < 2; i++) {
            int offset = (iter * 128 + i * 64) % 384;
            avx512_specific(&accumulator_float[offset], &global_float_array[offset], pattern + i);
        }
#endif
        
#ifdef __AVX2__
        for (int i = 0; i < 4; i++) {
            int offset = (iter * 64 + i * 32) % 448;
            avx2_specific(&accumulator_int[offset], &global_int_array[offset], pattern + i * 13);
        }
#endif
        
#ifdef __SSE2__
        for (int i = 0; i < 8; i++) {
            int offset = (iter * 32 + i * 16) % 480;
            sse2_specific(&accumulator_float[offset], &global_float_array[offset], pattern + i * 7);
        }
#endif
        
        /* Float shuffles with many operands */
        for (int i = 0; i < 2; i++) {
            int offset = (iter * 128 + i * 64) % 384;
            v16sf float_result;
            shuffle_v16sf_complex(&float_result, &global_float_array[offset], pattern + i * 23);
            
            /* Accumulate float results */
            for (int j = 0; j < 16; j++) {
                accumulator_float[offset + j] += ((float *)&float_result)[j];
            }
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long int_checksum = 0;
    double float_checksum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
        
        /* Volatile store to memory */
        volatile int store_int = accumulator_int[i];
        volatile float store_float = accumulator_float[i];
        (void)store_int;
        (void)store_float;
    }
    
    printf("Integer checksum: %lld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
