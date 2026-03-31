#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int32_t global_int_data[512];
volatile double global_float_data[512];
int32_t accumulator_int[512];
double accumulator_float[512];

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float */

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
        global_float_data[i] = (double)((i * 1103515245 + seed) & 0xFFF) / 4096.0;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operand case */
#ifdef __AVX2__
static v16si shuffle_large_int_vector(v16si a, v16si b, volatile int* mask_indices) {
    /* Create control mask from volatile indices - prevents constant folding */
    int32_t mask_data[16];
    for (int i = 0; i < 16; i++) {
        mask_data[i] = mask_indices[i % 8] & 0x1F;  /* Mask to 0-31 range */
    }
    
    /* Use volatile to force runtime evaluation */
    volatile int32_t* volatile_mask = mask_data;
    
    /* Load mask into vector - this adds more operands to the expression */
    v16si mask = *(v16si*)volatile_mask;
    
    /* Complex shuffle with many implicit operands */
    v16si result;
    
    /* Nested control flow to complicate the CFG */
    if (mask_indices[0] & 1) {
        /* This shuffle should require many operand slots during expansion */
        result = __builtin_shuffle(a, b, mask);
        
        /* Additional arithmetic to use the result */
        result = result + (a >> 2);
        result = result * (v16si){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    } else {
        /* Alternative shuffle pattern */
        v16si mask2 = mask + (v16si){16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        result = __builtin_shuffle(a, b, mask2);
        result = result - (b << 1);
    }
    
    return result;
}
#endif

#ifdef __AVX512F__
static v8df shuffle_large_double_vector(v8df a, v8df b, volatile int* mask_indices) {
    /* Create two different masks from volatile indices */
    int64_t mask1_data[8];
    int64_t mask2_data[8];
    
    for (int i = 0; i < 8; i++) {
        mask1_data[i] = (mask_indices[i] & 0xF) * 2;
        mask2_data[i] = (mask_indices[i] & 0xF) * 2 + 1;
    }
    
    volatile int64_t* volatile_mask1 = mask1_data;
    volatile int64_t* volatile_mask2 = mask2_data;
    
    v8df mask_vec1 = *(v8df*)volatile_mask1;
    v8df mask_vec2 = *(v8df*)volatile_mask2;
    
    v8df result;
    
    /* Switch statement with volatile condition */
    switch (mask_indices[0] & 3) {
        case 0: {
            /* Complex expression that may require many operands */
            v8df temp1 = __builtin_shuffle(a, b, mask_vec1);
            v8df temp2 = __builtin_shuffle(b, a, mask_vec2);
            result = temp1 * temp2 + a;
            break;
        }
        case 1: {
            /* Different shuffle pattern */
            v8df combined_mask = mask_vec1 + mask_vec2;
            result = __builtin_shuffle(a, b, combined_mask);
            result = result / (v8df){1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
            break;
        }
        default: {
            /* Mix of shuffles and arithmetic */
            v8df temp = __builtin_shuffle(a, a, mask_vec1);
            result = __builtin_shuffle(temp, b, mask_vec2);
            result = result * 2.5;
        }
    }
    
    return result;
}
#endif

/* Mixed SIMD patterns with narrowing/expanding */
#ifdef __SSE2__
static v8si narrow_and_expand(v8si a, v8si b, volatile int* indices) {
    /* First narrow 256-bit to 128-bit conceptually */
    int32_t narrow_data[4];
    for (int i = 0; i < 4; i++) {
        narrow_data[i] = ((int32_t*)&a)[i * 2] + ((int32_t*)&b)[i * 2];
    }
    
    /* Then expand back with shuffle */
    v8si narrow_vec = *(v8si*)narrow_data;
    
    /* Create complex mask with many elements */
    int32_t expand_mask[8];
    for (int i = 0; i < 8; i++) {
        expand_mask[i] = (indices[i % 4] + i) & 0x7;
    }
    
    volatile int32_t* volatile_mask = expand_mask;
    v8si mask = *(v8si*)volatile_mask;
    
    /* Shuffle that may require many operand slots */
    v8si result = __builtin_shuffle(narrow_vec, mask);
    
    /* Additional operations to prevent elimination */
    if (indices[0] & 1) {
        result = result + a;
    } else {
        result = result - b;
    }
    
    return result;
}
#endif

/* Function using __builtin_shufflevector with explicit many arguments */
static v16sf shufflevector_complex(v16sf a, v16sf b, volatile int* indices) {
    /* __builtin_shufflevector with many explicit indices - may trigger 10+ operand case */
    v16sf result;
    
    /* Loop to create dynamic control flow */
    for (int attempt = 0; attempt < 3; attempt++) {
        if (indices[attempt] & 1) {
            /* Shufflevector with many explicit indices - potentially 10+ operands total */
            result = __builtin_shufflevector(a, b, 
                0, 2, 4, 6, 8, 10, 12, 14,  /* 8 indices from first arg */
                16, 18, 20, 22, 24, 26, 28, 30  /* 8 more indices - total 16 */
            );
        } else {
            /* Alternative pattern */
            result = __builtin_shufflevector(b, a,
                1, 3, 5, 7, 9, 11, 13, 15,
                17, 19, 21, 23, 25, 27, 29, 31
            );
        }
        
        /* Break loop based on volatile condition */
        if (indices[attempt] > 100) break;
    }
    
    return result;
}

/* Main test function */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    volatile int mask_indices[8];
    for (int i = 0; i < 8; i++) {
        mask_indices[i] = (i * seed + 12345) & 0xFF;
    }
    
    int iterations = 10;
    for (int iter = 0; iter < iterations; iter++) {
        /* Update mask indices each iteration */
        for (int i = 0; i < 8; i++) {
            mask_indices[i] = (mask_indices[i] * 1664525 + 1013904223) & 0xFF;
        }
        
#ifdef __AVX2__
        /* Test with large integer vectors */
        v16si int_vec1, int_vec2;
        memcpy(&int_vec1, (void*)&global_int_data[iter * 16], sizeof(v16si));
        memcpy(&int_vec2, (void*)&global_int_data[iter * 16 + 16], sizeof(v16si));
        
        v16si int_result = shuffle_large_int_vector(int_vec1, int_vec2, mask_indices);
        
        /* Accumulate result */
        for (int i = 0; i < 16; i++) {
            accumulator_int[iter * 16 + i] += ((int32_t*)&int_result)[i];
        }
#endif
        
#ifdef __AVX512F__
        /* Test with large double vectors */
        v8df double_vec1, double_vec2;
        memcpy(&double_vec1, (void*)&global_float_data[iter * 8], sizeof(v8df));
        memcpy(&double_vec2, (void*)&global_float_data[iter * 8 + 8], sizeof(v8df));
        
        v8df double_result = shuffle_large_double_vector(double_vec1, double_vec2, mask_indices);
        
        /* Accumulate result */
        for (int i = 0; i < 8; i++) {
            accumulator_float[iter * 8 + i] += ((double*)&double_result)[i];
        }
#endif
        
#ifdef __SSE2__
        /* Test mixed SIMD patterns */
        v8si mixed_vec1, mixed_vec2;
        memcpy(&mixed_vec1, (void*)&global_int_data[iter * 8 + 256], sizeof(v8si));
        memcpy(&mixed_vec2, (void*)&global_int_data[iter * 8 + 264], sizeof(v8si));
        
        v8si mixed_result = narrow_and_expand(mixed_vec1, mixed_vec2, mask_indices);
        
        for (int i = 0; i < 8; i++) {
            accumulator_int[256 + iter * 8 + i] += ((int32_t*)&mixed_result)[i];
        }
#endif
        
        /* Test shufflevector with float vectors */
        v16sf float_vec1, float_vec2;
        memcpy(&float_vec1, (void*)&global_float_data[256], sizeof(v16sf));
        memcpy(&float_vec2, (void*)&global_float_data[272], sizeof(v16sf));
        
        v16sf float_result = shufflevector_complex(float_vec1, float_vec2, mask_indices);
        
        for (int i = 0; i < 16; i++) {
            accumulator_float[256 + i] += ((float*)&float_result)[i];
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_checksum = 0;
    double float_checksum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
    }
    
    printf("Checksums: int = %lld, float = %f\n", 
           (long long)int_checksum, float_checksum);
    
    return 0;
}
