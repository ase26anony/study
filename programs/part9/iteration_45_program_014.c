#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int32_t global_int_data[512];
volatile float global_float_data[512];
int32_t accumulator_int[512];
float accumulator_float[512];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef int64_t v4di __attribute__((vector_size(32)));      /* 256-bit double int */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = rand() % 1000;
        global_float_data[i] = (float)(rand() % 1000) / 10.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function using __builtin_shuffle with 10+ operands - integer version */
#ifdef __AVX2__
void shuffle_int_10_operand(volatile int32_t* src, int32_t* dest, 
                           int mask_idx, int loop_counter) {
    /* Load 8-element vectors */
    v8si a = *(v8si*)&src[0];
    v8si b = *(v8si*)&src[8];
    v8si c = *(v8si*)&src[16];
    v8si d = *(v8si*)&src[24];
    
    /* Create volatile mask to prevent constant folding */
    volatile int mask_data[8];
    for (int i = 0; i < 8; i++) {
        mask_data[i] = (mask_idx + i * loop_counter) % 16;
    }
    
    /* Complex control flow to stress expander */
    if (mask_idx % 3 == 0) {
        /* Case 1: Direct shuffle with 10 operands */
        v8si result = __builtin_shuffle(a, b, 
            (v8si){mask_data[0], mask_data[1], mask_data[2], mask_data[3],
                   mask_data[4], mask_data[5], mask_data[6], mask_data[7]});
        
        /* Perform arithmetic to ensure operation isn't eliminated */
        result = result + (v8si){1, 2, 3, 4, 5, 6, 7, 8};
        *(v8si*)&dest[0] = result;
    } 
    else if (mask_idx % 3 == 1) {
        /* Case 2: Nested shuffles */
        v8si temp = __builtin_shuffle(a, c, 
            (v8si){mask_data[0] % 8, mask_data[1] % 8, mask_data[2] % 8, 
                   mask_data[3] % 8, mask_data[4] % 8, mask_data[5] % 8,
                   mask_data[6] % 8, mask_data[7] % 8});
        
        v8si result = __builtin_shuffle(temp, d,
            (v8si){(mask_data[0] + 1) % 8, (mask_data[1] + 1) % 8,
                   (mask_data[2] + 1) % 8, (mask_data[3] + 1) % 8,
                   (mask_data[4] + 1) % 8, (mask_data[5] + 1) % 8,
                   (mask_data[6] + 1) % 8, (mask_data[7] + 1) % 8});
        
        result = result * (v8si){2, 2, 2, 2, 2, 2, 2, 2};
        *(v8si*)&dest[0] = result;
    }
    else {
        /* Case 3: Complex shuffle chain */
        v8si temp1 = __builtin_shuffle(a, b,
            (v8si){mask_data[0] % 8, mask_data[1] % 8, 2, 3, 4, 5, 6, 7});
        
        v8si temp2 = __builtin_shuffle(c, d,
            (v8si){0, 1, mask_data[2] % 8, mask_data[3] % 8, 
                   mask_data[4] % 8, mask_data[5] % 8, 6, 7});
        
        v8si result = __builtin_shuffle(temp1, temp2,
            (v8si){mask_data[0] % 8, mask_data[1] % 8, mask_data[2] % 8 + 8,
                   mask_data[3] % 8 + 8, mask_data[4] % 8, mask_data[5] % 8,
                   mask_data[6] % 8 + 8, mask_data[7] % 8 + 8});
        
        *(v8si*)&dest[0] = result;
    }
}
#endif

