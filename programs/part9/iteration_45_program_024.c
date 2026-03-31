#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));        /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float (512-bit) */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double (512-bit) */

/* Global data arrays */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static double global_doubles[ARRAY_SIZE];

/* Accumulator arrays */
static int32_t accum_ints[ARRAY_SIZE] = {0};
static float accum_floats[ARRAY_SIZE] = {0};
static double accum_doubles[ARRAY_SIZE] = {0.0};

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_ints[i] = (int32_t)(r >> 16) & 0x7FFF;
        global_floats[i] = (float)((r >> 16) & 0x7FFF) * 0.1f;
        global_doubles[i] = (double)((r >> 16) & 0x7FFF) * 0.01;
    }
}

/* Function to create volatile mask indices - prevents constant folding */
static volatile int mask_idx = 0;

/* Complex shuffle with 10+ operands using __builtin_shufflevector */
static v16si shuffle_16si_complex(v16si a, v16si b, v16si c, v16si d, 
                                  int idx0, int idx1, int idx2, int idx3,
                                  int idx4, int idx5, int idx6, int idx7) {
    /* Use volatile to prevent constant folding of indices */
    volatile int vi0 = idx0;
    volatile int vi1 = idx1;
    volatile int vi2 = idx2;
    volatile int vi3 = idx3;
    volatile int vi4 = idx4;
    volatile int vi5 = idx5;
    volatile int vi6 = idx6;
    volatile int vi7 = idx7;
    
    /* This __builtin_shufflevector call requires 10+ operands during expansion:
     * 2 input vectors + 16 mask indices = 18 operands total
     * The expander will need to handle this as a multi-operand case
     */
    v16si result = __builtin_shufflevector(a, b, 
        vi0, vi1, vi2, vi3, vi4, vi5, vi6, vi7,
        vi0+8, vi1+8, vi2+8, vi3+8, vi4+8, vi5+8, vi6+8, vi7+8);
    
    /* Mix with other vectors using arithmetic to create more complex patterns */
    result = result + c;
    result = result - d;
    
    return result;
}

/* Mixed-type shuffle with floating point vectors */
static v16sf shuffle_16sf_mixed(v16sf a, v16sf b, v16sf c,
                                int idx0, int idx1, int idx2, int idx3,
                                int idx4, int idx5, int idx6, int idx7,
                                int idx8, int idx9, int idx10, int idx11) {
    /* Volatile indices to prevent optimization */
    volatile int vi[12];
    vi[0] = idx0; vi[1] = idx1; vi[2] = idx2; vi[3] = idx3;
    vi[4] = idx4; vi[5] = idx5; vi[6] = idx6; vi[7] = idx7;
    vi[8] = idx8; vi[9] = idx9; vi[10] = idx10; vi[11] = idx11;
    
    /* 11+ operands: 2 vectors + 12 indices = 14 operands */
    v16sf result = __builtin_shufflevector(a, b,
        vi[0], vi[1], vi[2], vi[3], vi[4], vi[5], vi[6], vi[7],
        vi[8], vi[9], vi[10], vi[11], vi[0]+4, vi[1]+4, vi[2]+4, vi[3]+4);
    
    /* Additional operation to create dependency */
    result = result * c;
    
    return result;
}

