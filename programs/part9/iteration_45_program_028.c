#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static int32_t global_ints[512];
static float global_floats[512];
static volatile int32_t volatile_mask[64];
static int32_t accumulator[512] = {0};

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Initialize with deterministic pseudo-random sequence */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (float)(rand() % 1000) / 10.0f;
    }
    for (int i = 0; i < 64; i++) {
        volatile_mask[i] = rand() % 32;
    }
}

/* Function using __builtin_shuffle with 10+ operands (256-bit integer) */
__attribute__((noinline))
v8si shuffle_v8si_complex(v8si a, v8si b, v8si c, v8si mask1, v8si mask2) {
    /* Create complex shuffle pattern - this may require many operands during expansion */
    v8si temp1 = __builtin_shuffle(a, b, mask1);
    v8si temp2 = __builtin_shuffle(b, c, mask2);
    
    /* Another shuffle combining results - potentially 10+ operands */
    v8si combined_mask = mask1 + mask2;
    v8si result = __builtin_shuffle(temp1, temp2, combined_mask);
    
    return result;
}

/* Function using __builtin_shufflevector with large vectors (512-bit) */
__attribute__((noinline))
v16si shuffle_v16si_many_ops(v16si a, v16si b, v16si c, v16si d, 
                             int idx0, int idx1, int idx2, int idx3,
                             int idx4, int idx5, int idx6, int idx7) {
    /* __builtin_shufflevector with many indices - may hit 11 operand case */
    v16si result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx0+8, idx1+8, idx2+8, idx3+8, idx4+8, idx5+8, idx6+8, idx7+8);
    
    /* Mix with another shuffle */
    v16si temp = __builtin_shufflevector(c, d,
        idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0,
        idx7+8, idx6+8, idx5+8, idx4+8, idx3+8, idx2+8, idx1+8, idx0+8);
    
    return result + temp;
}