#ifdef __AVX512F__
/* Function targeting 11+ operands with 512-bit vectors */
void shuffle_float_11_operand(volatile float* src, float* dest,
                             int mask_idx, int loop_counter) {
    /* Load 16-element vectors */
    v16sf a = *(v16sf*)&src[0];
    v16sf b = *(v16sf*)&src[16];
    v16sf c = *(v16sf*)&src[32];
    
    /* Volatile mask with 16 elements */
    volatile int mask_data[16];
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_idx * 3 + i * loop_counter) % 32;
    }
    
    /* Switch statement to create complex control flow */
    switch (mask_idx % 4) {
        case 0: {
            /* 11-operand pattern: shuffle with large mask */
            v16sf result = __builtin_shuffle(a, b,
                (v16si){mask_data[0], mask_data[1], mask_data[2], mask_data[3],
                       mask_data[4], mask_data[5], mask_data[6], mask_data[7],
                       mask_data[8], mask_data[9], mask_data[10], mask_data[11],
                       mask_data[12], mask_data[13], mask_data[14], mask_data[15]});
            
            result = result * (v16sf){1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
                                     1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f};
            *(v16sf*)&dest[0] = result;
            break;
        }
        case 1: {
            /* Mixed shufflevector with 10+ arguments */
            v16sf temp = __builtin_shufflevector(a, b, c,
                0, 16, 2, 18, 4, 20, 6, 22, 8, 24, 10, 26, 12, 28, 14, 30);
            
            /* Additional operation to prevent elimination */
            temp = temp + (v16sf){0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f,
                                 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f};
            *(v16sf*)&dest[0] = temp;
            break;
        }
        case 2: {
            /* Complex nested shuffles */
            v16sf temp1 = __builtin_shuffle(a, b,
                (v16si){mask_data[0] % 16, mask_data[1] % 16, 2, 3, 4, 5, 6, 7,
                       8, 9, 10, 11, 12, 13, 14, 15});
            
            v16sf temp2 = __builtin_shuffle(b, c,
                (v16si){0, 1, mask_data[2] % 16 + 16, mask_data[3] % 16 + 16,
                       4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
            
            v16sf result = __builtin_shuffle(temp1, temp2,
                (v16si){mask_data[4], mask_data[5], mask_data[6], mask_data[7],
                       mask_data[8], mask_data[9], mask_data[10], mask_data[11],
                       mask_data[12], mask_data[13], mask_data[14], mask_data[15],
                       0, 1, 2, 3});
            
            *(v16sf*)&dest[0] = result;
            break;
        }
        default: {
            /* Simple fallback that still uses shuffle */
            v16sf result = __builtin_shuffle(a, a,
                (v16si){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
            *(v16sf*)&dest[0] = result;
            break;
        }
    }
}
#endif

/* Mixed SIMD patterns with narrowing/expanding */
#ifdef __SSE2__
void mixed_simd_patterns(volatile int32_t* src, int32_t* dest, int idx) {
    /* Use smaller vectors to test conversion paths */
    typedef int32_t v4si __attribute__((vector_size(16)));
    
    v4si a = *(v4si*)&src[0];
    v4si b = *(v4si*)&src[4];
    
    volatile int mask[4];
    for (int i = 0; i < 4; i++) {
        mask[i] = (idx + i) % 8;
    }
    
    /* Narrowing then expanding pattern */
    v4si shuffled = __builtin_shuffle(a, b, 
        (v4si){mask[0], mask[1], mask[2], mask[3]});
    
    /* Store and reload to simulate boundary */
    *(v4si*)&dest[0] = shuffled;
    
    /* Additional shuffle to increase operand count in expansion */
    if (idx % 2 == 0) {
        v4si c = *(v4si*)&src[8];
        v4si final = __builtin_shuffle(shuffled, c,
            (v4si){mask[0] % 4, mask[1] % 4, mask[2] % 4 + 4, mask[3] % 4 + 4});
        *(v4si*)&dest[0] = final;
    }
}
#endif

/* Double precision shuffle with many operands */
#ifdef __AVX__
void double_shuffle_10_operand(volatile double* src, double* dest, int mask_idx) {
    v4df a = *(v4df*)&src[0];
    v4df b = *(v4df*)&src[4];
    
    volatile int mask[4];
    for (int i = 0; i < 4; i++) {
        mask[i] = (mask_idx * 5 + i * 3) % 8;
    }
    
    /* Complex if-else chain with shuffles */
    if (mask_idx < 10) {
        v4df result = __builtin_shuffle(a, b,
            (v4di){mask[0], mask[1], mask[2], mask[3]});
        
        /* Force memory barrier */
        asm volatile("" : : : "memory");
        
        result = result * (v4df){2.0, 2.0, 2.0, 2.0};
        *(v4df*)&dest[0] = result;
    } else if (mask_idx < 20) {
        v4df temp = __builtin_shuffle(a, a,
            (v4di){3, 2, 1, 0});
        
        v4df result = __builtin_shuffle(temp, b,
            (v4di){mask[0] % 4, mask[1] % 4, mask[2] % 4 + 4, mask[3] % 4 + 4});
        
        *(v4df*)&dest[0] = result;
    } else {
        v4df result = __builtin_shuffle(a, b,
            (v4di){0, 1, 2, 3});
        *(v4df*)&dest[0] = result;
    }
}
#endif

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        int base_idx = (iter * 37) % 256;  /* Non-linear progression */
        
        #ifdef __AVX2__
        shuffle_int_10_operand(&global_int_data[base_idx], 
                              &accumulator_int[base_idx], 
                              iter, base_idx);
        #endif
        
        #ifdef __AVX512F__
        shuffle_float_11_operand(&global_float_data[base_idx],
                                &accumulator_float[base_idx],
                                iter, base_idx);
        #endif
        
        #ifdef __SSE2__
        mixed_simd_patterns(&global_int_data[base_idx + 64],
                           &accumulator_int[base_idx + 64],
                           iter);
        #endif
        
        #ifdef __AVX__
        double_shuffle_10_operand((double*)&global_float_data[base_idx + 128],
                                 (double*)&accumulator_float[base_idx + 128],
                                 iter);
        #endif
        
        /* Additional volatile operation as barrier */
        volatile int barrier = iter;
        (void)barrier;
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
    }
    
    printf("Checksum - Integer: %ld, Float: %f\n", int_sum, float_sum);
    
    return 0;
}