/* Double precision shuffle with control flow */
static v8df shuffle_8df_with_control(v8df a, v8df b, v8df c, int base_idx, int selector) {
    v8df result;
    
    /* Complex control flow around vector operations */
    if (selector & 1) {
        volatile int idx0 = (base_idx + 0) & 7;
        volatile int idx1 = (base_idx + 1) & 7;
        volatile int idx2 = (base_idx + 2) & 7;
        volatile int idx3 = (base_idx + 3) & 7;
        volatile int idx4 = (base_idx + 4) & 7;
        volatile int idx5 = (base_idx + 5) & 7;
        volatile int idx6 = (base_idx + 6) & 7;
        volatile int idx7 = (base_idx + 7) & 7;
        
        /* 10 operands: 2 vectors + 8 indices */
        result = __builtin_shufflevector(a, b,
            idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    } else {
        volatile int idx0 = (base_idx + 7) & 7;
        volatile int idx1 = (base_idx + 6) & 7;
        volatile int idx2 = (base_idx + 5) & 7;
        volatile int idx3 = (base_idx + 4) & 7;
        volatile int idx4 = (base_idx + 3) & 7;
        volatile int idx5 = (base_idx + 2) & 7;
        volatile int idx6 = (base_idx + 1) & 7;
        volatile int idx7 = (base_idx + 0) & 7;
        
        result = __builtin_shufflevector(b, a,
            idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    }
    
    /* Switch statement with vector operations inside */
    switch (selector & 3) {
        case 0:
            result = result + c;
            break;
        case 1:
            result = result - c;
            break;
        case 2:
            result = result * c;
            break;
        case 3:
            /* Nested shuffle */
            volatile int idx = base_idx & 3;
            result = __builtin_shufflevector(result, c, 0, 1, 2, 3, 4+idx, 5+idx, 6+idx, 7+idx);
            break;
    }
    
    return result;
}

/* Function using __builtin_shuffle with GCC vector extensions */
static v8si shuffle_v8si_dynamic(v8si a, v8si b, v8si mask) {
    /* __builtin_shuffle with vector mask - may expand to many operands */
    v8si result = __builtin_shuffle(a, b, mask);
    
    /* Volatile store to prevent optimization */
    volatile v8si temp = result;
    result = temp;
    
    return result;
}

/* Main processing function with loops and control flow */
void process_vectors(int iterations, int seed_mod) {
    volatile int control = seed_mod;
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = (iter * 17 + control) & 31;
        int selector = (iter + control) & 255;
        
        /* Load data from global arrays into vectors */
        v16si int_vec1 = *(v16si*)&global_ints[iter * 16];
        v16si int_vec2 = *(v16si*)&global_ints[iter * 16 + 16];
        v16si int_vec3 = *(v16si*)&global_ints[iter * 16 + 32];
        v16si int_vec4 = *(v16si*)&global_ints[iter * 16 + 48];
        
        v16sf float_vec1 = *(v16sf*)&global_floats[iter * 16];
        v16sf float_vec2 = *(v16sf*)&global_floats[iter * 16 + 16];
        v16sf float_vec3 = *(v16sf*)&global_floats[iter * 16 + 32];
        
        v8df double_vec1 = *(v8df*)&global_doubles[iter * 8];
        v8df double_vec2 = *(v8df*)&global_doubles[iter * 8 + 8];
        v8df double_vec3 = *(v8df*)&global_doubles[iter * 8 + 16];
        
        /* Perform complex shuffles with many operands */
        if (iter & 1) {
            v16si int_result = shuffle_16si_complex(
                int_vec1, int_vec2, int_vec3, int_vec4,
                base_idx, base_idx+1, base_idx+2, base_idx+3,
                base_idx+4, base_idx+5, base_idx+6, base_idx+7);
            
            /* Store to accumulator */
            v16si* accum_ptr = (v16si*)&accum_ints[iter * 16];
            *accum_ptr = *accum_ptr + int_result;
        }
        
        if (iter & 2) {
            v16sf float_result = shuffle_16sf_mixed(
                float_vec1, float_vec2, float_vec3,
                base_idx, base_idx+1, base_idx+2, base_idx+3,
                base_idx+4, base_idx+5, base_idx+6, base_idx+7,
                base_idx+8, base_idx+9, base_idx+10, base_idx+11);
            
            v16sf* accum_ptr = (v16sf*)&accum_floats[iter * 16];
            *accum_ptr = *accum_ptr + float_result;
        }
        
        /* Always process doubles with control flow */
        v8df double_result = shuffle_8df_with_control(
            double_vec1, double_vec2, double_vec3,
            base_idx, selector);
        
        v8df* accum_ptr = (v8df*)&accum_doubles[iter * 8];
        *accum_ptr = *accum_ptr + double_result;
        
        /* Additional mixed-SIMD pattern: narrow 512-bit to 256-bit and back */
        if (iter & 4) {
#ifdef __AVX512F__
            /* Use 512-bit vectors */
            v8si narrow_vec1 = __builtin_convertvector(int_vec1, v8si);
            v8si narrow_vec2 = __builtin_convertvector(int_vec2, v8si);
            
            volatile v8si mask = {0, 2, 4, 6, 8, 10, 12, 14};
            v8si narrow_result = shuffle_v8si_dynamic(narrow_vec1, narrow_vec2, mask);
            
            /* Expand back */
            v16si expanded = __builtin_shufflevector(
                narrow_result, narrow_result,
                0, 1, 2, 3, 4, 5, 6, 7,
                0, 1, 2, 3, 4, 5, 6, 7);
            
            v16si* accum_ptr = (v16si*)&accum_ints[(iter + 1) % iterations * 16];
            *accum_ptr = *accum_ptr + expanded;
#endif
        }
    }
}

/* Compute checksum/hash of results */
int64_t compute_checksum(void) {
    int64_t checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += accum_ints[i];
        checksum += (int64_t)(accum_floats[i] * 1000.0f);
        checksum += (int64_t)(accum_doubles[i] * 10000.0);
    }
    
    return checksum;
}

int main(int argc, char* argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Initializing with seed: %d\n", seed);
    init_arrays(seed);
    
    /* Set volatile mask index from command line or seed */
    if (argc > 2) {
        mask_idx = atoi(argv[2]);
    } else {
        mask_idx = seed;
    }
    
    int iterations = 10;
    
    /* Architecture-specific compilation paths */
#ifdef __SSE2__
    printf("SSE2 enabled\n");
#endif
    
#ifdef __AVX2__
    printf("AVX2 enabled\n");
#endif
    
#ifdef __AVX512F__
    printf("AVX512F enabled\n");
    iterations = 8;  /* Fewer iterations for 512-bit vectors */
#endif
    
    /* Process vectors with complex control flow */
    process_vectors(iterations, seed);
    
    /* Compute and print checksum */
    int64_t checksum = compute_checksum();
    printf("Result checksum: %lld\n", (long long)checksum);
    
    return 0;
}
