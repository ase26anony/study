#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static volatile int32_t volatile_ints[ARRAY_SIZE];
static volatile float volatile_floats[ARRAY_SIZE];

/* Accumulator arrays */
static int32_t acc_ints[ARRAY_SIZE];
static float acc_floats[ARRAY_SIZE];

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_ints[i] = (i * 1103515245 + seed) % 1000;
        global_floats[i] = ((i * 1103515245 + seed) % 1000) * 0.1f;
        volatile_ints[i] = global_ints[i];
        volatile_floats[i] = global_floats[i];
        acc_ints[i] = 0;
        acc_floats[i] = 0.0f;
    }
}

/* Function 1: Large integer shuffle with 10+ operands */
#ifdef __AVX2__
static void shuffle_large_int_10op(int idx, int mask_seed) {
    /* Load 512-bit vectors (16 ints each) */
    v16si a = *(v16si*)&global_ints[idx];
    v16si b = *(v16si*)&global_ints[idx + 16];
    
    /* Create control mask from volatile variables to prevent constant folding */
    int32_t mask_vals[16];
    for (int i = 0; i < 16; i++) {
        mask_vals[i] = (volatile_ints[i + mask_seed] + i) % 32;
    }
    v16si mask = *(v16si*)mask_vals;
    
    /* Complex shuffle with many operands - this may expand to 10+ operands */
    v16si result;
    
    /* Use if-else chain to create complex control flow */
    if (volatile_ints[0] % 2) {
        /* First shuffle pattern */
        result = __builtin_shuffle(a, b, mask);
    } else {
        /* Second shuffle pattern with different mask */
        v16si mask2 = mask + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        result = __builtin_shuffle(b, a, mask2);
    }
    
    /* Additional operation on result */
    result = result + (v16si){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    /* Store to accumulator */
    v16si* acc = (v16si*)&acc_ints[idx];
    *acc = *acc + result;
}
#endif

/* Function 2: Mixed float/double shuffle with complex control flow */
#ifdef __AVX512F__
static void shuffle_mixed_float_11op(int idx, int mask_seed) {
    /* Load different vector types */
    v16sf fa = *(v16sf*)&global_floats[idx];
    v16sf fb = *(v16sf*)&global_floats[idx + 16];
    v8df da = *(v8df*)&global_floats[idx + 32];
    v8df db = *(v8df*)&global_floats[idx + 40];
    
    /* Create masks from volatile data */
    int32_t imask_vals[16];
    int64_t dmask_vals[8];
    
    for (int i = 0; i < 16; i++) {
        imask_vals[i] = (volatile_ints[i + mask_seed] + i * 3) % 32;
    }
    for (int i = 0; i < 8; i++) {
        dmask_vals[i] = (volatile_ints[i + mask_seed + 16] + i * 5) % 16;
    }
    
    v16si imask = *(v16si*)imask_vals;
    v8df dmask = *(v8df*)dmask_vals;
    
    /* Complex control flow with switch */
    v16sf fresult;
    v8df dresult;
    
    switch (volatile_ints[mask_seed] % 4) {
        case 0:
            fresult = __builtin_shuffle(fa, fb, imask);
            dresult = __builtin_shuffle(da, db, dmask);
            break;
        case 1:
            fresult = __builtin_shuffle(fb, fa, imask + 
                (v16si){16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31});
            dresult = __builtin_shuffle(db, da, dmask + (v8df){8,9,10,11,12,13,14,15});
            break;
        case 2:
            /* Nested shuffle operations */
            v16sf temp = __builtin_shuffle(fa, fb, imask);
            fresult = __builtin_shuffle(temp, fa, imask);
            dresult = __builtin_shuffle(da, db, dmask);
            break;
        default:
            fresult = fa;
            dresult = da;
            break;
    }
    
    /* Convert and mix results */
    fresult = fresult + (v16sf){0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,
                                0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f,1.6f};
    
    /* Store results */
    v16sf* facc = (v16sf*)&acc_floats[idx];
    *facc = *facc + fresult;
    
    /* Convert double to float and accumulate */
    for (int i = 0; i < 8; i++) {
        acc_floats[idx + 48 + i] += (float)dresult[i];
    }
}
#endif

/* Function 3: Narrowing and expanding shuffles */
#ifdef __SSE2__
static void shuffle_narrow_wide(int idx, int mask_seed) {
    /* Work with 256-bit vectors */
    v8si va = *(v8si*)&global_ints[idx];
    v8si vb = *(v8si*)&global_ints[idx + 8];
    
    /* Create complex mask */
    int32_t mask_vals[8];
    for (int i = 0; i < 8; i++) {
        mask_vals[i] = (volatile_ints[i + mask_seed] * 7 + i) % 16;
    }
    
    /* Use __builtin_shufflevector for explicit control */
    int32_t mask2_vals[16];
    for (int i = 0; i < 16; i++) {
        mask2_vals[i] = (volatile_ints[i + mask_seed + 8] + i * 11) % 16;
    }
    
    /* Loop with conditional shuffles */
    v8si result = {0};
    for (int i = 0; i < 4; i++) {
        if (volatile_ints[idx + i] % 3 == 0) {
            /* This shuffle may require many operands during expansion */
            result = result + __builtin_shuffle(va, vb, *(v8si*)mask_vals);
        } else {
            /* Alternative shuffle pattern */
            v8si temp = va + vb;
            result = result + __builtin_shuffle(temp, va, *(v8si*)mask_vals);
        }
        
        /* Rotate mask */
        for (int j = 0; j < 8; j++) {
            mask_vals[j] = (mask_vals[j] + 1) % 16;
        }
    }
    
    /* Store result */
    v8si* acc = (v8si*)&acc_ints[idx + 32];
    *acc = *acc + result;
}
#endif

/* Function 4: Very large shufflevector call - targeting 11 operands */
static void shufflevector_11operand(int idx) {
#ifdef __AVX512F__
    /* Create 512-bit vectors */
    v16si v1 = *(v16si*)&global_ints[idx];
    v16si v2 = *(v16si*)&global_ints[idx + 16];
    v16si v3 = *(v16si*)&global_ints[idx + 32];
    
    /* Use __builtin_shufflevector with explicit indices */
    /* This directly creates many operands */
    v16si result = __builtin_shufflevector(v1, v2, v3,
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23,
        8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
    
    /* Additional processing */
    result = result * (v16si){2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
    
    /* Store */
    v16si* acc = (v16si*)&acc_ints[idx + 64];
    *acc = *acc + result;
#endif
}

/* Main function with architecture-specific paths */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    int iterations = 10;
    int mask_base = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        mask_base = (mask_base + iter * 17) % 256;
        
        /* Call different shuffle functions based on iteration */
        switch (iter % 4) {
            case 0:
#ifdef __AVX2__
                shuffle_large_int_10op(iter * 16, mask_base);
#endif
                break;
            case 1:
#ifdef __AVX512F__
                shuffle_mixed_float_11op(iter * 16, mask_base);
#endif
                break;
            case 2:
#ifdef __SSE2__
                shuffle_narrow_wide(iter * 16, mask_base);
#endif
                break;
            case 3:
                shufflevector_11operand(iter * 16);
                break;
        }
        
        /* Force memory barrier with volatile store */
        volatile int barrier = iter;
        (void)barrier;
    }
    
    /* Compute checksums */
    int64_t int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += acc_ints[i];
        float_sum += acc_floats[i];
    }
    
    printf("Integer checksum: %ld\n", (long)int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    return 0;
}