/* Mixed floating-point shuffle with control flow */
__attribute__((noinline))
v8sf shuffle_v8sf_with_control(v8sf a, v8sf b, v8sf mask, int pattern) {
    v8sf result;
    
    /* Complex control flow to stress expander */
    switch (pattern & 7) {
        case 0: {
            /* Shuffle with many implicit operands */
            v8sf temp = __builtin_shuffle(a, b, mask);
            result = temp * temp;
            break;
        }
        case 1: {
            /* Different shuffle pattern */
            v8sf rev_mask = __builtin_shuffle(mask, mask, 
                (v8si){7, 6, 5, 4, 3, 2, 1, 0});
            result = __builtin_shuffle(a, b, rev_mask);
            break;
        }
        case 2: {
            /* Nested shuffles */
            v8sf temp1 = __builtin_shuffle(a, a, mask);
            v8sf temp2 = __builtin_shuffle(b, b, mask);
            result = __builtin_shuffle(temp1, temp2, mask);
            break;
        }
        default: {
            /* Complex multi-step shuffle */
            v8sf mask2 = mask + (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
            v8sf inter = __builtin_shuffle(a, b, mask);
            result = __builtin_shuffle(inter, a, mask2);
            break;
        }
    }
    
    return result;
}

/* Function that uses double vectors with volatile mask indices */
__attribute__((noinline))
v4df shuffle_v4df_volatile_mask(v4df a, v4df b, volatile int* mask_indices) {
    /* Create mask from volatile indices - prevents constant folding */
    v4df mask = {mask_indices[0] % 8, mask_indices[1] % 8, 
                 mask_indices[2] % 8, mask_indices[3] % 8};
    
    /* Convert to integer vector for shuffle */
    v8si int_mask = __builtin_convertvector(mask, v8si);
    
    /* Perform shuffle - may expand to many operands */
    v4df result = __builtin_shuffle(a, b, int_mask);
    
    return result;
}

/* Mixed SIMD patterns with narrowing/expanding */
__attribute__((noinline))
void mixed_simd_patterns(int* indices, float* floats, int iter) {
    /* Load 512-bit vectors */
    v16si vi = *(v16si*)&global_ints[iter * 16];
    v16sf vf = *(v16sf*)&global_floats[iter * 16];
    
    /* Narrow 512-bit to 256-bit via shuffle */
    v8si vi_narrow = __builtin_shufflevector(vi, vi,
        indices[0] % 16, indices[1] % 16, indices[2] % 16, indices[3] % 16,
        indices[4] % 16, indices[5] % 16, indices[6] % 16, indices[7] % 16);
    
    v8sf vf_narrow = __builtin_shufflevector(vf, vf,
        indices[8] % 16, indices[9] % 16, indices[10] % 16, indices[11] % 16,
        indices[12] % 16, indices[13] % 16, indices[14] % 16, indices[15] % 16);
    
    /* Expand back with different pattern */
    v16si vi_expanded = __builtin_shufflevector(vi_narrow, vi_narrow,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Store to accumulator */
    for (int i = 0; i < 16; i++) {
        accumulator[iter * 16 + i] += vi_expanded[i];
    }
}

/* Architecture-specific variants */
#ifdef __AVX512F__
__attribute__((noinline))
void avx512_specific_shuffle(int pattern) {
    v16si a = {0}, b = {0};
    v16si mask = {0};
    
    /* Complex shuffle that might use 10+ operands */
    for (int i = 0; i < 16; i++) {
        mask[i] = (pattern + i) % 32;
    }
    
    v16si result = __builtin_shuffle(a, b, mask);
    /* Use result to prevent elimination */
    volatile v16si* volatile_ptr = &result;
    (void)volatile_ptr;
}
#endif

#ifdef __AVX2__
__attribute__((noinline))
void avx2_specific_shuffle(int pattern) {
    v8si a = {0}, b = {0};
    v8si mask = {0};
    
    for (int i = 0; i < 8; i++) {
        mask[i] = (pattern * i) % 16;
    }
    
    /* Multiple shuffles in control flow */
    if (pattern & 1) {
        v8si temp = __builtin_shuffle(a, b, mask);
        v8si mask2 = mask + (v8si){1, 2, 3, 4, 5, 6, 7, 8};
        v8si result = __builtin_shuffle(temp, a, mask2);
        (void)result;
    }
}
#endif

#ifdef __SSE2__
__attribute__((noinline))
void sse2_specific_shuffle(int pattern) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si mask = {pattern % 4, (pattern + 1) % 4, (pattern + 2) % 4, (pattern + 3) % 4};
    
    v4si result = __builtin_shuffle(a, b, mask);
    (void)result;
}
#endif

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Create volatile condition to prevent dead code elimination */
    volatile int use_complex = 1;
    
    /* Loop with different shuffle patterns */
    for (int iter = 0; iter < 10; iter++) {
        /* Use volatile mask indices */
        int indices[16];
        for (int i = 0; i < 16; i++) {
            indices[i] = volatile_mask[(iter * 4 + i) % 64];
        }
        
        /* Mixed SIMD patterns */
        mixed_simd_patterns(indices, global_floats, iter);
        
        /* Architecture-specific code paths */
        if (use_complex) {
#ifdef __AVX512F__
            avx512_specific_shuffle(iter);
#endif
#ifdef __AVX2__
            avx2_specific_shuffle(iter);
#endif
#ifdef __SSE2__
            sse2_specific_shuffle(iter);
#endif
        }
        
        /* Load vectors for shuffle functions */
        v8si vi1 = *(v8si*)&global_ints[iter * 8];
        v8si vi2 = *(v8si*)&global_ints[iter * 8 + 8];
        v8si vi3 = *(v8si*)&global_ints[iter * 8 + 16];
        v8si mask1 = {0, 2, 4, 6, 1, 3, 5, 7};
        v8si mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
        
        /* Call complex shuffle function */
        v8si shuffled = shuffle_v8si_complex(vi1, vi2, vi3, mask1, mask2);
        
        /* Store result */
        for (int i = 0; i < 8; i++) {
            accumulator[iter * 8 + i] += shuffled[i];
        }
        
        /* Floating-point shuffle with control flow */
        v8sf vf1 = *(v8sf*)&global_floats[iter * 8];
        v8sf vf2 = *(v8sf*)&global_floats[iter * 8 + 8];
        v8sf vf_mask = {1.0f, 3.0f, 5.0f, 7.0f, 0.0f, 2.0f, 4.0f, 6.0f};
        
        v8sf fresult = shuffle_v8sf_with_control(vf1, vf2, vf_mask, iter);
        
        /* Use result to prevent elimination */
        volatile float* volatile_float = (float*)&fresult;
        (void)volatile_float;
    }
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
