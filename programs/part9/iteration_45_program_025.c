#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static int32_t global_ints[512];
static double global_floats[512];
static volatile int32_t volatile_mask[64];
static int32_t accumulator[512] = {0};

/* Initialize with deterministic pseudo-random sequence */
static void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (rand() % 1000) * 0.1;
    }
    for (int i = 0; i < 64; i++) {
        volatile_mask[i] = rand() % 32;
    }
}

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double (512-bit) */
typedef int32_t v4si __attribute__((vector_size(16)));      /* 4x int32 (128-bit) */
typedef double v2df __attribute__((vector_size(16)));       /* 2x double (128-bit) */

/* Function that uses __builtin_shuffle with many operands */
static v16si shuffle_16si_complex(v16si a, v16si b, v16si c, v16si d, 
                                  v4si mask1, v4si mask2, v4si mask3, v4si mask4) {
    /* Create control mask from multiple inputs - forces runtime evaluation */
    v16si control;
    
    /* Use volatile to prevent constant folding */
    volatile int idx0 = volatile_mask[0];
    volatile int idx1 = volatile_mask[1];
    volatile int idx2 = volatile_mask[2];
    
    /* Complex control flow to keep shuffle operands alive */
    if (idx0 > 500) {
        /* First shuffle: interleave elements from all 4 vectors */
        v16si temp = __builtin_shufflevector(a, b, 
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        
        /* Second shuffle with different vectors */
        v16si temp2 = __builtin_shufflevector(c, d,
            8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
            
        /* Combine results - this may require many operands during expansion */
        return __builtin_shuffle(temp, temp2, 
            mask1[0], mask1[1], mask1[2], mask1[3],
            mask2[0], mask2[1], mask2[2], mask2[3],
            mask3[0], mask3[1], mask3[2], mask3[3],
            mask4[0], mask4[1], mask4[2], mask4[3]);
    } else {
        /* Alternative shuffle pattern */
        v16si temp = __builtin_shufflevector(b, a,
            31, 15, 30, 14, 29, 13, 28, 12, 27, 11, 26, 10, 25, 9, 24, 8);
            
        v16si temp2 = __builtin_shufflevector(d, c,
            7, 23, 6, 22, 5, 21, 4, 20, 3, 19, 2, 18, 1, 17, 0, 16);
            
        /* Another complex shuffle with many operands */
        return __builtin_shuffle(temp, temp2,
            mask4[3], mask4[2], mask4[1], mask4[0],
            mask3[3], mask3[2], mask3[1], mask3[0],
            mask2[3], mask2[2], mask2[1], mask2[0],
            mask1[3], mask1[2], mask1[1], mask1[0]);
    }
}

/* Mixed integer/float shuffle function */
static v8df shuffle_mixed_types(v8df a, v8df b, v4df c, v4df d,
                                v4si int_mask, v2df float_mask) {
    volatile int idx = volatile_mask[3] % 4;
    
    switch (idx) {
        case 0: {
            /* Shuffle within 512-bit vector */
            v8df temp = __builtin_shuffle(a, b,
                0, 8, 1, 9, 2, 10, 3, 11);
                
            /* Narrow to 256-bit */
            v4df narrowed = __builtin_shufflevector(temp, temp,
                0, 2, 4, 6);
                
            /* Expand back with shuffle */
            return __builtin_shufflevector(narrowed, c,
                0, 1, 2, 3, 4, 5, 6, 7);
        }
        
        case 1: {
            /* Complex shuffle with conversion-like pattern */
            v8df result = __builtin_shuffle(a, b,
                int_mask[0] % 8, int_mask[1] % 8, 
                int_mask[2] % 8, int_mask[3] % 8,
                4, 5, 6, 7);
                
            /* Additional shuffle with float mask */
            return __builtin_shuffle(result, result,
                (int)float_mask[0] % 8, (int)float_mask[1] % 8,
                2, 3, 4, 5, 6, 7);
        }
        
        default: {
            /* Fallback with simple shuffle */
            return __builtin_shuffle(a, a, 7, 6, 5, 4, 3, 2, 1, 0);
        }
    }
}

/* Function with loop-dependent shuffle patterns */
static void process_vector_chunk(int chunk_idx) {
    /* Load data from global arrays */
    v16si int_vec1 = *(v16si*)&global_ints[chunk_idx * 16];
    v16si int_vec2 = *(v16si*)&global_ints[chunk_idx * 16 + 16];
    v16si int_vec3 = *(v16si*)&global_ints[chunk_idx * 16 + 32];
    v16si int_vec4 = *(v16si*)&global_ints[chunk_idx * 16 + 48];
    
    v8df float_vec1 = *(v8df*)&global_floats[chunk_idx * 8];
    v8df float_vec2 = *(v8df*)&global_floats[chunk_idx * 8 + 8];
    v4df float_vec3 = *(v4df*)&global_floats[chunk_idx * 8 + 16];
    v4df float_vec4 = *(v4df*)&global_floats[chunk_idx * 8 + 20];
    
    /* Create masks from loop-dependent values */
    v4si mask1 = {chunk_idx, chunk_idx + 1, chunk_idx + 2, chunk_idx + 3};
    v4si mask2 = {chunk_idx + 4, chunk_idx + 5, chunk_idx + 6, chunk_idx + 7};
    v4si mask3 = {chunk_idx + 8, chunk_idx + 9, chunk_idx + 10, chunk_idx + 11};
    v4si mask4 = {chunk_idx + 12, chunk_idx + 13, chunk_idx + 14, chunk_idx + 15};
    
    v2df float_mask = {(double)(chunk_idx % 8), (double)((chunk_idx + 1) % 8)};
    
    /* Perform complex shuffle operations */
    v16si shuffled_ints = shuffle_16si_complex(int_vec1, int_vec2, 
                                               int_vec3, int_vec4,
                                               mask1, mask2, mask3, mask4);
    
    v8df shuffled_floats = shuffle_mixed_types(float_vec1, float_vec2,
                                               float_vec3, float_vec4,
                                               mask1, float_mask);
    
    /* Do some arithmetic to use the results */
    v16si int_result = shuffled_ints + int_vec1;
    v8df float_result = shuffled_floats * float_vec1;
    
    /* Store to accumulator with volatile write to prevent elimination */
    volatile int32_t* volatile_ptr = (volatile int32_t*)&accumulator[chunk_idx * 16];
    memcpy((void*)volatile_ptr, &int_result, sizeof(int_result));
    
    /* Also accumulate float results as integers */
    int64_t* float_as_int = (int64_t*)&float_result;
    for (int i = 0; i < 8; i++) {
        accumulator[chunk_idx * 16 + i] += (int32_t)float_as_int[i];
    }
}

/* Architecture-specific variants */
#ifdef __AVX512F__
static v16si avx512_shuffle_16si(v16si a, v16si b, v16si c, v16si d,
                                 v16si mask) {
    /* AVX512-specific shuffle pattern */
    return __builtin_shuffle(a, b, 
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11],
        mask[12], mask[13], mask[14], mask[15]);
}
#endif

#ifdef __AVX2__
static v8si avx2_shuffle_8si(v8si a, v8si b, v8si c, v8si d,
                             v8si mask) {
    /* AVX2 shuffle with many operands */
    v8si temp1 = __builtin_shufflevector(a, b, 0, 8, 1, 9, 2, 10, 3, 11);
    v8si temp2 = __builtin_shufflevector(c, d, 4, 12, 5, 13, 6, 14, 7, 15);
    
    return __builtin_shuffle(temp1, temp2,
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7]);
}
#endif

