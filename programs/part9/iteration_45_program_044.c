#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int32_t global_int_data[512];
volatile float global_float_data[512];
int32_t accumulator_int[512];
float accumulator_float[512];

/* Initialize with deterministic pseudo-random sequence */
void init_data(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = rand() % 1000;
        global_float_data[i] = (rand() % 1000) / 10.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));        /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float (512-bit) */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double (512-bit) */

/* Function 1: Complex shuffle with 10+ operands using int32x16 */
__attribute__((noinline))
void shuffle_int16_with_many_operands(int offset, volatile int* mask_source) {
    /* Load 512-bit vector (16 ints) */
    v16si vec1 = *(v16si*)&global_int_data[offset];
    v16si vec2 = *(v16si*)&global_int_data[offset + 16];
    
    /* Create control mask from volatile source to prevent constant folding */
    int32_t mask_data[16];
    for (int i = 0; i < 16; i++) {
        mask_data[i] = mask_source[i] & 0x1F; /* Keep within 0-31 range */
    }
    v16si mask = *(v16si*)mask_data;
    
    /* Complex shuffle operation that may require 10+ operands during expansion */
    v16si result;
    
    /* Use volatile condition to prevent optimization */
    volatile int use_complex_shuffle = 1;
    
    if (use_complex_shuffle) {
        /* This shuffle with mask may expand to many operands */
        result = __builtin_shuffle(vec1, vec2, mask);
        
        /* Additional arithmetic to ensure the result is used */
        result = result + vec1;
        result = result * (v16si){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    } else {
        result = vec1;
    }
    
    /* Store back to accumulator */
    v16si* acc_ptr = (v16si*)&accumulator_int[offset];
    *acc_ptr = *acc_ptr + result;
}

/* Function 2: Float shuffle with mixed types and control flow */
__attribute__((noinline))
void shuffle_float8_with_control_flow(int offset, volatile int pattern) {
    v8sf vec1 = *(v8sf*)&global_float_data[offset];
    v8sf vec2 = *(v8sf*)&global_float_data[offset + 8];
    
    /* Different shuffle patterns based on runtime value */
    v8sf result;
    
    switch (pattern & 0x3) {
        case 0: {
            /* Complex pattern 0 - may require many operands */
            int32_t mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
            v8si int_mask = *(v8si*)mask;
            result = __builtin_shuffle(vec1, vec2, int_mask);
            break;
        }
        case 1: {
            /* Pattern 1 with __builtin_shufflevector */
            result = __builtin_shufflevector(vec1, vec2, 
                0, 8, 1, 9, 2, 10, 3, 11);
            break;
        }
        case 2: {
            /* Pattern 2 - more complex shuffle */
            int32_t mask[8] = {3, 2, 1, 0, 7, 6, 5, 4};
            v8si int_mask = *(v8si*)mask;
            result = __builtin_shuffle(vec1, vec2, int_mask);
            
            /* Nested shuffle to increase operand count */
            v8sf temp = __builtin_shuffle(result, vec1, int_mask);
            result = result + temp;
            break;
        }
        default: {
            /* Default pattern - identity */
            result = vec1;
            break;
        }
    }
    
    /* Store with arithmetic operation */
    v8sf* acc_ptr = (v8sf*)&accumulator_float[offset];
    *acc_ptr = *acc_ptr + result * 1.5f;
}

/* Function 3: Double precision shuffle with loop-dependent masks */
__attribute__((noinline))
void shuffle_double8_complex(int offset, int iter) {
    v8df vec1 = *(v8df*)&global_float_data[offset];     /* Using float array as doubles */
    v8df vec2 = *(v8df*)&global_float_data[offset + 8];
    
    /* Create mask based on iteration to prevent constant folding */
    int64_t mask_data[8];
    for (int i = 0; i < 8; i++) {
        mask_data[i] = (iter + i) % 16;
    }
    v8df mask_vec = *(v8df*)mask_data;
    
    /* Convert mask to appropriate type for shuffle */
    typedef int64_t v8di __attribute__((vector_size(64)));
    v8di mask = *(v8di*)mask_data;
    
    /* Complex operation sequence */
    v8df shuffled1 = __builtin_shuffle(vec1, vec2, mask);
    
    /* Second shuffle with different pattern */
    int64_t mask2_data[8];
    for (int i = 0; i < 8; i++) {
        mask2_data[i] = (iter * 2 + i) % 16;
    }
    v8di mask2 = *(v8di*)mask2_data;
    
    v8df shuffled2 = __builtin_shuffle(vec1, vec2, mask2);
    
    /* Combine results */
    v8df result = shuffled1 * 0.5 + shuffled2 * 0.5;
    
    /* Store to float accumulator (converting double to float) */
    for (int i = 0; i < 8; i++) {
        accumulator_float[offset + i] += (float)result[i];
    }
}

/* Function 4: Mixed width operations - narrowing and expanding */
__attribute__((noinline))
void mixed_width_shuffles(int offset) {
    /* Start with 512-bit vector */
    v16si wide_vec = *(v16si*)&global_int_data[offset];
    
    /* Narrow to 256-bit using shuffle */
    v8si narrow_mask = (v8si){0, 2, 4, 6, 8, 10, 12, 14};
    v8si narrow1 = __builtin_shuffle(wide_vec, narrow_mask);
    
    /* Another narrow vector */
    v8si narrow2 = __builtin_shuffle(wide_vec, (v8si){1, 3, 5, 7, 9, 11, 13, 15});
    
    /* Expand back to 512-bit using shufflevector with many operands */
    v16si expanded = __builtin_shufflevector(narrow1, narrow2,
        0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
    
    /* Store result */
    v16si* acc_ptr = (v16si*)&accumulator_int[offset];
    *acc_ptr = *acc_ptr + expanded;
}

/* Architecture-specific variants */
#ifdef __AVX512F__
__attribute__((noinline))
void avx512_specific_shuffle(int offset) {
    v16si vec1 = *(v16si*)&global_int_data[offset];
    v16si vec2 = *(v16si*)&global_int_data[offset + 16];
    
    /* Complex mask that might trigger 11-operand expansion */
    int32_t complex_mask[16];
    for (int i = 0; i < 16; i++) {
        complex_mask[i] = (i * 3) % 32;
    }
    
    v16si mask = *(v16si*)complex_mask;
    v16si result = __builtin_shuffle(vec1, vec2, mask);
    
    /* Multiple operations to increase register pressure */
    result = result + vec1 - vec2;
    result = result * (v16si){3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    
    v16si* acc_ptr = (v16si*)&accumulator_int[offset];
    *acc_ptr = *acc_ptr + result;
}
#endif

#ifdef __AVX2__
__attribute__((noinline))
void avx2_specific_shuffle(int offset) {
    v8df vec1 = *(v8df*)&global_float_data[offset];
    v8df vec2 = *(v8df*)&global_float_data[offset + 8];
    
    /* Shuffle pattern that might need many operands */
    int64_t mask[8] = {0, 8, 2, 10, 4, 12, 6, 14};
    typedef int64_t v8di __attribute__((vector_size(64)));
    v8di shuffle_mask = *(v8di*)mask;
    
    v8df result = __builtin_shuffle(vec1, vec2, shuffle_mask);
    
    /* Store as floats */
    for (int i = 0; i < 8; i++) {
        accumulator_float[offset + i] += (float)result[i];
    }
}
#endif

#ifdef __SSE2__
__attribute__((noinline))
void sse2_specific_shuffle(int offset) {
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf vec1 = *(v4sf*)&global_float_data[offset];
    v4sf vec2 = *(v4sf*)&global_float_data[offset + 4];
    
    /* Multiple shuffle operations in sequence */
    int32_t mask1[4] = {3, 2, 1, 0};
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si shuffle_mask1 = *(v4si*)mask1;
    
    v4sf result1 = __builtin_shuffle(vec1, vec2, shuffle_mask1);
    
    int32_t mask2[4] = {0, 4, 1, 5};
    v4si shuffle_mask2 = *(v4si*)mask2;
    v4sf result2 = __builtin_shuffle(vec1, vec2, shuffle_mask2);
    
    v4sf result = result1 + result2;
    
    /* Accumulate */
    v4sf* acc_ptr = (v4sf*)&accumulator_float[offset];
    *acc_ptr = *acc_ptr + result;
}
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    init_data(seed);
    
    /* Volatile mask source to prevent constant folding */
    volatile int mask_source[16];
    for (int i = 0; i < 16; i++) {
        mask_source[i] = (seed + i * 7) % 32;
    }
    
    /* Main loop with different shuffle operations */
    for (int iter = 0; iter < 10; iter++) {
        for (int offset = 0; offset < 256; offset += 32) {
            /* Call different shuffle functions */
            shuffle_int16_with_many_operands(offset, mask_source);
            shuffle_float8_with_control_flow(offset, iter);
            
            if (iter % 3 == 0) {
                shuffle_double8_complex(offset, iter);
            }
            
            if (iter % 2 == 0) {
                mixed_width_shuffles(offset);
            }
            
            /* Architecture-specific calls */
#ifdef __AVX512F__
            if (offset % 64 == 0) {
                avx512_specific_shuffle(offset);
            }
#endif
            
#ifdef __AVX2__
            if (offset % 32 == 0) {
                avx2_specific_shuffle(offset);
            }
#endif
            
#ifdef __SSE2__
            if (offset % 16 == 0) {
                sse2_specific_shuffle(offset);
            }
#endif
        }
        
        /* Modify mask source slightly each iteration */
        for (int i = 0; i < 16; i++) {
            mask_source[i] = (mask_source[i] + 1) % 32;
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
    }
    
    printf("Integer checksum: %ld\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    /* Additional volatile store to ensure all operations complete */
    volatile int final_check = (int)int_sum + (int)float_sum;
    printf("Final combined: %d\n", final_check);
    
    return 0;
}
