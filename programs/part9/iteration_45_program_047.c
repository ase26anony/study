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
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int lcg = seed;
    for (int i = 0; i < 512; i++) {
        lcg = lcg * 1103515245 + 12345;
        global_int_array[i] = (int)(lcg % 1000);
        global_float_array[i] = (float)(lcg % 1000) * 0.1f;
        accumulator[i] = 0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
__attribute__((noinline))
v8si shuffle_10_operand_int(v8si a, v8si b, v8si mask1, v8si mask2, 
                           v8si mask3, v8si mask4, volatile int idx) {
    /* Complex control flow to prevent optimization */
    v8si result;
    if (idx & 1) {
        /* First shuffle pattern - uses 10 operands total */
        v8si temp = __builtin_shuffle(a, b, mask1);
        /* Nested shuffle with another mask */
        result = __builtin_shuffle(temp, a, mask2);
    } else {
        /* Second shuffle pattern */
        v8si temp = __builtin_shuffle(b, a, mask3);
        result = __builtin_shuffle(temp, b, mask4);
    }
    
    /* Additional arithmetic to use result */
    return result + (v8si){1, 2, 3, 4, 5, 6, 7, 8};
}

__attribute__((noinline))
v8sf shuffle_11_operand_float(v8sf a, v8sf b, v8si mask1, v8si mask2,
                             v8si mask3, v8si mask4, v8si mask5, volatile int idx) {
    /* Switch statement with volatile condition */
    v8sf result;
    switch (idx % 4) {
        case 0:
            /* Pattern requiring 11 operands during expansion */
            result = __builtin_shuffle(a, b, mask1);
            break;
        case 1:
            result = __builtin_shuffle(b, a, mask2);
            break;
        case 2: {
            /* Complex expression that might need many operand slots */
            v8sf temp1 = __builtin_shuffle(a, b, mask3);
            v8sf temp2 = __builtin_shuffle(b, a, mask4);
            result = __builtin_shuffle(temp1, temp2, mask5);
            break;
        }
        default:
            result = a + b;
    }
    
    /* Prevent dead code elimination */
    volatile v8sf* volatile_ptr = &result;
    (void)volatile_ptr;
    
    return result * 2.0f;
}
#endif

#ifdef __AVX512F__
__attribute__((noinline))
v16si large_shuffle_10plus(v16si a, v16si b, v16si mask, 
                          volatile int pattern, volatile int* control) {
    v16si result;
    
    /* Loop with conditional shuffle execution */
    for (int i = 0; i < 4; i++) {
        if (pattern & (1 << i)) {
            /* Each iteration could potentially generate multi-operand expansion */
            v16si temp = __builtin_shuffle(a, b, mask + (v16si){i});
            
            /* Mix with __builtin_shufflevector for variety */
            if (i == 0) {
                /* __builtin_shufflevector with explicit indices - many operands */
                result = __builtin_shufflevector(a, b, 
                    0, 16, 1, 17, 2, 18, 3, 19,
                    4, 20, 5, 21, 6, 22, 7, 23);
            } else {
                result = temp;
            }
            
            /* Store intermediate result to memory */
            v16si* mem = (v16si*)&accumulator[i * 16];
            *mem = result;
        }
    }
    
    /* Use control value to affect result */
    if (*control > 0) {
        return result + (v16si){*control};
    }
    return result;
}

__attribute__((noinline))
v8df double_shuffle_complex(v8df a, v8df b, v8si mask1, v8si mask2,
                           volatile int idx1, volatile int idx2) {
    /* Nested control flow with multiple shuffle possibilities */
    v8df result;
    
    if (idx1 < idx2) {
        /* First complex shuffle chain */
        v8df t1 = __builtin_shuffle(a, b, mask1);
        v8df t2 = __builtin_shuffle(b, a, mask2);
        
        /* Create another mask dynamically */
        v8si mask3 = mask1 + (v8si){idx1};
        result = __builtin_shuffle(t1, t2, mask3);
    } else if (idx1 > idx2) {
        /* Different shuffle pattern */
        v8si mask4 = mask2 - (v8si){idx2};
        result = __builtin_shuffle(a, b, mask4);
    } else {
        /* Default: interleave shuffle with many explicit indices */
        result = __builtin_shufflevector(a, b,
            0, 8, 1, 9, 2, 10, 3, 11,
            4, 12, 5, 13, 6, 14, 7, 15);
    }
    
    return result;
}
#endif

/* SSE2 fallback for architectures without AVX */
#ifdef __SSE2__
__attribute__((noinline))
v4si sse2_shuffle_multi(v4si a, v4si b, v4si mask1, v4si mask2,
                       volatile int selector) {
    v4si result;
    
    /* Multiple shuffle patterns in switch */
    switch (selector & 3) {
        case 0:
            result = __builtin_shuffle(a, b, mask1);
            break;
        case 1:
            result = __builtin_shuffle(b, a, mask2);
            break;
        case 2: {
            /* Chain shuffles */
            v4si t1 = __builtin_shuffle(a, b, mask1);
            v4si t2 = __builtin_shuffle(b, a, mask2);
            v4si mask3 = mask1 + mask2;
            result = __builtin_shuffle(t1, t2, mask3);
            break;
        }
        default:
            result = a + b;
    }
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    global_seed = seed;
    
    init_arrays(seed);
    
    volatile int loop_counter = 0;
    volatile int pattern_selector = seed % 8;
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        loop_counter = iter;
        
#ifdef __SSE2__
        /* Test SSE2 path */
        v4si sse_a = *(v4si*)&global_int_array[iter * 4];
        v4si sse_b = *(v4si*)&global_int_array[iter * 4 + 4];
        v4si sse_mask1 = {3, 2, 1, 0};
        v4si sse_mask2 = {1, 0, 3, 2};
        
        v4si sse_result = sse2_shuffle_multi(sse_a, sse_b, sse_mask1, 
                                            sse_mask2, pattern_selector);
        
        /* Accumulate result */
        for (int i = 0; i < 4; i++) {
            accumulator[iter * 4 + i] += sse_result[i];
        }
#endif

#ifdef __AVX2__
        /* Test AVX2 path - 256-bit vectors */
        v8si avx2_a = *(v8si*)&global_int_array[iter * 8];
        v8si avx2_b = *(v8si*)&global_int_array[iter * 8 + 8];
        v8si avx2_mask1 = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si avx2_mask2 = {0, 1, 2, 3, 4, 5, 6, 7};
        v8si avx2_mask3 = {3, 2, 1, 0, 7, 6, 5, 4};
        v8si avx2_mask4 = {4, 5, 6, 7, 0, 1, 2, 3};
        
        v8si int_result = shuffle_10_operand_int(avx2_a, avx2_b, avx2_mask1,
                                                avx2_mask2, avx2_mask3,
                                                avx2_mask4, pattern_selector);
        
        /* Float version */
        v8sf avx2_float_a = *(v8sf*)&global_float_array[iter * 8];
        v8sf avx2_float_b = *(v8sf*)&global_float_array[iter * 8 + 8];
        v8si avx2_mask5 = {1, 0, 3, 2, 5, 4, 7, 6};
        
        v8sf float_result = shuffle_11_operand_float(avx2_float_a, avx2_float_b,
                                                    avx2_mask1, avx2_mask2,
                                                    avx2_mask3, avx2_mask4,
                                                    avx2_mask5, pattern_selector);
        
        /* Mix results */
        for (int i = 0; i < 8; i++) {
            accumulator[iter * 8 + i] += int_result[i];
            accumulator[iter * 8 + i + 256] += (int)float_result[i];
        }
#endif

#ifdef __AVX512F__
        /* Test AVX512 path - 512-bit vectors */
        if (iter < 5) {  /* Limit iterations for large vectors */
            v16si avx512_a = *(v16si*)&global_int_array[iter * 16];
            v16si avx512_b = *(v16si*)&global_int_array[iter * 16 + 16];
            v16si avx512_mask = {15, 14, 13, 12, 11, 10, 9, 8,
                                 7, 6, 5, 4, 3, 2, 1, 0};
            
            volatile int control_var = pattern_selector + iter;
            v16si avx512_result = large_shuffle_10plus(avx512_a, avx512_b,
                                                      avx512_mask, 
                                                      pattern_selector,
                                                      &control_var);
            
            /* Double precision test */
            v8df avx512_double_a = *(v8df*)&global_float_array[iter * 8];
            v8df avx512_double_b = *(v8df*)&global_float_array[iter * 8 + 8];
            v8si avx512_dmask1 = {7, 6, 5, 4, 3, 2, 1, 0};
            v8si avx512_dmask2 = {0, 1, 2, 3, 4, 5, 6, 7};
            
            v8df double_result = double_shuffle_complex(avx512_double_a,
                                                       avx512_double_b,
                                                       avx512_dmask1,
                                                       avx512_dmask2,
                                                       pattern_selector,
                                                       iter);
            
            /* Store results */
            for (int i = 0; i < 16; i++) {
                accumulator[iter * 16 + i] += avx512_result[i];
            }
        }
#endif
        
        /* Update pattern selector */
        pattern_selector = (pattern_selector * 1103515245 + 12345) & 0x7F;
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += (unsigned long long)accumulator[i];
    }
    
    printf("Final checksum: %llu (seed: %d)\n", checksum, seed);
    
    return 0;
}