#ifdef __SSE2__
static v4si sse2_shuffle_4si(v4si a, v4si b, v4si c, v4si d,
                             v4si mask1, v4si mask2) {
    /* SSE2 shuffle pattern */
    v4si temp = __builtin_shufflevector(a, b, mask1[0], mask1[1], mask1[2], mask1[3]);
    v4si temp2 = __builtin_shufflevector(c, d, mask2[0], mask2[1], mask2[2], mask2[3]);
    
    return __builtin_shuffle(temp, temp2, 0, 1, 2, 3);
}
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    init_arrays(seed);
    
    /* Process multiple chunks with different shuffle patterns */
    for (int chunk = 0; chunk < 10; chunk++) {
        process_vector_chunk(chunk % 16);
        
        /* Architecture-specific code paths */
#ifdef __AVX512F__
        if (chunk % 3 == 0) {
            v16si avx512_vec1 = *(v16si*)&global_ints[chunk * 16];
            v16si avx512_vec2 = *(v16si*)&global_ints[chunk * 16 + 16];
            v16si avx512_vec3 = *(v16si*)&global_ints[chunk * 16 + 32];
            v16si avx512_vec4 = *(v16si*)&global_ints[chunk * 16 + 48];
            v16si avx512_mask = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
            
            v16si avx512_result = avx512_shuffle_16si(avx512_vec1, avx512_vec2,
                                                      avx512_vec3, avx512_vec4,
                                                      avx512_mask);
            /* Use the result */
            for (int i = 0; i < 16; i++) {
                accumulator[chunk * 16 + i] += avx512_result[i];
            }
        }
#endif
        
#ifdef __AVX2__
        if (chunk % 4 == 1) {
            v8si avx2_vec1 = *(v8si*)&global_ints[chunk * 8];
            v8si avx2_vec2 = *(v8si*)&global_ints[chunk * 8 + 8];
            v8si avx2_vec3 = *(v8si*)&global_ints[chunk * 8 + 16];
            v8si avx2_vec4 = *(v8si*)&global_ints[chunk * 8 + 24];
            v8si avx2_mask = {0,1,2,3,4,5,6,7};
            
            v8si avx2_result = avx2_shuffle_8si(avx2_vec1, avx2_vec2,
                                                avx2_vec3, avx2_vec4,
                                                avx2_mask);
            for (int i = 0; i < 8; i++) {
                accumulator[chunk * 8 + i] += avx2_result[i];
            }
        }
#endif
    }
    
    /* Compute final checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
