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
typedef int v8si __attribute__((vector_size(32)));      /* 8 ints - 256-bit */
typedef int v16si __attribute__((vector_size(64)));     /* 16 ints - 512-bit */
typedef float v8sf __attribute__((vector_size(32)));    /* 8 floats - 256-bit */
typedef float v16sf __attribute__((vector_size(64)));   /* 16 floats - 512-bit */
typedef double v4df __attribute__((vector_size(32)));   /* 4 doubles - 256-bit */
typedef double v8df __attribute__((vector_size(64)));   /* 8 doubles - 512-bit */

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        data_int[i] = (i * 1103515245 + 12345) & 0x7FFF;
        data_float[i] = (float)((i * 1103515245 + 12345) & 0x7FFF) * 0.001f;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
static v16si shuffle_large_int_vector(v16si a, v16si b, volatile int mask_idx) {
    /* Complex control flow to prevent optimization */
    v16si result;
    
    if (mask_idx & 1) {
        /* Create a shuffle mask with 16 indices - will use many operands */
        int idx0 = (mask_idx + 0) & 15;
        int idx1 = (mask_idx + 1) & 15;
        int idx2 = (mask_idx + 2) & 15;
        int idx3 = (mask_idx + 3) & 15;
        int idx4 = (mask_idx + 4) & 15;
        int idx5 = (mask_idx + 5) & 15;
        int idx6 = (mask_idx + 6) & 15;
        int idx7 = (mask_idx + 7) & 15;
        int idx8 = (mask_idx + 8) & 15;
        int idx9 = (mask_idx + 9) & 15;
        int idx10 = (mask_idx + 10) & 15;
        int idx11 = (mask_idx + 11) & 15;
        int idx12 = (mask_idx + 12) & 15;
        int idx13 = (mask_idx + 13) & 15;
        int idx14 = (mask_idx + 14) & 15;
        int idx15 = (mask_idx + 15) & 15;
        
        /* This shuffle uses 18 operands total (2 vectors + 16 indices) */
        result = __builtin_shuffle(a, b, 
            idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
            idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    } else {
        /* Alternative shuffle pattern */
        result = __builtin_shuffle(a, b, 
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0);
    }
    
    /* Additional arithmetic to use the result */
    return result + (v16si){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
}
#endif

#ifdef __AVX512F__
static v16sf shuffle_large_float_vector(v16sf a, v16sf b, volatile int pattern) {
    v16sf result;
    
    switch (pattern & 3) {
        case 0: {
            /* Pattern 0: Interleave elements */
            result = __builtin_shuffle(a, b,
                0, 16, 1, 17, 2, 18, 3, 19,
                4, 20, 5, 21, 6, 22, 7, 23);
            break;
        }
        case 1: {
            /* Pattern 1: Reverse and blend */
            result = __builtin_shuffle(a, b,
                15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0);
            break;
        }
        case 2: {
            /* Pattern 2: Complex pattern with runtime indices */
            int base = pattern;
            result = __builtin_shuffle(a, b,
                (base + 0) & 31, (base + 1) & 31, (base + 2) & 31, (base + 3) & 31,
                (base + 4) & 31, (base + 5) & 31, (base + 6) & 31, (base + 7) & 31,
                (base + 8) & 31, (base + 9) & 31, (base + 10) & 31, (base + 11) & 31,
                (base + 12) & 31, (base + 13) & 31, (base + 14) & 31, (base + 15) & 31);
            break;
        }
        default: {
            /* Default: Identity shuffle */
            result = __builtin_shuffle(a, a,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
            break;
        }
    }
    
    /* Scale the result */
    return result * (v16sf){2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f,
                            2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
}
#endif

/* Mixed SIMD patterns with narrowing/widening */
#ifdef __AVX2__
static v8si narrow_and_expand(v16si large_vec, volatile int shift) {
    /* First extract lower 128-bit (narrowing) */
    v8si lower = __builtin_shuffle(large_vec, large_vec,
        0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Then expand back with pattern */
    v8si expanded = __builtin_shuffle(lower, lower,
        (shift + 0) & 7, (shift + 1) & 7, (shift + 2) & 7, (shift + 3) & 7,
        (shift + 4) & 7, (shift + 5) & 7, (shift + 6) & 7, (shift + 7) & 7);
    
    return expanded;
}
#endif

/* Function using __builtin_shufflevector for even more operands */
#ifdef __AVX512F__
static v8df shufflevector_double_large(v8df a, v8df b, v8df c, volatile int mode) {
    v8df result;
    
    if (mode == 0) {
        /* shufflevector can take many vector arguments */
        result = __builtin_shufflevector(a, b, c,
            0, 8, 16, 1, 9, 17, 2, 10,  /* 11 operands total */
            18, 3, 11, 19, 4, 12, 20, 5,
            13, 21, 6, 14, 22, 7, 15, 23);
    } else {
        /* Alternative pattern */
        result = __builtin_shufflevector(a, b,
            7, 6, 5, 4, 3, 2, 1, 0,
            15, 14, 13, 12, 11, 10, 9, 8);
    }
    
    return result;
}
#endif

/* Main processing function with complex control flow */
void process_vectors(int iterations, volatile int pattern_seed) {
    for (int i = 0; i < iterations; i++) {
        int base_idx = (i * 16) % 480;
        
        #ifdef __AVX512F__
        {
            /* Load 512-bit vectors */
            v16si int_vec1 = *(v16si*)(&data_int[base_idx]);
            v16si int_vec2 = *(v16si*)(&data_int[base_idx + 16]);
            v16sf float_vec1 = *(v16sf*)(&data_float[base_idx]);
            v16sf float_vec2 = *(v16sf*)(&data_float[base_idx + 16]);
            
            /* Perform large shuffles */
            v16si shuffled_int = shuffle_large_int_vector(int_vec1, int_vec2, pattern_seed + i);
            v16sf shuffled_float = shuffle_large_float_vector(float_vec1, float_vec2, pattern_seed + i);
            
            /* Store results to accumulator */
            for (int j = 0; j < 16; j++) {
                accumulator[base_idx + j] += shuffled_int[j];
                accumulator[base_idx + j] += (int64_t)shuffled_float[j];
            }
            
            /* Volatile store to prevent optimization */
            volatile v16si temp = shuffled_int;
            (void)temp;
        }
        #endif
        
        #ifdef __AVX2__
        {
            /* Process 256-bit vectors */
            v8si med_int_vec = *(v8si*)(&data_int[base_idx]);
            v8sf med_float_vec = *(v8sf*)(&data_float[base_idx]);
            
            /* Nested control flow */
            if ((pattern_seed + i) & 1) {
                /* Complex shuffle with many indices */
                v8si shuffled = __builtin_shuffle(med_int_vec, med_int_vec,
                    (i + 0) & 7, (i + 1) & 7, (i + 2) & 7, (i + 3) & 7,
                    (i + 4) & 7, (i + 5) & 7, (i + 6) & 7, (i + 7) & 7);
                
                for (int j = 0; j < 8; j++) {
                    accumulator[base_idx + j] += shuffled[j];
                }
            }
            
            /* Another shuffle in loop */
            for (int k = 0; k < 2; k++) {
                v8sf shuffled_float = __builtin_shuffle(med_float_vec, med_float_vec,
                    7 - k, 6 - k, 5 - k, 4 - k, 3 - k, 2 - k, 1 - k, 0 - k);
                
                volatile v8sf temp = shuffled_float;
                (void)temp;
            }
        }
        #endif
        
        #ifdef __SSE2__
        {
            /* 128-bit vector operations */
            typedef int v4si __attribute__((vector_size(16)));
            v4si small_vec = *(v4si*)(&data_int[base_idx]);
            
            /* Multiple shuffle patterns in switch */
            switch ((pattern_seed + i) & 7) {
                case 0:
                case 1:
                case 2:
                    small_vec = __builtin_shuffle(small_vec, small_vec, 3, 2, 1, 0);
                    break;
                case 3:
                case 4:
                    small_vec = __builtin_shuffle(small_vec, small_vec, 
                        (i + 0) & 3, (i + 1) & 3, (i + 2) & 3, (i + 3) & 3);
                    break;
                default:
                    small_vec = __builtin_shuffle(small_vec, small_vec, 0, 0, 0, 0);
                    break;
            }
            
            for (int j = 0; j < 4; j++) {
                accumulator[base_idx + j] += small_vec[j];
            }
        }
        #endif
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    volatile int pattern_seed = seed;
    
    /* Process vectors with different patterns */
    process_vectors(10, pattern_seed);
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
        checksum ^= (accumulator[i] << (i & 15));
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return 0;
}
