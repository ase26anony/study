#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator[512] __attribute__((aligned(64)));

/* Vector type definitions using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));    /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));   /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));   /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));   /* 512-bit double */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < 512; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int)(r >> 16) & 0x7FFF;
        global_float_array[i] = (float)((r >> 16) & 0xFF) * 0.1f;
        accumulator[i] = 0;
    }
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
static v8si shuffle_v8si_many_ops(v8si a, v8si b, v8si mask1, v8si mask2, 
                                  v8si mask3, v8si mask4, int idx) {
    volatile int temp_idx = idx;
    
    /* Complex control flow to prevent optimization */
    if (temp_idx & 1) {
        /* First shuffle pattern - requires many operands during expansion */
        v8si result = __builtin_shuffle(a, b, mask1);
        
        /* Nested shuffle with different masks */
        if (temp_idx & 2) {
            result = __builtin_shuffle(result, a, mask2);
        } else {
            result = __builtin_shuffle(result, b, mask3);
        }
        
        /* Final shuffle with conditional mask */
        return __builtin_shuffle(result, (temp_idx & 4) ? mask2 : mask4, mask1);
    } else {
        /* Alternative path with different shuffle patterns */
        v8si temp = __builtin_shuffle(b, a, mask3);
        return __builtin_shuffle(temp, mask4, mask2);
    }
}
#endif

#ifdef __AVX512F__
/* Function targeting 11+ operand expansion */
static v16si shuffle_v16si_complex(v16si a, v16si b, v16si c, 
                                   v16si mask1, v16si mask2,
                                   v16si mask3, v16si mask4,
                                   int pattern) {
    volatile int pat = pattern;
    
    switch (pat & 0x7) {
        case 0:
        case 1:
            /* Chain of shuffles that may require many operands */
            return __builtin_shuffle(
                __builtin_shuffle(a, b, mask1),
                __builtin_shuffle(c, a, mask2),
                mask3
            );
            
        case 2:
        case 3:
            /* Different pattern with more shuffle combinations */
            return __builtin_shuffle(
                a,
                __builtin_shuffle(
                    __builtin_shuffle(b, c, mask1),
                    __builtin_shuffle(a, b, mask4),
                    mask2
                ),
                mask3
            );
            
        default:
            /* Complex nested shuffle expression */
            v16si t1 = __builtin_shuffle(a, b, mask1);
            v16si t2 = __builtin_shuffle(b, c, mask2);
            v16si t3 = __builtin_shuffle(c, a, mask3);
            return __builtin_shuffle(t1, t2, mask4);
    }
}
#endif

/* Mixed integer/float shuffle function */
#ifdef __AVX2__
static v8sf shuffle_mixed_types(v8sf fa, v8sf fb, v8si int_mask, 
                                v8si alt_mask, int selector) {
    volatile int sel = selector;
    
    /* Convert between types to stress different optab paths */
    v8si ia = *(v8si*)&fa;
    v8si ib = *(v8si*)&fb;
    
    if (sel & 1) {
        /* Shuffle integer representation */
        v8si shuffled_int = __builtin_shuffle(ia, ib, int_mask);
        /* Convert back to float */
        return *(v8sf*)&shuffled_int;
    } else {
        /* Direct float shuffle with integer mask */
        return __builtin_shufflevector(fa, fb, 
            0, 2, 4, 6, 8, 10, 12, 14);
    }
}
#endif

/* Function using __builtin_shufflevector with explicit indices */
#ifdef __AVX512F__
static v8df shufflevector_v8df_large(v8df a, v8df b, int* indices) {
    volatile int idx0 = indices[0] & 15;
    volatile int idx1 = indices[1] & 15;
    volatile int idx2 = indices[2] & 15;
    volatile int idx3 = indices[3] & 15;
    volatile int idx4 = indices[4] & 15;
    volatile int idx5 = indices[5] & 15;
    volatile int idx6 = indices[6] & 15;
    volatile int idx7 = indices[7] & 15;
    
    /* __builtin_shufflevector with many explicit indices */
    return __builtin_shufflevector(a, b,
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
}
#endif

/* Main processing function with loops and control flow */
void process_vectors(int iterations, int seed_offset) {
    volatile int iter_mod = iterations;
    
    for (int i = 0; i < iterations; i++) {
        int base_idx = (i * 16) & 0x1FF;  /* Wrap within array bounds */
        
#ifdef __AVX2__
        /* Load 256-bit vectors */
        v8si va = *(v8si*)&global_int_array[base_idx];
        v8si vb = *(v8si*)&global_int_array[base_idx + 8];
        v8si mask1 = {0, 2, 4, 6, 1, 3, 5, 7};
        v8si mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si mask3 = {1, 0, 3, 2, 5, 4, 7, 6};
        v8si mask4 = {6, 7, 4, 5, 2, 3, 0, 1};
        
        /* Perform shuffle with many operands */
        v8si result = shuffle_v8si_many_ops(va, vb, mask1, mask2, mask3, mask4, 
                                           i + seed_offset);
        
        /* Store result with volatile write */
        volatile v8si* store_ptr = (v8si*)&accumulator[base_idx];
        *store_ptr = result;
#endif

#ifdef __AVX512F__
        if (iter_mod & 0x1) {
            /* Load 512-bit vectors */
            v16si vc = *(v16si*)&global_int_array[base_idx];
            v16si vd = *(v16si*)&global_int_array[base_idx + 16];
            v16si ve = *(v16si*)&global_int_array[base_idx + 32];
            
            v16si mask_a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
            v16si mask_b = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
            v16si mask_c = {1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14};
            v16si mask_d = {14,15,12,13,10,11,8,9,6,7,4,5,2,3,0,1};
            
            /* Complex shuffle with potential 11+ operand expansion */
            v16si result512 = shuffle_v16si_complex(vc, vd, ve, mask_a, mask_b,
                                                   mask_c, mask_d, i);
            
            /* Accumulate results */
            for (int j = 0; j < 16; j++) {
                accumulator[base_idx + j] += result512[j];
            }
        }
#endif

#ifdef __AVX2__
        /* Process float vectors */
        v8sf fva = *(v8sf*)&global_float_array[base_idx];
        v8sf fvb = *(v8sf*)&global_float_array[base_idx + 8];
        v8si int_mask = {0, 7, 1, 6, 2, 5, 3, 4};
        
        v8sf float_result = shuffle_mixed_types(fva, fvb, int_mask, mask1, i);
        
        /* Convert float result to integer and accumulate */
        int* int_result = (int*)&float_result;
        for (int j = 0; j < 8; j++) {
            accumulator[base_idx + j] += int_result[j];
        }
#endif

#ifdef __AVX512F__
        /* Double precision shuffle with explicit indices */
        if (i % 3 == 0) {
            v8df da = *(v8df*)&global_float_array[base_idx];
            v8df db = *(v8df*)&global_float_array[base_idx + 8];
            
            int indices[8];
            for (int j = 0; j < 8; j++) {
                indices[j] = (i + j + seed_offset) & 0xF;
            }
            
            v8df double_result = shufflevector_v8df_large(da, db, indices);
            
            /* Accumulate double results as integers */
            int* dbl_as_int = (int*)&double_result;
            for (int j = 0; j < 16; j++) {
                accumulator[base_idx + j] += dbl_as_int[j];
            }
        }
#endif
    }
}

int main(int argc, char* argv[]) {
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    /* Process with different iteration counts */
    process_vectors(10, seed);
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += (unsigned int)accumulator[i];
    }
    
    printf("Checksum: %llu (seed: %d)\n", checksum, seed);
    
    return 0;
}
