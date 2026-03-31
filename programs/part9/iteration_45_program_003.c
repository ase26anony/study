#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int global_seed;
static int data_int[512];
static float data_float[512];
static int64_t accumulator[512] = {0};

/* Vector types using GCC extensions */
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
        data_int[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        data_float[i] = (float)((i * 1103515245 + 12345) & 0xFFFF) / 65536.0f;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
__attribute__((noinline))
v16si shuffle_int_16way(v16si a, v16si b, v16si mask) {
    volatile int idx = global_seed & 15;
    
    /* This should generate 10+ operands: a, b, mask, and 16 indices */
    v16si result = __builtin_shuffle(a, b, mask);
    
    /* Additional shuffle with runtime-dependent pattern */
    if (idx > 8) {
        /* Another complex shuffle - potentially 11 operands */
        v16si mask2 = {idx, idx+1, idx+2, idx+3, idx+4, idx+5, idx+6, idx+7,
                       idx+8, idx+9, idx+10, idx+11, idx+12, idx+13, idx+14, idx+15};
        result = __builtin_shuffle(result, mask2);
    }
    
    return result;
}

/* Function 2: Mixed float/double shuffles with control flow */
__attribute__((noinline))
v8df shuffle_mixed_types(v8df a, v8df b, v8sf float_mask, int pattern) {
    volatile int mode = pattern % 4;
    v8df result;
    
    switch (mode) {
        case 0: {
            /* Complex shufflevector with many operands */
            v8df temp = __builtin_shufflevector(a, b, 0, 2, 4, 6, 8, 10, 12, 14);
            result = __builtin_shufflevector(temp, a, 7, 6, 5, 4, 3, 2, 1, 0);
            break;
        }
        case 1: {
            /* Another pattern with potential 10+ operands */
            v8df temp = __builtin_shufflevector(a, b, 1, 3, 5, 7, 9, 11, 13, 15);
            result = __builtin_shufflevector(temp, b, 0, 2, 4, 6, 1, 3, 5, 7);
            break;
        }
        case 2: {
            /* Nested shuffles to increase operand count */
            v8df t1 = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
            v8df t2 = __builtin_shufflevector(a, b, 4, 12, 5, 13, 6, 14, 7, 15);
            result = __builtin_shufflevector(t1, t2, 0, 2, 4, 6, 1, 3, 5, 7);
            break;
        }
        default: {
            /* Most complex case - multiple shuffles in sequence */
            v8df t1 = __builtin_shufflevector(a, b, 0, 1, 8, 9, 2, 3, 10, 11);
            v8df t2 = __builtin_shufflevector(a, b, 4, 5, 12, 13, 6, 7, 14, 15);
            v8df t3 = __builtin_shufflevector(t1, t2, 0, 4, 1, 5, 2, 6, 3, 7);
            result = __builtin_shufflevector(t3, a, 7, 6, 5, 4, 3, 2, 1, 0);
            break;
        }
    }
    
    return result;
}

/* Function 3: Narrowing and widening with shuffles */
__attribute__((noinline))
v8si narrow_and_expand(v16si wide, int shift) {
    volatile int s = shift & 7;
    
    /* First narrow 512-bit to 256-bit */
    v8si narrow = __builtin_shufflevector(
        wide, wide, 
        0 + s, 2 + s, 4 + s, 6 + s, 8 + s, 10 + s, 12 + s, 14 + s
    );
    
    /* Then expand back with pattern */
    v8si expanded = __builtin_shufflevector(
        narrow, narrow,
        7, 6, 5, 4, 3, 2, 1, 0
    );
    
    /* Conditional additional shuffle */
    if (s > 4) {
        expanded = __builtin_shufflevector(
            expanded, expanded,
            0, 2, 4, 6, 1, 3, 5, 7
        );
    }
    
    return expanded;
}

/* Function 4: Architecture-specific variants */
#ifdef __AVX512F__
__attribute__((noinline))
v16sf avx512_shuffle_complex(v16sf a, v16sf b, v16si mask) {
    volatile int selector = global_seed & 3;
    v16sf result;
    
    if (selector == 0) {
        /* Pattern requiring many operands */
        result = __builtin_shuffle(a, b, mask);
    } else if (selector == 1) {
        /* Alternative pattern */
        v16sf t1 = __builtin_shufflevector(a, b, 
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        result = __builtin_shuffle(t1, mask);
    } else {
        /* Complex chain of operations */
        v16sf t1 = __builtin_shuffle(a, mask);
        v16sf t2 = __builtin_shuffle(b, mask);
        result = __builtin_shufflevector(t1, t2,
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
    }
    
    return result;
}
#endif

#ifdef __AVX2__
__attribute__((noinline))
v8df avx2_double_shuffle(v8df a, v8df b, int pattern) {
    volatile int p = pattern;
    v8df result;
    
    /* Switch with multiple shuffle patterns */
    switch (p & 7) {
        case 0:
            result = __builtin_shufflevector(a, b, 0, 8, 2, 10, 4, 12, 6, 14);
            break;
        case 1:
            result = __builtin_shufflevector(a, b, 1, 9, 3, 11, 5, 13, 7, 15);
            break;
        case 2:
            result = __builtin_shufflevector(a, b, 0, 1, 8, 9, 2, 3, 10, 11);
            break;
        case 3:
            result = __builtin_shufflevector(a, b, 4, 5, 12, 13, 6, 7, 14, 15);
            break;
        case 4: {
            /* Nested shuffles for more operands */
            v8df t1 = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
            v8df t2 = __builtin_shufflevector(a, b, 4, 12, 5, 13, 6, 14, 7, 15);
            result = __builtin_shufflevector(t1, t2, 0, 2, 4, 6, 1, 3, 5, 7);
            break;
        }
        default: {
            /* Complex case with multiple operations */
            v8df t1 = __builtin_shufflevector(a, b, 0, 2, 4, 6, 8, 10, 12, 14);
            v8df t2 = __builtin_shufflevector(a, b, 1, 3, 5, 7, 9, 11, 13, 15);
            result = __builtin_shufflevector(t1, t2, 0, 4, 1, 5, 2, 6, 3, 7);
            break;
        }
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
    
    init_data(seed);
    
    /* Process data in chunks */
    for (int iter = 0; iter < 10; iter++) {
        volatile int offset = iter * 16;
        
        /* Load integer vectors */
        v16si int_vec1 = *(v16si*)(&data_int[offset]);
        v16si int_vec2 = *(v16si*)(&data_int[offset + 16]);
        v16si mask_int = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15};
        
        /* Call shuffle functions */
        v16si shuffled_int = shuffle_int_16way(int_vec1, int_vec2, mask_int);
        
        /* Store intermediate result */
        *(v16si*)(&accumulator[offset]) += (v16si)shuffled_int;
        
        /* Load float/double vectors */
        v8df double_vec1 = *(v8df*)(&data_float[offset]);
        v8df double_vec2 = *(v8df*)(&data_float[offset + 8]);
        v8sf float_mask = {0, 2, 4, 6, 1, 3, 5, 7};
        
        /* Mixed type shuffles */
        v8df shuffled_double = shuffle_mixed_types(double_vec1, double_vec2, float_mask, iter);
        
        /* Convert and accumulate */
        for (int i = 0; i < 8; i++) {
            accumulator[offset + i] += (int64_t)shuffled_double[i];
        }
        
        /* Narrowing/expanding operations */
        v8si narrowed = narrow_and_expand(int_vec1, iter);
        for (int i = 0; i < 8; i++) {
            accumulator[offset + i + 8] += narrowed[i];
        }
        
        /* Architecture-specific paths */
#ifdef __AVX512F__
        v16sf float_vec1 = *(v16sf*)(&data_float[offset]);
        v16sf float_vec2 = *(v16sf*)(&data_float[offset + 16]);
        v16sf shuffled_float = avx512_shuffle_complex(float_vec1, float_vec2, mask_int);
        
        for (int i = 0; i < 16; i++) {
            accumulator[offset + i] += (int64_t)shuffled_float[i];
        }
#endif
        
#ifdef __AVX2__
        v8df avx2_result = avx2_double_shuffle(double_vec1, double_vec2, iter);
        for (int i = 0; i < 8; i++) {
            accumulator[offset + i + 16] += (int64_t)avx2_result[i];
        }
#endif
    }
    
    /* Compute final checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    return 0;
}
