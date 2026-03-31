#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int32_t global_int_data[512];
static volatile double global_float_data[512];
static volatile int32_t accumulator[512];

/* Initialize with deterministic pseudo-random sequence */
void init_data(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = rand() % 1000;
        global_float_data[i] = (rand() % 1000) * 0.1;
        accumulator[i] = 0;
    }
}

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double */
typedef int64_t v4di __attribute__((vector_size(32)));      /* 4x int64 */
typedef int64_t v8di __attribute__((vector_size(64)));      /* 8x int64 */

/* Function to create variable indices - prevents constant folding */
static inline v16si make_variable_mask16(int base, volatile int offset) {
    v16si mask = {0};
    for (int i = 0; i < 16; i++) {
        mask[i] = (base + i + offset) % 16;
    }
    return mask;
}

static inline v8si make_variable_mask8(int base, volatile int offset) {
    v8si mask = {0};
    for (int i = 0; i < 8; i++) {
        mask[i] = (base + i + offset) % 8;
    }
    return mask;
}

/* Complex shuffle with 10+ operands - targeting case 10/11 */
void shuffle_10_operand_int(volatile int32_t* src, volatile int32_t* dst, 
                           int iter, volatile int mask_offset) {
    /* Load data into large vectors */
    v16si data1 = *(v16si*)&src[0];
    v16si data2 = *(v16si*)&src[16];
    v16si data3 = *(v16si*)&src[32];
    v16si data4 = *(v16si*)&src[48];
    
    /* Create variable masks */
    v16si mask1 = make_variable_mask16(iter * 3, mask_offset);
    v16si mask2 = make_variable_mask16(iter * 7, mask_offset + 1);
    
    /* Complex nested control flow */
    if (mask_offset % 3 == 0) {
        /* This shuffle uses 10 operands: 4 data vectors + 2 masks + 4 temp results */
        v16si temp1 = __builtin_shuffle(data1, data2, mask1);
        v16si temp2 = __builtin_shuffle(data3, data4, mask2);
        
        /* Another shuffle combining results - potentially 11 operands in expansion */
        v16si mask3 = make_variable_mask16(iter * 11, mask_offset + 2);
        v16si result = __builtin_shuffle(temp1, temp2, mask3);
        
        /* Store with volatile to prevent elimination */
        *(volatile v16si*)&dst[0] = result;
    } else if (mask_offset % 3 == 1) {
        /* Alternative path with different shuffle pattern */
        v16si mask4 = make_variable_mask16(iter * 5, mask_offset + 3);
        v16si result = __builtin_shuffle(data1, data3, mask4);
        *(volatile v16si*)&dst[16] = result;
    } else {
        /* Third path with shufflevector */
        v16si result = __builtin_shufflevector(data1, data2, 
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        *(volatile v16si*)&dst[32] = result;
    }
}

/* Mixed floating-point and integer shuffles */
void shuffle_mixed_types(volatile double* src_f, volatile int32_t* dst_i,
                        int iter, volatile int selector) {
    v8df fdata1 = *(v8df*)&src_f[0];
    v8df fdata2 = *(v8df*)&src_f[8];
    
    /* Switch statement to create complex control flow */
    switch (selector % 4) {
        case 0: {
            v8di imask = {0, 8, 1, 9, 2, 10, 3, 11};
            /* Shuffle with conversion-like pattern */
            v8df shuffled = __builtin_shuffle(fdata1, fdata2, imask);
            
            /* Convert and store */
            v8si converted = { (int32_t)shuffled[0], (int32_t)shuffled[1],
                              (int32_t)shuffled[2], (int32_t)shuffled[3],
                              (int32_t)shuffled[4], (int32_t)shuffled[5],
                              (int32_t)shuffled[6], (int32_t)shuffled[7] };
            *(volatile v8si*)&dst_i[0] = converted;
            break;
        }
        case 1: {
            /* Different shuffle pattern */
            v8di mask = {7, 6, 5, 4, 3, 2, 1, 0};
            v8df shuffled = __builtin_shuffle(fdata1, mask);
            v8si converted = { (int32_t)shuffled[0], (int32_t)shuffled[1],
                              (int32_t)shuffled[2], (int32_t)shuffled[3],
                              (int32_t)shuffled[4], (int32_t)shuffled[5],
                              (int32_t)shuffled[6], (int32_t)shuffled[7] };
            *(volatile v8si*)&dst_i[8] = converted;
            break;
        }
        case 2:
        case 3: {
            /* Complex shuffle with many operands */
            v8df fdata3 = *(v8df*)&src_f[16];
            v8di mask1 = {0, 8, 2, 10, 4, 12, 6, 14};
            v8di mask2 = {1, 9, 3, 11, 5, 13, 7, 15};
            
            v8df temp1 = __builtin_shuffle(fdata1, fdata2, mask1);
            v8df temp2 = __builtin_shuffle(fdata1, fdata3, mask2);
            
            v8di mask3 = {0, 2, 4, 6, 1, 3, 5, 7};
            v8df result = __builtin_shuffle(temp1, temp2, mask3);
            
            v8si converted = { (int32_t)result[0], (int32_t)result[1],
                              (int32_t)result[2], (int32_t)result[3],
                              (int32_t)result[4], (int32_t)result[5],
                              (int32_t)result[6], (int32_t)result[7] };
            *(volatile v8si*)&dst_i[16] = converted;
            break;
        }
    }
}

/* Narrowing and widening operations */
void narrowing_widening_shuffle(volatile int32_t* src, int iter) {
    v16si wide1 = *(v16si*)&src[0];
    v16si wide2 = *(v16si*)&src[16];
    
    /* Narrow: 512-bit -> 256-bit */
    v8si narrow_mask = make_variable_mask8(iter, 0);
    v8si narrowed = __builtin_shuffle(
        (v8si)wide1, (v8si)wide2, narrow_mask);
    
    /* Widen back: 256-bit -> 512-bit with pattern */
    v16si widen_mask = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
    v16si widened = __builtin_shuffle(
        (v16si)narrowed, (v16si)narrowed, widen_mask);
    
    /* Accumulate result */
    for (int i = 0; i < 16; i++) {
        accumulator[iter * 16 + i] += widened[i];
    }
}

/* Architecture-specific variants */
#ifdef __AVX512F__
void avx512_specific_shuffle(volatile int32_t* src, int iter) {
    v16si data1 = *(v16si*)&src[0];
    v16si data2 = *(v16si*)&src[16];
    v16si data3 = *(v16si*)&src[32];
    
    /* Complex shuffle pattern that might use 11 operands */
    v16si mask1 = make_variable_mask16(iter, 0);
    v16si mask2 = make_variable_mask16(iter * 2, 1);
    v16si mask3 = make_variable_mask16(iter * 3, 2);
    
    v16si temp1 = __builtin_shuffle(data1, data2, mask1);
    v16si temp2 = __builtin_shuffle(data2, data3, mask2);
    v16si result = __builtin_shuffle(temp1, temp2, mask3);
    
    /* Mix with arithmetic */
    result = result + mask1;
    
    for (int i = 0; i < 16; i++) {
        accumulator[iter * 16 + i] += result[i];
    }
}
#endif

#ifdef __AVX2__
void avx2_specific_shuffle(volatile int32_t* src, int iter) {
    v8si data1 = *(v8si*)&src[0];
    v8si data2 = *(v8si*)&src[8];
    v8si data3 = *(v8si*)&src[16];
    v8si data4 = *(v8si*)&src[24];
    
    v8si mask1 = make_variable_mask8(iter, 0);
    v8si mask2 = make_variable_mask8(iter * 2, 1);
    
    /* Chain of shuffles */
    v8si temp1 = __builtin_shuffle(data1, data2, mask1);
    v8si temp2 = __builtin_shuffle(data3, data4, mask2);
    
    v8si mask3 = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si result = __builtin_shuffle(temp1, temp2, mask3);
    
    for (int i = 0; i < 8; i++) {
        accumulator[iter * 8 + i] += result[i];
    }
}
#endif

#ifdef __SSE2__
void sse2_specific_shuffle(volatile int32_t* src, int iter) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si data1 = *(v4si*)&src[0];
    v4si data2 = *(v4si*)&src[4];
    v4si data3 = *(v4si*)&src[8];
    v4si data4 = *(v4si*)&src[12];
    
    /* Multiple shuffles to increase operand count */
    v4si mask1 = {3, 2, 1, 0};
    v4si mask2 = {1, 0, 3, 2};
    
    v4si temp1 = __builtin_shuffle(data1, data2, mask1);
    v4si temp2 = __builtin_shuffle(data3, data4, mask2);
    
    v4si mask3 = {0, 4, 1, 5};
    v4si result = __builtin_shuffle(temp1, temp2, mask3);
    
    for (int i = 0; i < 4; i++) {
        accumulator[iter * 4 + i] += result[i];
    }
}
#endif

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    volatile int mask_offset = seed % 100;
    
    /* Main loop with complex control flow */
    for (int iter = 0; iter < 10; iter++) {
        volatile int selector = (iter + seed) % 10;
        
        /* Call different shuffle functions based on control flow */
        if (iter % 3 == 0) {
            shuffle_10_operand_int((int32_t*)global_int_data, 
                                  (int32_t*)accumulator, 
                                  iter, mask_offset + iter);
        } else if (iter % 3 == 1) {
            shuffle_mixed_types((double*)global_float_data,
                               (int32_t*)accumulator,
                               iter, selector);
        } else {
            narrowing_widening_shuffle((int32_t*)global_int_data, iter);
        }
        
        /* Architecture-specific paths */
#ifdef __AVX512F__
        if (selector % 2 == 0) {
            avx512_specific_shuffle((int32_t*)global_int_data, iter);
        }
#endif
        
#ifdef __AVX2__
        if (selector % 3 == 0) {
            avx2_specific_shuffle((int32_t*)global_int_data, iter);
        }
#endif
        
#ifdef __SSE2__
        if (selector % 5 == 0) {
            sse2_specific_shuffle((int32_t*)global_int_data, iter);
        }
#endif
        
        /* Modify mask_offset to create varying patterns */
        mask_offset = (mask_offset * 13 + 7) % 64;
    }
    
    /* Compute final checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    return 0;
}
