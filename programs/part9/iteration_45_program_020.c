#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays initialized with deterministic pseudo-random data */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static double global_floats[ARRAY_SIZE];
static int32_t accumulator_ints[ARRAY_SIZE];
static double accumulator_floats[ARRAY_SIZE];

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Simple deterministic pseudo-random generator */
static uint32_t prng_state = 0;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize global arrays with deterministic data */
static void init_arrays(int seed) {
    prng_state = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_ints[i] = (int32_t)prng_next();
        global_floats[i] = (double)prng_next() / 4294967296.0;
        accumulator_ints[i] = 0;
        accumulator_floats[i] = 0.0;
    }
}

/* Function 1: Large integer shuffle with 10+ operands */
#ifdef __AVX2__
static void shuffle_large_int_vector(int offset, volatile int mask_idx) {
    /* Load 512-bit vector (16 ints) */
    v16si vec1 = *(v16si*)(&global_ints[offset]);
    v16si vec2 = *(v16si*)(&global_ints[offset + 16]);
    
    /* Use volatile to prevent constant folding of mask */
    volatile int idx = mask_idx;
    
    /* Create control mask with runtime-dependent indices */
    int32_t mask[16];
    for (int i = 0; i < 16; i++) {
        mask[i] = (idx + i * 3) % 32;  /* Mix indices from both vectors */
    }
    
    /* Force complex control flow */
    if (idx & 1) {
        /* Path 1: __builtin_shuffle with 10+ operands */
        v16si control = *(v16si*)mask;
        
        /* This should require 10 operands during expansion:
           - 2 input vectors (vec1, vec2)
           - 16 mask elements (control vector) = 18 total elements
           The expander will pack them into ops array */
        v16si result = __builtin_shuffle(vec1, vec2, control);
        
        /* Perform arithmetic to prevent elimination */
        result = result + vec1;
        
        /* Store to accumulator */
        v16si* acc = (v16si*)(&accumulator_ints[offset]);
        *acc = *acc + result;
    } else {
        /* Path 2: Alternative shuffle pattern */
        int32_t mask2[16];
        for (int i = 0; i < 16; i++) {
            mask2[i] = (idx + i * 5) % 32;
        }
        
        v16si control2 = *(v16si*)mask2;
        v16si result = __builtin_shuffle(vec1, vec2, control2);
        result = result * 2 - vec2;
        
        v16si* acc = (v16si*)(&accumulator_ints[offset]);
        *acc = *acc + result;
    }
}
#endif

/* Function 2: Double-precision shuffle with mixed operations */
#ifdef __AVX512F__
static void shuffle_large_double_vector(int offset, volatile int pattern) {
    /* Load 512-bit vectors (8 doubles each) */
    v8df vec1 = *(v8df*)(&global_floats[offset]);
    v8df vec2 = *(v8df*)(&global_floats[offset + 8]);
    
    volatile int pat = pattern;
    double control_mask[8];
    
    /* Switch statement to create different control flow paths */
    switch (pat % 4) {
        case 0:
            for (int i = 0; i < 8; i++) {
                control_mask[i] = (double)((pat + i * 2) % 16);
            }
            break;
        case 1:
            for (int i = 0; i < 8; i++) {
                control_mask[i] = (double)((pat + i * 7) % 16);
            }
            break;
        case 2:
            for (int i = 0; i < 8; i++) {
                control_mask[i] = (double)((pat + i * 11) % 16);
            }
            break;
        default:
            for (int i = 0; i < 8; i++) {
                control_mask[i] = (double)((pat + i * 13) % 16);
            }
    }
    
    /* Use __builtin_shufflevector which can require many operands */
    v8df control = *(v8df*)control_mask;
    
    /* This shuffle operation with large vectors may trigger the 11-operand case */
    v8df result = __builtin_shuffle(vec1, vec2, control);
    
    /* Additional arithmetic to create more complex RTL */
    if (pat & 8) {
        result = result * 1.5;
    } else {
        result = result / 1.5;
    }
    
    /* Volatile store to prevent optimization */
    volatile v8df temp = result;
    
    /* Accumulate result */
    v8df* acc = (v8df*)(&accumulator_floats[offset]);
    *acc = *acc + temp;
}
#endif

