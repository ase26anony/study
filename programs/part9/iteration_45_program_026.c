#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator[512] __attribute__((aligned(64)));

/* Vector types using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 8 ints - 256-bit */
typedef int v16si __attribute__((vector_size(64)));     /* 16 ints - 512-bit */
typedef float v8sf __attribute__((vector_size(32)));    /* 8 floats - 256-bit */
typedef float v16sf __attribute__((vector_size(64)));   /* 16 floats - 512-bit */
typedef double v4df __attribute__((vector_size(32)));   /* 4 doubles - 256-bit */
typedef double v8df __attribute__((vector_size(64)));   /* 8 doubles - 512-bit */

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

/* Complex shuffle with 10+ operands - integer version */
__attribute__((noinline))
v16si shuffle_int_16way(v16si a, v16si b, v16si c, v16si mask) {
    volatile int idx = global_seed & 0xF;  /* Prevent constant folding */
    
    /* Complex control flow to stress expander */
    if (idx < 8) {
        /* First pattern: interleave elements with 10+ operand shuffle */
        v16si result = __builtin_shuffle(a, b, mask);
        
        /* Additional shuffle with arithmetic to create more operands */
        v16si temp = __builtin_shuffle(result, c, 
            (v16si){0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23});
        
        return temp + (v16si){idx, idx, idx, idx, idx, idx, idx, idx,
                              idx, idx, idx, idx, idx, idx, idx, idx};
    } else {
        /* Second pattern: different shuffle arrangement */
        v16si temp1 = __builtin_shuffle(b, a, mask);
        v16si temp2 = __builtin_shuffle(c, temp1, 
            (v16si){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0});
        
        return temp2 * (v16si){2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
    }
}

/* Complex shuffle with 10+ operands - float version */
__attribute__((noinline))
v16sf shuffle_float_16way(v16sf a, v16sf b, v16sf c, v16si mask) {
    volatile int idx = global_seed & 0x7;
    
    switch (idx) {
        case 0:
        case 1:
        case 2: {
            /* This should trigger 10+ operand expansion */
            v16sf result = __builtin_shuffle(a, b, mask);
            
            /* Nested shuffle with conversion */
            v8sf narrow = __builtin_convertvector(result, v8sf);
            v16sf expanded = __builtin_shufflevector(narrow, narrow,
                0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
            
            return expanded + c;
        }
        case 3:
        case 4: {
            /* Alternative shuffle pattern */
            v16sf temp = __builtin_shuffle(b, c, mask);
            v16sf rotated = __builtin_shuffle(temp, temp,
                (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0});
            
            return rotated * (v16sf){1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
                                     1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f};
        }
        default: {
            /* Complex pattern with multiple shuffles */
            v16sf t1 = __builtin_shuffle(a, b, 
                (v16si){0,16,2,18,4,20,6,22,8,24,10,26,12,28,14,30});
            v16sf t2 = __builtin_shuffle(c, t1,
                (v16si){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0});
            
            return t1 + t2;
        }
    }
}

/* Double precision shuffle with mixed operations */
__attribute__((noinline))
v8df shuffle_double_8way(v8df a, v8df b, v8df c, v8si mask) {
    volatile int pattern = global_seed & 0x3;
    
    for (int i = 0; i < 3; i++) {
        if (pattern == i) {
            /* Loop-dependent shuffle indices */
            v8si dynamic_mask = mask + (v8si){i, i, i, i, i, i, i, i};
            
            /* Shuffle with 10+ operands when counting all implicit ones */
            v8df result = __builtin_shuffle(a, b, dynamic_mask);
            
            /* Additional operation to create more complex RTL */
            v8df scaled = result * (v8df){1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8};
            
            /* Another shuffle mixing with third vector */
            v8df final = __builtin_shuffle(scaled, c,
                (v8si){0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15});
            
            return final;
        }
    }
    
    return a + b + c;
}

/* Mixed SIMD width operations */
__attribute__((noinline))
void mixed_width_shuffles(int offset) {
    /* Load 256-bit vectors */
    v8si vi8 = *(v8si*)&global_int_array[offset];
    v8sf vf8 = *(v8sf*)&global_float_array[offset];
    
    /* Load 512-bit vectors */
    v16si vi16 = *(v16si*)&global_int_array[offset * 2];
    v16sf vf16 = *(v16sf*)&global_float_array[offset * 2];
    
    volatile int mask_idx = offset & 0xF;
    v16si mask16 = (v16si){mask_idx, mask_idx+1, mask_idx+2, mask_idx+3,
                          mask_idx+4, mask_idx+5, mask_idx+6, mask_idx+7,
                          mask_idx+8, mask_idx+9, mask_idx+10, mask_idx+11,
                          mask_idx+12, mask_idx+13, mask_idx+14, mask_idx+15};
    
    /* Perform shuffles with different vector widths */
    v16si int_result = shuffle_int_16way(vi16, vi16, vi16, mask16);
    v16sf float_result = shuffle_float_16way(vf16, vf16, vf16, mask16);
    
    /* Convert and mix results */
    v8si narrow_int = __builtin_convertvector(int_result, v8si);
    v8sf narrow_float = __builtin_convertvector(float_result, v8sf);
    
    /* Store results to accumulator */
    *(v8si*)&accumulator[offset] += narrow_int;
    *(v8sf*)&accumulator[offset + 8] = __builtin_convertvector(
        narrow_float + __builtin_convertvector(narrow_int, v8sf), v8si);
}

/* Architecture-specific variants */
#ifdef __AVX512F__
__attribute__((noinline))
void avx512_specific_shuffles(int offset) {
    v16si va = *(v16si*)&global_int_array[offset];
    v16si vb = *(v16si*)&global_int_array[offset + 16];
    v16si vc = *(v16si*)&global_int_array[offset + 32];
    
    /* Complex shuffle pattern likely needing many operands */
    v16si mask = (v16si){0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    for (int i = 0; i < 4; i++) {
        if ((global_seed >> i) & 1) {
            v16si result = __builtin_shuffle(va, vb, mask);
            result = __builtin_shuffle(result, vc, 
                (v16si){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0});
            
            *(v16si*)&accumulator[offset] += result;
        }
    }
}
#endif

#ifdef __AVX2__
__attribute__((noinline))
void avx2_specific_shuffles(int offset) {
    v8df va = *(v8df*)&global_int_array[offset];
    v8df vb = *(v8df*)&global_int_array[offset + 8];
    v8df vc = *(v8df*)&global_int_array[offset + 16];
    
    v8si mask = (v8si){0,8,2,10,4,12,6,14,1,9,3,11,5,13,7,15};
    
    v8df result = shuffle_double_8way(va, vb, vc, mask);
    
    /* Store through volatile pointer to prevent elimination */
    volatile v8df* volatile_ptr = &result;
    *(v8df*)&accumulator[offset] = __builtin_convertvector(*volatile_ptr, v8si);
}
#endif

#ifdef __SSE2__
__attribute__((noinline))
void sse2_specific_shuffles(int offset) {
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = *(v4si*)&global_int_array[offset];
    v4si b = *(v4si*)&global_int_array[offset + 4];
    v4si c = *(v4si*)&global_int_array[offset + 8];
    
    /* Chain multiple shuffles to create complex patterns */
    v4si r1 = __builtin_shuffle(a, b, (v4si){0,4,1,5});
    v4si r2 = __builtin_shuffle(c, r1, (v4si){3,2,1,0});
    v4si r3 = __builtin_shuffle(r1, r2, (v4si){0,1,4,5});
    
    *(v4si*)&accumulator[offset] += r3;
}
#endif

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    printf("Starting vector shuffle tests with seed %d\n", seed);
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        global_seed = iter;  /* Change volatile seed each iteration */
        
        for (int offset = 0; offset < 480; offset += 32) {
            mixed_width_shuffles(offset);
            
            /* Architecture-specific paths */
#ifdef __AVX512F__
            if (iter % 3 == 0) {
                avx512_specific_shuffles(offset);
            }
#endif
            
#ifdef __AVX2__
            if (iter % 2 == 0) {
                avx2_specific_shuffles(offset);
            }
#endif
            
#ifdef __SSE2__
            if (iter % 4 == 0) {
                sse2_specific_shuffles(offset);
            }
#endif
            
            /* Volatile memory barrier */
            volatile int barrier = accumulator[offset];
            (void)barrier;
        }
    }
    
    /* Compute final checksum */
    long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
