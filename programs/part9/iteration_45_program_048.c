#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int64_t global_accumulator[512] = {0};

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < 512; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int)(r >> 16) & 0x7FFF;
        global_float_array[i] = (float)((r >> 8) & 0xFF) * 0.1f;
    }
}

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Function that uses shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
void shuffle_large_int_vector(int* src, int* dst, volatile int mask_val) {
    v8si a = *(v8si*)&src[0];
    v8si b = *(v8si*)&src[8];
    
    /* Create a control mask with runtime-dependent indices */
    volatile int idx0 = mask_val % 8;
    volatile int idx1 = (mask_val + 1) % 8;
    volatile int idx2 = (mask_val + 2) % 8;
    volatile int idx3 = (mask_val + 3) % 8;
    volatile int idx4 = (mask_val + 4) % 8;
    volatile int idx5 = (mask_val + 5) % 8;
    volatile int idx6 = (mask_val + 6) % 8;
    volatile int idx7 = (mask_val + 7) % 8;
    
    /* This shuffle uses 10 operands: 2 input vectors + 8 indices */
    v8si result = __builtin_shuffle(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Additional operation to prevent dead code elimination */
    result = result + a;
    
    *(v8si*)&dst[0] = result;
}
#endif

#ifdef __AVX512F__
void shuffle_very_large_vector(int* src, int* dst, volatile int pattern) {
    v16si a = *(v16si*)&src[0];
    v16si b = *(v16si*)&src[16];
    
    /* Complex control flow to keep the shuffle dynamic */
    if (pattern & 1) {
        /* This shufflevector uses 11 operands: 2 input vectors + 9 indices */
        v16si result = __builtin_shufflevector(a, b,
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        
        /* Mix with arithmetic */
        result = result * (v16si){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
        *(v16si*)&dst[0] = result;
    } else {
        /* Alternative shuffle pattern */
        v16si result = __builtin_shufflevector(a, b,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
        *(v16si*)&dst[0] = result;
    }
}
#endif

/* Mixed floating-point shuffle with control flow */
#ifdef __AVX2__
void shuffle_float_double_mixed(float* fsrc, double* dsrc, 
                                float* fdst, double* ddst, 
                                volatile int mode) {
    v8sf fa = *(v8sf*)&fsrc[0];
    v8sf fb = *(v8sf*)&fsrc[8];
    v4df da = *(v4df*)&dsrc[0];
    v4df db = *(v4df*)&dsrc[4];
    
    switch (mode % 4) {
        case 0: {
            /* Shuffle with 10 operands */
            v8sf fresult = __builtin_shuffle(fa, fb,
                0, 8, 1, 9, 2, 10, 3, 11);
            fresult = fresult * (v8sf){1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f};
            *(v8sf*)&fdst[0] = fresult;
            break;
        }
        case 1: {
            /* Another shuffle pattern */
            v4df dresult = __builtin_shuffle(da, db,
                0, 4, 1, 5);
            dresult = dresult + (v4df){1.0, 1.0, 1.0, 1.0};
            *(v4df*)&ddst[0] = dresult;
            break;
        }
        case 2: {
            /* Complex shuffle with mixing */
            v8sf fresult = __builtin_shuffle(fa, fb,
                7, 6, 5, 4, 3, 2, 1, 0);
            /* Store intermediate result to volatile memory */
            volatile v8sf temp = fresult;
            fresult = fresult + *(v8sf*)&temp;
            *(v8sf*)&fdst[0] = fresult;
            break;
        }
        default: {
            /* Use shufflevector for more operands */
            v8sf fresult = __builtin_shufflevector(fa, fb,
                0, 8, 2, 10, 4, 12, 6, 14);
            *(v8sf*)&fdst[0] = fresult;
        }
    }
}
#endif

/* SSE2 version for baseline testing */
#ifdef __SSE2__
void shuffle_sse2_vectors(int* src, int* dst, volatile int mask) {
    v4si a = *(v4si*)&src[0];
    v4si b = *(v4si*)&src[4];
    
    /* Nested if-else to create complex control flow */
    if (mask > 0) {
        if (mask > 10) {
            v4si result = __builtin_shuffle(a, b,
                (mask + 0) % 4, (mask + 1) % 4, (mask + 2) % 4, (mask + 3) % 4);
            result = result - a;
            *(v4si*)&dst[0] = result;
        } else {
            v4si result = __builtin_shuffle(a, b, 3, 2, 1, 0);
            *(v4si*)&dst[0] = result;
        }
    } else {
        v4si result = a + b;
        *(v4si*)&dst[0] = result;
    }
}
#endif

/* Main processing loop with mixed operations */
void process_vectors(int iterations, volatile int start_mask) {
    int temp_buffer[64];
    float temp_float_buffer[64];
    double temp_double_buffer[32];
    
    for (int i = 0; i < iterations; i++) {
        volatile int mask = (start_mask + i * 7) % 32;
        
        /* Call different shuffle functions based on architecture */
#ifdef __SSE2__
        shuffle_sse2_vectors(&global_int_array[i * 8], temp_buffer, mask);
        
        /* Accumulate results */
        for (int j = 0; j < 4; j++) {
            global_accumulator[i * 8 + j] += temp_buffer[j];
        }
#endif

#ifdef __AVX2__
        shuffle_large_int_vector(&global_int_array[i * 16], temp_buffer, mask);
        
        /* Mix with floating point operations */
        shuffle_float_double_mixed(
            &global_float_array[i * 16],
            (double*)&global_float_array[i * 16],  /* Reuse as doubles */
            temp_float_buffer,
            temp_double_buffer,
            mask
        );
        
        /* Accumulate integer results */
        for (int j = 0; j < 8; j++) {
            global_accumulator[i * 16 + j] += temp_buffer[j];
            global_accumulator[i * 16 + j + 8] += (int64_t)temp_float_buffer[j];
        }
#endif

#ifdef __AVX512F__
        if (i % 3 == 0) {
            shuffle_very_large_vector(&global_int_array[i * 32], temp_buffer, mask);
            
            /* Accumulate results */
            for (int j = 0; j < 16; j++) {
                global_accumulator[i * 32 + j] += temp_buffer[j];
            }
        }
#endif
    }
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    /* Process vectors with complex control flow */
    volatile int start_mask = seed % 100;
    process_vectors(10, start_mask);
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += global_accumulator[i];
    }
    
    printf("Result checksum: %ld\n", (long)checksum);
    
    /* Additional volatile operations to ensure all code paths are considered */
    volatile int dummy = 0;
    if (dummy) {
        /* This code should never execute, but prevents optimization */
#ifdef __AVX512F__
        v16si test_vec = {0};
        v16si result = __builtin_shufflevector(test_vec, test_vec,
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        dummy = result[0];
#endif
    }
    
    return 0;
}