/* Function 3: Mixed integer/float operations with narrowing/expanding */
#ifdef __AVX2__
static void mixed_vector_operations(int offset, volatile int mode) {
    /* Work with 256-bit vectors */
    v8si int_vec = *(v8si*)(&global_ints[offset]);
    v4df float_vec = *(v4df*)(&global_floats[offset]);
    
    volatile int m = mode;
    
    /* Complex control flow with nested conditionals */
    if (m > 100) {
        /* Path with shuffle between different vector types */
        int32_t shuffle_mask[8];
        for (int i = 0; i < 8; i++) {
            shuffle_mask[i] = (m + i * 17) % 16;
        }
        
        v8si mask = *(v8si*)shuffle_mask;
        v8si shuffled = __builtin_shuffle(int_vec, int_vec, mask);
        
        /* Convert and mix with floats */
        v4df converted = (v4df)shuffled;
        v4df mixed = converted * float_vec;
        
        /* Store back */
        v4df* acc = (v4df*)(&accumulator_floats[offset]);
        *acc = *acc + mixed;
    } else {
        /* Alternative path with different shuffle pattern */
        int32_t alt_mask[8];
        for (int i = 0; i < 8; i++) {
            alt_mask[i] = (m + i * 23) % 8;
        }
        
        v8si alt_mask_vec = *(v8si*)alt_mask;
        v8si result = __builtin_shuffle(int_vec, int_vec, alt_mask_vec);
        
        /* Chain multiple operations */
        result = result + (int_vec >> 2);
        result = result * 3;
        
        v8si* acc = (v8si*)(&accumulator_ints[offset]);
        *acc = *acc + result;
    }
}
#endif

/* Function 4: 128-bit operations for baseline testing */
#ifdef __SSE2__
static void sse2_vector_operations(int offset, volatile int idx) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    v4si int_vec = *(v4si*)(&global_ints[offset]);
    v2df float_vec = *(v2df*)(&global_floats[offset]);
    
    /* Create runtime-dependent shuffle */
    int32_t mask[4];
    for (int i = 0; i < 4; i++) {
        mask[i] = (idx + i * 3) % 8;
    }
    
    v4si mask_vec = *(v4si*)mask;
    v4si shuffled = __builtin_shuffle(int_vec, int_vec, mask_vec);
    
    /* Mix with accumulator */
    v4si* acc = (v4si*)(&accumulator_ints[offset]);
    *acc = *acc + shuffled;
}
#endif

/* Main function with complex control flow */
int main(int argc, char* argv[]) {
    /* Use command-line seed or default */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Volatile variable to prevent compile-time optimization */
    volatile int runtime_value = seed;
    
    /* Main loop with complex control flow */
    for (int iter = 0; iter < 10; iter++) {
        for (int offset = 0; offset < ARRAY_SIZE - 64; offset += 64) {
            /* Mix different shuffle functions based on iteration */
            if (iter % 3 == 0) {
#ifdef __AVX2__
                shuffle_large_int_vector(offset, runtime_value + iter);
#endif
            } else if (iter % 3 == 1) {
#ifdef __AVX512F__
                shuffle_large_double_vector(offset, runtime_value + iter * 7);
#endif
            } else {
#ifdef __AVX2__
                mixed_vector_operations(offset, runtime_value + iter * 13);
#endif
            }
            
            /* Always include some SSE2 operations */
#ifdef __SSE2__
            sse2_vector_operations(offset, runtime_value + iter * 5);
#endif
            
            /* Modify runtime value to change control flow */
            runtime_value = (runtime_value * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += accumulator_ints[i];
        float_sum += accumulator_floats[i];
    }
    
    /* Print results (prevents elimination of entire computation) */
    printf("Checksum - Integer: %lld, Float: %f\n", 
           (long long)int_sum, float_sum);
    
    return 0;
}
