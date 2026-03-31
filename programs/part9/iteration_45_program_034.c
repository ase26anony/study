#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int32_t global_ints[512];
static volatile double global_floats[512];
static volatile int32_t accumulator[512];

/* Initialize with deterministic pseudo-random sequence */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = (rand() % 1000) - 500;
        global_floats[i] = (rand() % 1000) / 100.0 - 5.0;
        accumulator[i] = 0;
    }
}

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Function to create variable indices - prevents constant folding */
static inline v16si make_mask16(int base, volatile int* counter) {
    v16si mask = {0};
    int* m = (int*)&mask;
    for (int i = 0; i < 16; i++) {
        m[i] = (base + i + *counter) % 32;  /* Use 32 to allow wrapping */
    }
    return mask;
}

static inline v8si make_mask8(int base, volatile int* counter) {
    v8si mask = {0};
    int* m = (int*)&mask;
    for (int i = 0; i < 8; i++) {
        m[i] = (base + i + *counter) % 16;
    }
    return mask;
}

/* Complex shuffle with 10+ operands - integer version */
#ifdef __AVX512F__
void shuffle_int_512(volatile int32_t* src, volatile int32_t* dst, 
                     int pattern, volatile int* counter) {
    /* Load 4 vectors (64 bytes each = 512 bits total) */
    v16si v1 = *(v16si*)&src[0];
    v16si v2 = *(v16si*)&src[16];
    v16si v3 = *(v16si*)&src[32];
    v16si v4 = *(v16si*)&src[48];
    
    /* Create control masks from runtime values */
    v16si mask1 = make_mask16(pattern * 4, counter);
    v16si mask2 = make_mask16(pattern * 4 + 16, counter);
    
    /* Complex shuffle operation requiring many operands */
    v16si result;
    if (pattern % 3 == 0) {
        /* This should trigger 10+ operand expansion */
        result = __builtin_shuffle(v1, v2, v3, v4, 
                                   mask1, mask2,
                                   v1, v2,  /* Extra vectors to reach operand count */
                                   *(v16si*)&src[64]);  /* 10th operand */
    } else if (pattern % 3 == 1) {
        /* Alternative pattern with different operand arrangement */
        result = __builtin_shufflevector(v1, v2, v3, v4,
                                         0, 16, 1, 17, 2, 18, 3, 19,
                                         4, 20, 5, 21, 6, 22, 7, 23,
                                         8, 24, 9, 25, 10, 26, 11, 27,
                                         12, 28, 13, 29, 14, 30, 15, 31);
    } else {
        /* Mix of shuffle types */
        v16si temp = __builtin_shuffle(v1, v2, mask1);
        result = __builtin_shuffle(temp, v3, mask2);
    }
    
    /* Store with volatile to prevent elimination */
    *(v16si*)dst = result;
    
    /* Additional operation to create use of result */
    v16si scaled = result + (result >> 2);
    *(v16si*)&accumulator[pattern * 16] += scaled;
}
#endif

