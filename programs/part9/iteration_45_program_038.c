#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static volatile int global_seed;
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static int32_t accumulator_int[ARRAY_SIZE];
static float accumulator_float[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int32_t v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
        global_float_array[i] = (float)((i * 1103515245 + seed) & 0x7FFF) / 32768.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function 1: Large integer shuffle with 10+ operands */
#ifdef __AVX2__
__attribute__((noinline))
void shuffle_int_avx2(int offset, volatile int mask_seed) {
    v8si* src1 = (v8si*)&global_int_array[offset];
    v8si* src2 = (v8si*)&global_int_array[offset + 8];
    v8si* src3 = (v8si*)&global_int_array[offset + 16];
    
    /* Create volatile mask to prevent constant folding */
    volatile int mask_val = mask_seed;
    v8si mask;
    for (int i = 0; i < 8; i++) {
        mask[i] = (mask_val + i) % 24;
    }
    
    /* Complex control flow to stress expander */
    if (mask_val & 1) {
        /* This shuffle uses 10 operands: 2 input vectors + 8 mask elements */
        v8si result = __builtin_shuffle(*src1, *src2, 
            mask[0], mask[1], mask[2], mask[3],
            mask[4], mask[5], mask[6], mask[7]);
        
        /* Additional operation to ensure the result is used */
        result = result + *src3;
        
        /* Store to volatile memory to prevent DCE */
        volatile v8si* dest = (volatile v8si*)&accumulator_int[offset];
        *dest = result;
    } else {
        /* Alternative path with different shuffle pattern */
        v8si temp = __builtin_shuffle(*src1, *src2, 7, 6, 5, 4, 3, 2, 1, 0);
        v8si result = __builtin_shuffle(temp, *src3,
            mask[0] % 8, mask[1] % 8, mask[2] % 8, mask[3] % 8,
            mask[4] % 8, mask[5] % 8, mask[6] % 8, mask[7] % 8);
        
        volatile v8si* dest = (volatile v8si*)&accumulator_int[offset];
        *dest = result;
    }
}
#endif

/* Function 2: Large float shuffle with 10+ operands */
#ifdef __AVX__
__attribute__((noinline))
void shuffle_float_avx(int offset, volatile int mask_seed) {
    v8sf* src1 = (v8sf*)&global_float_array[offset];
    v8sf* src2 = (v8sf*)&global_float_array[offset + 8];
    v8sf* src3 = (v8sf*)&global_float_array[offset + 16];
    
    volatile int mask_val = mask_seed;
    v8si mask;
    for (int i = 0; i < 8; i++) {
        mask[i] = (mask_val * (i + 1)) % 16;
    }
    
    /* Switch statement to create complex CFG */
    switch (mask_val & 3) {
        case 0:
            /* 10 operand case: 2 vectors + 8 mask values */
            {
                v8sf result = __builtin_shuffle(*src1, *src2,
                    mask[0], mask[1], mask[2], mask[3],
                    mask[4], mask[5], mask[6], mask[7]);
                result = result * *src3;
                volatile v8sf* dest = (volatile v8sf*)&accumulator_float[offset];
                *dest = result;
            }
            break;
            
        case 1:
            /* Nested shuffle operations */
            {
                v8sf temp1 = __builtin_shuffle(*src1, *src2, 0, 1, 2, 3, 4, 5, 6, 7);
                v8sf temp2 = __builtin_shuffle(*src2, *src3, 7, 6, 5, 4, 3, 2, 1, 0);
                v8sf result = __builtin_shuffle(temp1, temp2,
                    mask[0] % 8, mask[1] % 8, mask[2] % 8, mask[3] % 8,
                    mask[4] % 8, mask[5] % 8, mask[6] % 8, mask[7] % 8);
                volatile v8sf* dest = (volatile v8sf*)&accumulator_float[offset];
                *dest = result;
            }
            break;
            
        default:
            /* Direct store */
            volatile v8sf* dest = (volatile v8sf*)&accumulator_float[offset];
            *dest = *src1;
            break;
    }
}
#endif

/* Function 3: AVX-512 16-element shuffle (requires 16 mask operands) */
#ifdef __AVX512F__
__attribute__((noinline))
void shuffle_avx512(int offset, volatile int mask_seed) {
    v16si* src1 = (v16si*)&global_int_array[offset];
    v16si* src2 = (v16si*)&global_int_array[offset + 16];
    
    volatile int mask_val = mask_seed;
    v16si mask;
    for (int i = 0; i < 16; i++) {
        mask[i] = (mask_val + i * 3) % 32;
    }
    
    /* Loop with conditional shuffle execution */
    for (int iter = 0; iter < 2; iter++) {
        if ((mask_val + iter) & 1) {
            /* 11 operand case: 2 vectors + 9 mask values (partial) */
            v16si partial_shuffle = __builtin_shuffle(*src1, *src2,
                mask[0], mask[1], mask[2], mask[3], mask[4],
                mask[5], mask[6], mask[7], mask[8]);
            
            /* Complete with another shuffle */
            v16si result = __builtin_shuffle(partial_shuffle, *src2,
                mask[9], mask[10], mask[11], mask[12],
                mask[13], mask[14], mask[15], 0, 1, 2, 3, 4, 5, 6, 7);
            
            volatile v16si* dest = (volatile v16si*)&accumulator_int[offset];
            *dest = result;
        }
    }
}
#endif

/* Function 4: Mixed SIMD patterns with narrowing/expanding */
#ifdef __SSE2__
__attribute__((noinline))
void shuffle_mixed_sse2(int offset, volatile int mask_seed) {
    v4si* src_int = (v4si*)&global_int_array[offset];
    v4sf* src_float = (v4sf*)&global_float_array[offset];
    
    volatile int mask_val = mask_seed;
    v4si mask_int;
    for (int i = 0; i < 4; i++) {
        mask_int[i] = (mask_val + i * 5) % 8;
    }
    
    /* Mixed type operations */
    v4si int_vec = *src_int;
    v4sf float_vec = *src_float;
    
    /* Shuffle with conversion pattern */
    v4si shuffled_int = __builtin_shuffle(int_vec, int_vec,
        mask_int[0], mask_int[1], mask_int[2], mask_int[3]);
    
    /* Convert and mix */
    v4sf converted = __builtin_convertvector(shuffled_int, v4sf);
    v4sf result = converted + float_vec;
    
    volatile v4sf* dest = (volatile v4sf*)&accumulator_float[offset];
    *dest = result;
}
#endif

/* Function 5: Double precision shuffle with many operands */
#ifdef __AVX2__
__attribute__((noinline))
void shuffle_double_avx2(int offset, volatile int mask_seed) {
    v4df* src1 = (v4df*)&global_float_array[offset];  /* Using float array as double */
    v4df* src2 = (v4df*)&global_float_array[offset + 4];
    
    volatile int mask_val = mask_seed;
    v4df mask_double_idx;
    for (int i = 0; i < 4; i++) {
        mask_double_idx[i] = (double)((mask_val + i * 7) % 8);
    }
    
    /* Complex if-else chain */
    if (mask_val > 1000) {
        /* Unlikely path */
        volatile v4df* dest = (volatile v4df*)&accumulator_float[offset];
        *dest = *src1;
    } else if (mask_val > 500) {
        /* 10 operand shuffle pattern */
        v4df result = __builtin_shuffle(*src1, *src2,
            (int)mask_double_idx[0], (int)mask_double_idx[1],
            (int)mask_double_idx[2], (int)mask_double_idx[3],
            4, 5, 6, 7);  /* Additional constant indices */
        
        volatile v4df* dest = (volatile v4df*)&accumulator_float[offset];
        *dest = result;
    } else {
        /* Default shuffle */
        v4df result = __builtin_shuffle(*src1, *src2, 3, 2, 1, 0);
        volatile v4df* dest = (volatile v4df*)&accumulator_float[offset];
        *dest = result;
    }
}
#endif

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    volatile int runtime_mask = seed;
    
    /* Main processing loop */
    for (int iteration = 0; iteration < 10; iteration++) {
        int offset = (iteration * 17) % (ARRAY_SIZE - 64);
        
        /* Call different shuffle functions based on iteration */
        switch (iteration % 5) {
            case 0:
#ifdef __SSE2__
                shuffle_mixed_sse2(offset, runtime_mask + iteration);
#endif
                break;
                
            case 1:
#ifdef __AVX__
                shuffle_float_avx(offset, runtime_mask + iteration * 3);
#endif
                break;
                
            case 2:
#ifdef __AVX2__
                shuffle_int_avx2(offset, runtime_mask + iteration * 5);
#endif
                break;
                
            case 3:
#ifdef __AVX2__
                shuffle_double_avx2(offset, runtime_mask + iteration * 7);
#endif
                break;
                
            case 4:
#ifdef __AVX512F__
                shuffle_avx512(offset, runtime_mask + iteration * 11);
#endif
                break;
        }
        
        /* Modify runtime mask */
        runtime_mask = (runtime_mask * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
    }
    
    printf("Checksum - Integer: %lld, Float: %f\n", 
           (long long)int_sum, float_sum);
    
    return 0;
}
