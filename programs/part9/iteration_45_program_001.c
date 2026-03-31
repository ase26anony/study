#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator[512] __attribute__((aligned(64)));

/* Vector type definitions */
typedef int v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));    /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));   /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));   /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));   /* 512-bit double */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        global_int_array[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
        global_float_array[i] = (float)((i * 1103515245 + seed) & 0xFF) / 256.0f;
        accumulator[i] = 0;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
#ifdef __AVX2__
void shuffle_v16si_complex(int* dest, int* src, volatile int* mask_indices, int iter) {
    v16si vec1 = *(v16si*)(src);
    v16si vec2 = *(v16si*)(src + 16);
    v16si vec3 = *(v16si*)(src + 32);
    
    /* Use volatile variables for mask indices to prevent constant folding */
    volatile int idx0 = mask_indices[0] + iter;
    volatile int idx1 = mask_indices[1] + iter;
    volatile int idx2 = mask_indices[2] + iter;
    volatile int idx3 = mask_indices[3] + iter;
    volatile int idx4 = mask_indices[4] + iter;
    volatile int idx5 = mask_indices[5] + iter;
    volatile int idx6 = mask_indices[6] + iter;
    volatile int idx7 = mask_indices[7] + iter;
    volatile int idx8 = mask_indices[8] + iter;
    volatile int idx9 = mask_indices[9] + iter;
    volatile int idx10 = mask_indices[10] + iter;
    volatile int idx11 = mask_indices[11] + iter;
    volatile int idx12 = mask_indices[12] + iter;
    volatile int idx13 = mask_indices[13] + iter;
    volatile int idx14 = mask_indices[14] + iter;
    volatile int idx15 = mask_indices[15] + iter;
    
    /* Create mask vector from volatile indices - forces runtime evaluation */
    v16si mask = {
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15
    };
    
    /* Complex control flow to stress expander */
    if (global_seed % 3 == 0) {
        /* Case requiring 10+ operands: shuffle with mask */
        v16si result = __builtin_shuffle(vec1, vec2, mask);
        *(v16si*)dest = result;
    } else if (global_seed % 3 == 1) {
        /* Alternative shuffle pattern */
        v16si temp = __builtin_shuffle(vec1, vec3, mask);
        v16si result = __builtin_shuffle(temp, vec2, mask);
        *(v16si*)dest = result;
    } else {
        /* Nested shuffle operations */
        v16si temp1 = __builtin_shuffle(vec1, vec2, mask);
        v16si temp2 = __builtin_shuffle(vec3, vec1, mask);
        v16si result = __builtin_shuffle(temp1, temp2, mask);
        *(v16si*)dest = result;
    }
}
#endif

/* Function 2: Mixed float/double shuffles with __builtin_shufflevector */
#ifdef __AVX512F__
void shuffle_mixed_types(float* dest_f, double* dest_d, 
                         float* src_f, double* src_d, 
                         volatile int* mask, int iter) {
    v16sf fvec1 = *(v16sf*)src_f;
    v16sf fvec2 = *(v16sf*)(src_f + 16);
    v8df dvec1 = *(v8df*)src_d;
    v8df dvec2 = *(v8df*)(src_d + 8);
    
    volatile int m0 = mask[0] + iter;
    volatile int m1 = mask[1] + iter;
    volatile int m2 = mask[2] + iter;
    volatile int m3 = mask[3] + iter;
    volatile int m4 = mask[4] + iter;
    volatile int m5 = mask[5] + iter;
    volatile int m6 = mask[6] + iter;
    volatile int m7 = mask[7] + iter;
    volatile int m8 = mask[8] + iter;
    volatile int m9 = mask[9] + iter;
    volatile int m10 = mask[10] + iter;
    volatile int m11 = mask[11] + iter;
    volatile int m12 = mask[12] + iter;
    volatile int m13 = mask[13] + iter;
    volatile int m14 = mask[14] + iter;
    volatile int m15 = mask[15] + iter;
    
    /* Switch statement with volatile condition */
    switch (global_seed % 4) {
        case 0: {
            /* __builtin_shufflevector with many operands */
            v16sf fresult = __builtin_shufflevector(fvec1, fvec2, 
                m0, m1, m2, m3, m4, m5, m6, m7,
                m8, m9, m10, m11, m12, m13, m14, m15);
            *(v16sf*)dest_f = fresult;
            break;
        }
        case 1: {
            /* Double vector shuffle */
            v8df dresult = __builtin_shufflevector(dvec1, dvec2,
                m0, m1, m2, m3, m4, m5, m6, m7);
            *(v8df*)dest_d = dresult;
            break;
        }
        case 2: {
            /* Mixed operation - requires type conversion */
            v16sf temp = __builtin_shufflevector(fvec1, fvec2,
                m0, m1, m2, m3, m4, m5, m6, m7,
                m8, m9, m10, m11, m12, m13, m14, m15);
            /* Additional arithmetic to create more complex pattern */
            v16sf scaled = temp * (v16sf){2.0f};
            *(v16sf*)dest_f = scaled;
            break;
        }
        case 3: {
            /* Nested shuffles in loop */
            v8df dtemp = dvec1;
            for (int i = 0; i < 3; i++) {
                dtemp = __builtin_shufflevector(dtemp, dvec2,
                    m0 + i, m1 + i, m2 + i, m3 + i,
                    m4 + i, m5 + i, m6 + i, m7 + i);
            }
            *(v8df*)dest_d = dtemp;
            break;
        }
    }
}
#endif

/* Function 3: SSE2-compatible shuffles with narrowing/expanding */
#ifdef __SSE2__
void shuffle_narrow_expand(int* dest, int* src, volatile int* mask, int iter) {
    typedef int v4si __attribute__((vector_size(16)));
    
    v4si vec1 = *(v4si*)src;
    v4si vec2 = *(v4si*)(src + 4);
    v4si vec3 = *(v4si*)(src + 8);
    v4si vec4 = *(v4si*)(src + 12);
    
    volatile int m0 = (mask[0] + iter) % 8;
    volatile int m1 = (mask[1] + iter) % 8;
    volatile int m2 = (mask[2] + iter) % 8;
    volatile int m3 = (mask[3] + iter) % 8;
    volatile int m4 = (mask[4] + iter) % 8;
    volatile int m5 = (mask[5] + iter) % 8;
    volatile int m6 = (mask[6] + iter) % 8;
    volatile int m7 = (mask[7] + iter) % 8;
    
    /* Complex if-else chain */
    if (iter % 2 == 0) {
        /* Narrowing operation */
        v4si narrow = __builtin_shufflevector(vec1, vec2, m0, m1, m2, m3);
        v4si expanded = __builtin_shufflevector(narrow, vec3, m4, m5, m6, m7);
        *(v4si*)dest = expanded;
    } else {
        /* Different shuffle pattern */
        v4si temp1 = __builtin_shuffle(vec1, vec2, (v4si){m0, m1, m2, m3});
        v4si temp2 = __builtin_shuffle(vec3, vec4, (v4si){m4, m5, m6, m7});
        v4si result = __builtin_shuffle(temp1, temp2, (v4si){m0, m1, m4, m5});
        *(v4si*)dest = result;
    }
}
#endif

/* Function 4: Large shuffle with arithmetic operations */
void large_shuffle_with_ops(int* dest, int* src1, int* src2, 
                           volatile int* mask, int iter, int pattern) {
    v8si vec1 = *(v8si*)src1;
    v8si vec2 = *(v8si*)src2;
    
    volatile int indices[16];
    for (int i = 0; i < 16; i++) {
        indices[i] = (mask[i] + iter * 7) % 16;
    }
    
    /* Multiple shuffle patterns in switch */
    switch (pattern % 3) {
        case 0: {
            v8si mask_vec = {
                indices[0], indices[1], indices[2], indices[3],
                indices[4], indices[5], indices[6], indices[7]
            };
            v8si shuffled = __builtin_shuffle(vec1, vec2, mask_vec);
            v8si result = shuffled + vec1;  /* Arithmetic op */
            *(v8si*)dest = result;
            break;
        }
        case 1: {
            /* Two consecutive shuffles */
            v8si temp = __builtin_shuffle(vec1, vec2, 
                (v8si){indices[0], indices[1], indices[2], indices[3],
                       indices[4], indices[5], indices[6], indices[7]});
            v8si result = __builtin_shuffle(temp, vec1,
                (v8si){indices[8], indices[9], indices[10], indices[11],
                       indices[12], indices[13], indices[14], indices[15]});
            *(v8si*)dest = result * 2;
            break;
        }
        case 2: {
            /* Shuffle with mixed operations */
            v8si shuffle1 = __builtin_shuffle(vec1, vec2,
                (v8si){indices[0], indices[1], indices[2], indices[3],
                       indices[4], indices[5], indices[6], indices[7]});
            v8si shuffle2 = __builtin_shuffle(vec2, vec1,
                (v8si){indices[8], indices[9], indices[10], indices[11],
                       indices[12], indices[13], indices[14], indices[15]});
            v8si result = shuffle1 + shuffle2;
            *(v8si*)dest = result;
            break;
        }
    }
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Mask indices for shuffles - initialized with pattern */
    volatile int mask_indices[32];
    for (int i = 0; i < 32; i++) {
        mask_indices[i] = (i * 13 + seed) % 32;
    }
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        /* Use volatile store to prevent elimination */
        volatile int* volatile_ptr = accumulator;
        
#ifdef __AVX2__
        shuffle_v16si_complex((int*)accumulator + iter * 16, 
                             global_int_array + iter * 16,
                             mask_indices, iter);
        volatile_ptr[iter * 16] = accumulator[iter * 16];  /* Force store */
#endif

#ifdef __AVX512F__
        double double_array[256];
        for (int i = 0; i < 256; i++) {
            double_array[i] = (double)global_float_array[i];
        }
        shuffle_mixed_types((float*)accumulator + iter * 32,
                           double_array + iter * 16,
                           global_float_array + iter * 32,
                           double_array + iter * 16,
                           mask_indices, iter);
#endif

#ifdef __SSE2__
        shuffle_narrow_expand(accumulator + iter * 8,
                             global_int_array + iter * 8,
                             mask_indices, iter);
#endif

        /* Always compile this function (no arch guard) */
        large_shuffle_with_ops(accumulator + iter * 16,
                              global_int_array + iter * 16,
                              global_int_array + iter * 32,
                              mask_indices, iter, seed);
        
        /* Additional volatile operation to prevent optimization */
        asm volatile("" : : "r"(accumulator) : "memory");
    }
    
    /* Compute checksum */
    long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