#ifdef __AVX2__
void shuffle_double_256(volatile double* src, volatile double* dst,
                        int pattern, volatile int* counter) {
    /* Load vectors */
    v4df v1 = *(v4df*)&src[0];
    v4df v2 = *(v4df*)&src[4];
    v4df v3 = *(v4df*)&src[8];
    v4df v4 = *(v4df*)&src[12];
    
    /* Create integer mask for shuffle */
    v8si imask = make_mask8(pattern * 2, counter);
    
    /* Convert to appropriate mask type for double shuffle */
    long long mask_arr[4];
    int* imask_ptr = (int*)&imask;
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = imask_ptr[i] % 8;
    }
    
    v4df mask_vec = *(v4df*)mask_arr;
    
    /* Complex control flow with multiple shuffle patterns */
    switch (pattern % 5) {
        case 0:
        case 1: {
            /* Multi-operand shuffle sequence */
            v4df t1 = __builtin_shuffle(v1, v2, mask_vec);
            v4df t2 = __builtin_shuffle(v3, v4, mask_vec);
            *(v4df*)dst = t1 + t2;
            break;
        }
        case 2:
        case 3: {
            /* Different operand arrangement */
            v4df temp[4] = {v1, v2, v3, v4};
            v4df result = __builtin_shuffle(temp[0], temp[1], temp[2], temp[3],
                                            mask_vec, mask_vec,
                                            *(v4df*)&src[16],  /* Extra operands */
                                            *(v4df*)&src[20]);
            *(v4df*)dst = result;
            break;
        }
        default: {
            /* Simple case */
            *(v4df*)dst = __builtin_shuffle(v1, v2, mask_vec);
            break;
        }
    }
    
    /* Use result in computation */
    v4df result = *(v4df*)dst;
    v4df scaled = result * 1.5;
    
    /* Accumulate */
    for (int i = 0; i < 4; i++) {
        accumulator[pattern * 4 + i] += (int32_t)scaled[i];
    }
}
#endif

/* Mixed-size vector operations */
#ifdef __SSE2__
void mixed_shuffle_128(volatile int32_t* src_int, volatile double* src_dbl,
                       volatile int32_t* dst, int pattern, volatile int* counter) {
    /* Load different types */
    typedef int32_t v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    v4si vi1 = *(v4si*)&src_int[0];
    v4si vi2 = *(v4si*)&src_int[4];
    v2df vd1 = *(v2df*)&src_dbl[0];
    v2df vd2 = *(v2df*)&src_dbl[2];
    
    /* Create masks from runtime values */
    int mask_arr[4];
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = (pattern + i + *counter) % 8;
    }
    
    /* Complex if-else chain with different shuffle patterns */
    if (pattern % 2 == 0) {
        /* Integer shuffle with many operands */
        v4si mask_vec = *(v4si*)mask_arr;
        v4si result = __builtin_shuffle(vi1, vi2, vi1, vi2,  /* Multiple vectors */
                                        mask_vec, mask_vec,
                                        *(v4si*)&src_int[8],
                                        *(v4si*)&src_int[12]);
        *(v4si*)dst = result;
    } else {
        /* Float shuffle */
        long long df_mask[2] = {mask_arr[0] % 4, mask_arr[1] % 4};
        v2df df_mask_vec = *(v2df*)df_mask;
        v2df result = __builtin_shuffle(vd1, vd2, vd1, vd2,
                                        df_mask_vec, df_mask_vec,
                                        *(v2df*)&src_dbl[4]);
        /* Convert and store */
        int32_t* res_ptr = (int32_t*)&result;
        for (int i = 0; i < 2; i++) {
            dst[i] = (int32_t)res_ptr[i];
        }
    }
    
    /* Accumulate */
    for (int i = 0; i < 4; i++) {
        accumulator[pattern * 4 + i] += dst[i];
    }
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    volatile int counter = 0;
    
    /* Loop with complex control flow */
    for (int iter = 0; iter < 10; iter++) {
        counter = iter;
        
        /* Nested if-else to stress control flow around vector ops */
        if (iter % 3 == 0) {
            #ifdef __AVX512F__
            shuffle_int_512((int32_t*)global_ints, 
                           (int32_t*)&global_ints[256],
                           iter, &counter);
            #endif
        } else if (iter % 3 == 1) {
            #ifdef __AVX2__
            shuffle_double_256((double*)global_floats,
                              (double*)&global_floats[256],
                              iter, &counter);
            #endif
        } else {
            #ifdef __SSE2__
            mixed_shuffle_128((int32_t*)global_ints,
                             (double*)global_floats,
                             (int32_t*)&global_ints[128],
                             iter, &counter);
            #endif
        }
        
        /* Additional volatile operation to prevent reordering */
        volatile int barrier = counter;
        (void)barrier;
    }
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    return 0;
}
